#include "stationlist.h"
#include <list>
#include <iostream>

StationList::StationList(QObject *parent) : QObject(parent)
{
}

StationPtr StationList::station(int index)
{
    if (m_stations.size() < index)
        return nullptr;

    return m_stations[index];
}

StationPtr StationList::find(unsigned int hash)
{
    return m_stations[m_hashToRow[hash]];
}

size_t StationList::size() const
{
    return m_stations.size();
}

void StationList::append(StationPtr station)
{
    if (station == nullptr)
        return;

    std::lock_guard<std::mutex> guard(m_appendStationMutex);

    auto hash = station->hash();
    auto hashAndRow = m_hashToRow.find(hash);

    if (m_hashToRow.end() == hashAndRow)
    {
        emit preItemAppended();

        m_hashToRow[hash] = m_stations.size();
        m_stations.push_back(station);

        connect(station.get(), &Station::dataChanged, this, &StationList::onItemChanged);
        emit postItemAppended();
    } else if (station->distance()) {
        m_stations[hashAndRow->second]->setDistance(station->distance());
    }
}

void StationList::appendList(StationListPtr& stationList)
{
    for (auto& station: stationList->m_stations)
    {
        append(station);
    }

    stationList = nullptr;
}

StationPtr StationList::findNearest()
{
    if (m_stations.size() == 0) {
        return nullptr;
    }

    StationPtr nearest = m_stations.front();
    for (auto& station: m_stations)
    {
        if (station->distance() && station->distance() < nearest->distance()) {
            nearest = station;
        }
    }

    return nearest;
}

void StationList::calculateDistances(QGeoCoordinate coordinate)
{
    for (const auto& station: m_stations)
    {
        double distance = station->coordinate().distanceTo(coordinate);
        station->setDistance(distance);
    }
}

int StationList::row(int hash) const
{
    auto idAndRow = m_hashToRow.find(hash);
    if (m_hashToRow.end() == idAndRow)
        return -1;

    return idAndRow->second;
}

std::vector<StationList::Hash> StationList::favourites() const
{
    std::vector<Hash> favouriteStations;

    for (const auto & station: m_stations)
    {
        if (station->favourite())
            favouriteStations.push_back(station->hash());
    }

    return favouriteStations;
}

void StationList::onItemChanged(int hash)
{
    auto hashAndRow = m_hashToRow.find(hash);
    emit itemChanged(hashAndRow->second);
}
