#ifndef STATION_H
#define STATION_H

#include <QObject>
#include <memory>
#include "sensorlist.h"
#include "stationdata.h"

class QQuickView;

class Station;
class StationIndex;

using StationPtr = std::shared_ptr<Station>;
using StationIndexPtr = std::unique_ptr<StationIndex>;

struct StationIndex : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id NOTIFY idChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
public:

    int id() const;
    QString name() const;

    static StationIndexPtr getFromJson(const QJsonDocument &jsonDocument);
    static void bindToQml(QQuickView * view);

    void setId(int id);
    void setName(const QString &name);

    bool shouldGetNewData(int frequency);
    void setDateToCurent();
    void setStation(Station *station);

signals:
    void idChanged();
    void nameChanged();

private:
    int m_id = -1;
    QString m_name;
    Station* m_station;
    QDateTime m_date;
};

class Station : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString cityName READ cityName NOTIFY stationDataChanged)
    Q_PROPERTY(QString streetName READ streetName NOTIFY stationDataChanged)
    Q_PROPERTY(QString distance READ distanceString NOTIFY distanceChanged)
    Q_PROPERTY(int id READ id)
    Q_PROPERTY(bool favourite READ favourite WRITE setFavourite)
    Q_PROPERTY(StationIndex* stationIndex READ stationIndex NOTIFY stationIndexChanged)
    Q_PROPERTY(StationData stationData READ stationData NOTIFY stationDataChanged)
public:
    explicit Station(QObject *parent = nullptr);
    virtual ~Station() = default;

    StationIndex* stationIndex() const;
    void setStationIndex(StationIndexPtr stationIndex);
    static void bindToQml(QQuickView *view);

    SensorList *sensorList();
    void setSensorList(SensorListPtr thissensorList);

    int id() const;
    QString name() const;
    QString cityName() const;
    QString streetName() const;
    QString province() const;
    QGeoCoordinate coordinate() const;

    bool favourite() const;
    void setFavourite(bool value);

    StationData stationData() const;
    void setStationData(const StationData &stationData);

    double distance() const;
    void setDistance(double distance);
    QString distanceString() const;

signals:
    void stationIndexChanged();
    void stationDataChanged();
    void nameChanged();
    void dataChanged(int);
    void distanceChanged();

private:
    SensorListPtr m_sensorList;
    StationIndexPtr m_stationIndex;

    StationData m_stationData;

    bool m_favourite;
    double m_distance = 0.f;
};

#endif // STATION_H
