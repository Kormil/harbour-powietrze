#include "stationlist.h"
#include <list>

StationList::StationList(QObject *parent) : QObject(parent)
{
}

bool StationList::setItemAt(unsigned int index, Station *station)
{
    if (index > m_stations.size())
        return false;

    m_stations[index] = StationPtr(station);
    m_idToRow[station->id()] = index;
    return true;
}

Station* StationList::station(int index)
{
    return m_stations[index].get();
}

size_t StationList::size() const
{
    return m_stations.size();
}

void StationList::setStations(std::vector<StationPtr> &stations)
{
    m_stations = std::move(stations);
    m_idToRow.clear();

    for (unsigned int i = 0; i < m_stations.size(); ++i)
    {
        m_idToRow[m_stations[i]->id()] = i;
    }
}

StationListPtr StationList::getFromJson(const QJsonDocument &jsonDocument)
{
    StationListPtr stationList(new StationList());

    QJsonArray array = jsonDocument.array();

    for (const auto& station: array)
    {
        Station* item = new Station();
        StationData stationData;
        stationData.id = station.toObject()["id"].toInt();
        stationData.cityName = station.toObject()["city"].toObject()["name"].toString();
        stationData.street = station.toObject()["addressStreet"].toString();

        double lat = station.toObject()["gegrLat"].toString().toDouble();
        double lon = station.toObject()["gegrLon"].toString().toDouble();
        stationData.coordinate = QGeoCoordinate(lat, lon);

        stationData.province = station.toObject()["city"].toObject()["commune"].toObject()["provinceName"].toString();

        if (stationData.province.isNull())
            stationData.province = tr("OTHER");

        item->setStationData(stationData);
        stationList->append(item);
    }

    return std::move(stationList);
}


void StationList::append(Station* station)
{
    append(std::move(StationPtr(station)));
}

void StationList::append(StationPtr station)
{
    if (station == nullptr)
        return;

    auto idAndRow = m_idToRow.find(station->id());

    if (m_idToRow.end() == idAndRow)
    {
        emit preItemAppended();

        m_idToRow[station->id()] = m_stations.size();
        m_stations.push_back(std::move(station));

        connect(m_stations.back().get(), &Station::dataChanged, this, &StationList::onItemChanged);
        emit postItemAppended();
    }
    else
    {
        station->setStationData(m_stations[idAndRow->second]->stationData());
        m_stations[idAndRow->second] = std::move(station);
        connect(m_stations[idAndRow->second].get(), &Station::dataChanged, this, &StationList::onItemChanged);
        emit itemChanged(idAndRow->second);
    }
}

void StationList::appendList(StationListPtr& stationList)
{
    for (auto& station: stationList->m_stations)
    {
        append(std::move(station));
    }

    stationList = nullptr;
}

Station* StationList::find(int stationId)
{
    int index = row(stationId);

    if (index == -1)
        return nullptr;

    return m_stations[index].get();
}

void StationList::findDistances(QGeoCoordinate coordinate)
{
    for (const auto& station: m_stations)
    {
        double distance = station->coordinate().distanceTo(coordinate);
        station->setDistance(distance);
    }
}

int StationList::row(int stationId) const
{
    auto idAndRow = m_idToRow.find(stationId);
    if (m_idToRow.end() == idAndRow)
        return -1;

    return idAndRow->second;
}

std::vector<int> StationList::favouriteIds() const
{
    std::vector<int> favouriteStations;

    for (const auto & station: m_stations)
    {
        if (station->favourite())
            favouriteStations.push_back(station->id());
    }

    return favouriteStations;
}

void StationList::onItemChanged(int id)
{
    auto idAndRow = m_idToRow.find(id);
    emit itemChanged(idAndRow->second);
}
