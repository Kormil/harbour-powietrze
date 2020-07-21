#ifndef SETTINGS_H
#define SETTINGS_H

#include <memory>
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
    Q_PROPERTY(bool notifications READ notifications WRITE setNotifications NOTIFY notificationsChanged)
    Q_PROPERTY(unsigned short gpsUpdateFrequency READ gpsUpdateFrequency WRITE setGpsUpdateFrequency NOTIFY gpsUpdateFrequencyChanged)
public:
    static QObject *instance(QQmlEngine *engine, QJSEngine *scriptEngine);

    StationListPtr favouriteStations() const;
    Q_INVOKABLE void addFavouriteStation(Station *station);
    Q_INVOKABLE void removeFavouriteStation(int value);

    Q_INVOKABLE int updateFavouriteStations();
    Q_INVOKABLE void updateFavouriteStations(int time);

    Q_INVOKABLE QString license();
    Q_INVOKABLE QString providerLicense(QString providerName);

    Q_INVOKABLE unsigned short gpsUpdateFrequency();
    Q_INVOKABLE void setGpsUpdateFrequency(unsigned short gpsFrequency);

    Q_INVOKABLE QVariant providerSettings(QString name, QString key);
    Q_INVOKABLE void setProviderSettings(QString name, QString key, QVariant value);

    bool notifications() const;
    void setNotifications(const bool &value);

    QDateTime gpsLocationPaused() const;
    void setGpsLocationPaused(const QDateTime &value);

    static void bindToQml();

signals:
    void lastViewStationChanged();
    void favouriteStationsChanged();
    void updateFavouriteStationsChanged();
    void gpsUpdateFrequencyChanged();
    void notificationsChanged();
    void providerSettingsChanged(QString, QString);

private:
    QVariantList favouriteStationsData() const;

    explicit Settings(QObject *parent = nullptr);
    std::unique_ptr<QSettings> m_settings;
};

#endif // SETTINGS_H
