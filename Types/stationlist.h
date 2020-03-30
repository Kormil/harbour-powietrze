#ifndef STATIONLIST_H
#define STATIONLIST_H

#include <QObject>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "station.h"

class StationList;
using StationListPtr = std::shared_ptr<StationList>;

class StationList : public QObject
{
    Q_OBJECT
    using Hash = unsigned int;
public:
    explicit StationList(QObject *parent = nullptr);
    size_t size() const;

    StationPtr station(unsigned int index);
    StationPtr find(unsigned int hash);

    void append(StationPtr station);
    void appendList(StationListPtr &stationList);

    StationPtr findNearest();
    void calculateDistances(QGeoCoordinate coordinate);
    int row(int hash) const;

    std::vector<Hash> favourites() const;
signals:
    void preItemAppended();
    void postItemAppended();
    void itemChanged(int);

private slots:
    void onItemChanged(int hash);

private:
    std::vector<StationPtr> m_stations;
    std::map<Hash, unsigned int> m_hashToRow;

    std::mutex m_appendStationMutex;
};

#endif // STATIONLIST_H
