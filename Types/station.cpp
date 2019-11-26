#include "station.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QtQml>
#include <QQuickView>
#include "src/providersmanager.h"

Station::Station(QObject *parent) :
    QObject(parent),
    m_sensorList(nullptr),
    m_favourite(false)
{

}

QString Station::name() const
{
    if (m_stationData.cityName.isEmpty())
        return m_stationData.street;
    else if (m_stationData.street.isEmpty())
        return m_stationData.cityName;
    else
        return m_stationData.cityName + ", " + m_stationData.street;
}

QString Station::cityName() const
{
   return m_stationData.cityName;
}

QString Station::streetName() const
{
    return m_stationData.street;
}

QString Station::province() const
{
    return m_stationData.province;
}

QString Station::country() const
{
    return m_stationData.country;
}

QGeoCoordinate Station::coordinate() const
{
    return m_stationData.coordinate;
}

bool Station::favourite() const
{
    return m_favourite;
}

void Station::setFavourite(bool value)
{
    m_favourite = value;
    emit dataChanged(id());
}

StationData Station::stationData() const
{
    return m_stationData;
}

void Station::setStationData(const StationData &stationData)
{
    m_stationData = stationData;
    emit stationDataChanged();
    emit nameChanged();
}

double Station::distance() const
{
    return m_distance;
}

QString Station::distanceString() const
{
    if (std::abs(distance()) < 0.000000001)
        return QString();

    return QString::number(distance() / 1000, 'f', 2);
}

int Station::provider() const
{
    return m_stationData.provider;
}

QString Station::providerName() const
{
    ProvidersManager* providersManager = ProvidersManager::instance();
    auto providerData = providersManager->provider(provider());

    return providerData->shortName();
}

void Station::setDistance(double distance)
{
    m_distance = distance;
    emit distanceChanged();
    emit dataChanged(hash());
}

StationIndex *Station::stationIndex() const
{
    return m_stationIndex.get();
}

StationIndexPtr Station::stationIndexPtr() const
{
    return m_stationIndex;
}

void Station::setStationIndex(StationIndexPtr stationIndex)
{
    m_stationIndex = std::move(stationIndex);
    m_stationIndex->setStation(this);
    emit stationIndexChanged();
}

void Station::bindToQml(QQuickView * view)
{
    qRegisterMetaTypeStreamOperators<StationData>("StationData");
    qmlRegisterType<Station>("StationListModel", 1, 0, "Station");

    StationIndex::bindToQml(view);
}

SensorListPtr Station::sensorList()
{
    return m_sensorList;
}

void Station::setSensorList(SensorListPtr sensorList)
{
    m_sensorList = sensorList;

    if (m_sensorList)
        m_sensorList->setStation(this);
}

int Station::id() const
{
    return m_stationData.id;
}
