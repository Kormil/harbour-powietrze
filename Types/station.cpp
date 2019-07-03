#include "station.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QtQml>
#include <QQuickView>

Station::Station(QObject *parent) :
    QObject(parent),
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
    return QString::number(distance() / 1000, 'f', 2);
}

void Station::setDistance(double distance)
{
    m_distance = distance;
    emit distanceChanged();
    emit dataChanged(id());
}

StationIndex *Station::stationIndex() const
{
    return m_stationIndex.get();
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

SensorList* Station::sensorList()
{
    return m_sensorList.get();
}

void Station::setSensorList(SensorListPtr sensorList)
{
    m_sensorList = std::move(sensorList);

    if (m_sensorList != nullptr)
        m_sensorList->setStation(this);
}

int Station::id() const
{
    return m_stationData.id;
}

int StationIndex::id() const
{
    return m_id;
}

QString StationIndex::name() const
{
    return m_name;
}

StationIndexPtr StationIndex::getFromJson(const QJsonDocument &jsonDocument)
{
    if (jsonDocument.isNull())
        return StationIndexPtr(nullptr);

    QJsonObject object = jsonDocument.object()["stIndexLevel"].toObject();

    float id = object["id"].toInt();
    QString name = object["indexLevelName"].toString();

    StationIndex* stationIndex = new StationIndex;

    if (!name.isEmpty())
        stationIndex->setId(id);
    stationIndex->setName(name);
    return StationIndexPtr( stationIndex );
}

void StationIndex::bindToQml(QQuickView * view)
{
    Q_UNUSED(view);
    qmlRegisterType<Station>("StationListModel", 1, 0, "StationIndex");
}

void StationIndex::setId(int id)
{
    m_id = id;
    emit idChanged();
}

void StationIndex::setName(const QString &name)
{
    m_name = name;
    emit nameChanged();
}

bool StationIndex::shouldGetNewData(int frequency)
{
    if (m_station == nullptr)
        return false;

    QDateTime currentTime = QDateTime::currentDateTime();

    if (currentTime.time().hour() < m_date.time().hour())
        return true;

    QDateTime nextDataTime = m_date.addSecs( frequency * 60 );
    return currentTime > nextDataTime;
}

void StationIndex::setStation(Station *station)
{
    m_station = station;
}

void StationIndex::setDateToCurent()
{
    m_date = QDateTime::currentDateTime();
}
