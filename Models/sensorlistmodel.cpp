#include "sensorlistmodel.h"
#include "src/connection.h"
#include "Types/station.h"
#include "src/settings.h"
#include <iostream>

SensorListModel::SensorListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_connection(nullptr),
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
            Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
            float value = unitsConverter(UnitsType::MICROGRAM, static_cast<UnitsType>(settings->unitType()), it.value);
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

void SensorListModel::requestData(Connection *connection)
{
    if (!m_station)
        return;

    connection->sensorListRequest(m_station->stationData().id, [=](SensorListPtr sensorList) {
        if (!sensorList) {
            return;
        }

        m_station->setSensorList(std::move(sensorList));

        beginResetModel();
        connectModel();

        if (m_station->sensorList() == nullptr)
            return;

        requestSensorData(connection);
        m_station->sensorList()->setDateToCurent();
        endResetModel();
    });
}

int SensorListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_station && m_station->sensorList())
        return m_station->sensorList()->size();

    return 0;
}

void SensorListModel::requestSensorData(Connection *connection)
{
    for (const auto& sensor:  m_station->sensorList()->sensors())
    {
        int sensorDataID = sensor.id;
        connection->sensorDataRequest(sensorDataID, [this, sensorDataID](float sensorValue) {
             m_station->sensorList()->setValue(sensorDataID, sensorValue);
        });
    }
}

void SensorListModel::connectModel()
{
    if (m_station && m_station->sensorList())
        m_station->sensorList()->disconnect(this);

    if (m_station && m_station->sensorList())
    {
        connect(m_station->sensorList(), &SensorList::preItemAppended, this, [this]() {
            const int index = m_station->sensorList()->size();
            beginInsertRows(QModelIndex(), index, index);
        });

        connect(m_station->sensorList(), &SensorList::postItemAppended, this, [this]() {
            endInsertRows();
        });

        connect(m_station->sensorList(), &SensorList::valueChanged, this, [this](int index) {
            QModelIndex modelIndex = this->index(index);
            dataChanged(modelIndex, modelIndex, {SensorsListRole::VALUE});
        });
    }
}

void SensorListModel::setConnection(Connection *value)
{
    m_connection = value;
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

void SensorListModel::setStation(Station *station)
{
    beginResetModel();

    if (station == nullptr)
        return;

    m_station = station;

    if (m_connection)
        requestData(m_connection);

    endResetModel();
}
