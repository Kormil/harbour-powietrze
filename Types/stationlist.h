#ifndef STATIONLIST_H
#define STATIONLIST_H

#include <QObject>
#include <string>
#include <unordered_map>
#include <memory>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "station.h"

class StationList;

using StationListPtr = std::unique_ptr<StationList>;

class StationList : public QObject
{
    Q_OBJECT
public:
    explicit StationList(QObject *parent = nullptr);

    bool setItemAt(unsigned int index, Station *station);

    size_t size() const;

    Station* station(int index);

    void setStations(std::vector<StationPtr> &stations);

    static StationListPtr getFromJson(const QJsonDocument& jsonDocument);

    void append(Station* station);
    void append(StationPtr station);
    void appendList(StationListPtr &stationList);

    Station* find(int stationId);
    void findDistances(QGeoCoordinate coordinate);
    int row(int stationId) const;

    std::vector<int> favouriteIds() const;
signals:
    void preItemAppended();
    void postItemAppended();
    void itemChanged(int);

private slots:
    void onItemChanged(int id);

private:
    std::vector<StationPtr> m_stations;
    std::map<int, int> m_idToRow;
};

#endif // STATIONLIST_H
