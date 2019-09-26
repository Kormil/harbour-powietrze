#ifndef STATION_H
#define STATION_H

#include <QObject>
#include <memory>
#include "sensorlist.h"
#include "stationdata.h"
#include "stationindex.h"

class QQuickView;

class Station;
using StationPtr = std::shared_ptr<Station>;

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
    Q_PROPERTY(int provider READ provider NOTIFY dataChanged)
public:
    explicit Station(QObject *parent = nullptr);
    virtual ~Station() = default;

    StationIndex* stationIndex() const;
    StationIndexPtr stationIndexPtr() const;
    void setStationIndex(StationIndexPtr stationIndex);
    static void bindToQml(QQuickView *view);

    SensorListPtr sensorList();
    void setSensorList(SensorListPtr sensorList);

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

    int provider() const;

    unsigned int hash() const {
        return qHash(name() % QString(m_stationData.provider));
    }

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
