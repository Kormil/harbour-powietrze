#include "sensorlistmodel.h"
#include "src/connection/connection.h"
#include "src/modelsmanager.h"
#include "src/providersmanager.h"
#include "Types/station.h"
#include "src/settings.h"
#include "src/utils.h"

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

    const std::vector<Pollution>& sensors = m_station->sensorList()->sensors();
    const auto& it = sensors[row];

    int pollutionNameVariant = m_modelsManager->providerListModel()->provider(m_station->provider())->nameVariant();

    QString noData = tr("No data");
    switch (role)
    {
        case NAME: {
            if (pollutionNameVariant == 0) {
                return QVariant(it.name);
            } else {
                return QVariant(it.code);
            }
        }
        case VALUE:
        {
            if (it.value() == Connection::NoData) {
                return QVariant(noData);
            }

            float value = it.value();
            return QVariant(QString::number(value, 'f', 2));
        }
        case DATE:
        {
            if (QDate::currentDate() > it.date.date()) {
                return QVariant(it.date.toString("HH:mm dd-MM-yy"));
            } else {
                return QVariant(it.date.toString("HH:mm"));
            }
        }
        case NORM:
        {
            float value = Utils::calculateWHONorms(it);
            return QVariant(QString::number(value, 'f', 0));
        }
        case UNIT:
        {
            return QVariant(it.unit);
        }
    }

    return QVariant(noData);
}

QHash<int, QByteArray> SensorListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[NAME] = "name";
    names[VALUE] = "value";
    names[DATE] = "date";
    names[NORM] = "norm";
    names[UNIT] = "unit";
    return names;
}

void SensorListModel::requestData()
{
    if (!m_station)
        return;

    auto station = m_station;
    Connection* connection = ProvidersManager::instance()->connection(m_station->provider());
    connection->getSensorList(station, [=](SensorListPtr sensorList) {
        if (!sensorList) {
            return;
        }

        setSensorList(sensorList, station);
        requestSensorData(station);
    });
}

int SensorListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_station && m_station->sensorList())
        return m_station->sensorList()->size();

    return 0;
}

void SensorListModel::requestSensorData(StationPtr station)
{
    if (!station || !station->sensorList())
        return;

    Connection* connection = ProvidersManager::instance()->connection(station->provider());
    for (const auto& sensor:  station->sensorList()->sensors())
    {
        connection->getSensorData(sensor, [station](Pollution sensorData) {
             station->sensorList()->setData(sensorData);
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
            dataChanged(modelIndex, modelIndex, {SensorsListRole::NAME,
                                                 SensorsListRole::VALUE,
                                                 SensorsListRole::DATE,
                                                 SensorsListRole::NORM,
                                                 SensorsListRole::UNIT});
        });
    }
}

void SensorListModel::setModelsManager(ModelsManager *modelsManager)
{
    m_modelsManager = modelsManager;
}

void SensorListModel::setSensorList(SensorListPtr sensorList, StationPtr station)
{
    if (!sensorList || !station) {
        return;
    }

    station->setSensorList(sensorList);

    beginResetModel();
    connectModel();

    if (station->sensorList() == nullptr)
        return;

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

    if (m_modelsManager) {
        requestData();
    }

    endResetModel();
}
