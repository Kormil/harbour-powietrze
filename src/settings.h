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
    Q_PROPERTY(StationData lastViewStation READ lastViewStation WRITE setLastViewStation NOTIFY lastViewStationChanged)
public:
    static QObject *instance(QQmlEngine *engine, QJSEngine *scriptEngine);

    unsigned short unitType() const;
    void setUnitType(const unsigned short &value);
    Q_INVOKABLE QString unitName();
    Q_INVOKABLE QString unitName(unsigned short unitType);

    StationData lastViewStation() const;
    void setLastViewStation(const StationData& stationData);

    StationListPtr favouriteStations() const;
    Q_INVOKABLE void addFavouriteStation(Station *station);
    Q_INVOKABLE void removeFavouriteStation(int value);

    Q_INVOKABLE int updateFavouriteStations();
    Q_INVOKABLE void updateFavouriteStations(int time);


    Q_INVOKABLE QString license();
    Q_INVOKABLE QString version();

    static void bindToQml();
signals:
    void unitTypeChanged();
    void lastViewStationChanged();
    void favouriteStationsChanged();
    void updateFavouriteStationsChanged();

private:
    QVariantList favouriteStationsData() const;

    explicit Settings(QObject *parent = nullptr);
    QSettings *m_settings;
};

#endif // SETTINGS_H
