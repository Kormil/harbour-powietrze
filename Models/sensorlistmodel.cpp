#include "sensorlistmodel.h"
#include "src/connection/connection.h"
#include "src/modelsmanager.h"
#include "src/providersmanager.h"
#include "Types/station.h"
#include "src/settings.h"
#include <iostream>

SensorListModel::SensorListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_modelsManager(nullptr),
      m_station(nullptr)
{
}

QVariant SensorListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    if (m_station == nullptr || m_station->sensorList() == nullptr || role < SensorsListRole::NAME)
        return QVariant();

    const std::vector<SensorData>& sensors = m_station->sensorList()->sensors();
    const auto& it = sensors[row];

    switch (role)
    {
        case NAME:
            return QVariant(it.name);
        case VALUE:
        {
            if (it.value() == Connection::NoData) {
                return QVariant(tr("No data"));
            }

            Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
            float value = unitsConverter(UnitsType::MICROGRAM, static_cast<UnitsType>(settings->unitType()), it.value());
            return QVariant(QString::number(value, 'G', 5));
        }
    }

    return QVariant(tr("brak danych"));
}

QHash<int, QByteArray> SensorListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[NAME] = "name";
    names[VALUE] = "value";
    return names;
}

void SensorListModel::requestData()
{
    if (!m_station)
        return;

    Connection* connection = ProvidersManager::instance()->connection(m_station->provider());
    connection->sensorListRequest(m_station, [=](SensorListPtr sensorList) {
        setSensorList(sensorList, m_station);
    });
}

int SensorListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_station && m_station->sensorList())
        return m_station->sensorList()->size();

    return 0;
}

void SensorListModel::requestSensorData()
{
    Connection* connection = ProvidersManager::instance()->connection(m_station->provider());
    for (const auto& sensor:  m_station->sensorList()->sensors())
    {
        connection->sensorDataRequest(sensor, [this](SensorData sensorData) {
             m_station->sensorList()->setData(sensorData);
        });
    }
}

void SensorListModel::connectModel()
{
    if (m_station && m_station->sensorList())
        m_station->sensorList()->disconnect(this);

    if (m_station && m_station->sensorList())
    {
        connect(m_station->sensorList().get(), &SensorList::preItemAppended, this, [this]() {
            const int index = m_station->sensorList()->size();
            beginInsertRows(QModelIndex(), index, index);
        });

        connect(m_station->sensorList().get(), &SensorList::postItemAppended, this, [this]() {
            endInsertRows();
        });

        connect(m_station->sensorList().get(), &SensorList::valueChanged, this, [this](int index) {
            QModelIndex modelIndex = this->index(index);
            dataChanged(modelIndex, modelIndex, {SensorsListRole::VALUE});
        });
    }
}

void SensorListModel::setModelsManager(ModelsManager *modelsManager)
{
    m_modelsManager = modelsManager;
}

void SensorListModel::setSensorList(SensorListPtr sensorList, StationPtr station)
{
    if (!sensorList) {
        return;
    }

    station->setSensorList(sensorList);

    beginResetModel();
    connectModel();

    if (station->sensorList() == nullptr)
        return;

    requestSensorData();
    station->sensorList()->setDateToCurrent();
    endResetModel();
}

float SensorListModel::unitsConverter(SensorListModel::UnitsType from, SensorListModel::UnitsType to, float value) const
{
    int from_ = 1;
    int to_ = 1;

    switch (from)
    {
    case UnitsType::MICROGRAM: from_ = 1000000; break;
    case UnitsType::MILLIGRAM: from_ = 1000;    break;
    }

    switch (to)
    {
    case UnitsType::MICROGRAM: to_ = 1000000; break;
    case UnitsType::MILLIGRAM: to_ = 1000;    break;
    }

    return value * to_ / from_;
}

void SensorListModel::setStation(StationPtr station)
{
    beginResetModel();

    if (station == nullptr)
        return;

    m_station = station;

    if (m_modelsManager)
        requestData();

    endResetModel();
}
