#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariant>
#include "Types/station.h"
#include "Types/stationlist.h"

class QSettings;
class QJSEngine;
class QQmlEngine;

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(unsigned short unitType READ unitType WRITE setUnitType NOTIFY unitTypeChanged)
    Q_PROPERTY(unsigned short gpsUpdateFrequency READ gpsUpdateFrequency WRITE setGpsUpdateFrequency NOTIFY gpsUpdateFrequencyChanged)
public:
    static QObject *instance(QQmlEngine *engine, QJSEngine *scriptEngine);

    unsigned short unitType() const;
    void setUnitType(const unsigned short &value);
    Q_INVOKABLE QString unitName();
    Q_INVOKABLE QString unitName(unsigned short unitType);

    StationListPtr favouriteStations() const;
    Q_INVOKABLE void addFavouriteStation(Station *station);
    Q_INVOKABLE void removeFavouriteStation(int value);

    Q_INVOKABLE int updateFavouriteStations();
    Q_INVOKABLE void updateFavouriteStations(int time);

    Q_INVOKABLE QString license();
    Q_INVOKABLE QString version();

    Q_INVOKABLE unsigned short gpsUpdateFrequency();
    Q_INVOKABLE void setGpsUpdateFrequency(unsigned short unitType);

    static void bindToQml();
signals:
    void unitTypeChanged();
    void lastViewStationChanged();
    void favouriteStationsChanged();
    void updateFavouriteStationsChanged();
    void gpsUpdateFrequencyChanged();

private:
    QVariantList favouriteStationsData() const;

    explicit Settings(QObject *parent = nullptr);
    QSettings *m_settings;
};

#endif // SETTINGS_H
