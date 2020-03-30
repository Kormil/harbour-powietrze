#include "settings.h"
#include <iostream>
#include <QSettings>
#include <QtQml>
#include <QVariant>
#include <sailfishapp.h>

#define FAVOURITE_STATIONS QStringLiteral("stations/favourite")
#define NOTIFICATIONS_STATIONS QStringLiteral("stations/notifications")
#define UPDATE_FAVOURITE_STATIONS QStringLiteral("update/favouriteStations")
#define GPS_FREQUENCY QStringLiteral("gps/frequency")
#define GPS_PAUSED QStringLiteral("gps/paused")

Settings::Settings(QObject *parent) :
    m_settings(new QSettings(parent))
{

}

QObject *Settings::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    static Settings instance;
    return &instance;
}

QVariantList Settings::favouriteStationsData() const
{
    return m_settings->value(FAVOURITE_STATIONS).toList();
}

void Settings::addFavouriteStation(Station* station)
{
    int id = station->id();
    QVariantList stations = favouriteStationsData();

    if (stations.end() == std::find_if(stations.begin(), stations.end(), [id](const QVariant& item) {
        return item.value<StationData>().id == id;
    }))
    {
        stations.push_back(QVariant::fromValue(station->stationData()));
        m_settings->setValue(FAVOURITE_STATIONS, stations);
        emit favouriteStationsChanged();
    }
}

void Settings::removeFavouriteStation(int value)
{
    QVariantList stations = favouriteStationsData();

    auto station = std::find_if(stations.begin(), stations.end(), [value](const QVariant& item) {
        return item.value<StationData>().id == value;
    });

    if (stations.end() != station)
    {
        stations.erase(station);
        m_settings->setValue(FAVOURITE_STATIONS, stations);
        emit favouriteStationsChanged();
    }
}

int Settings::updateFavouriteStations()
{
    auto minutesToMicroseconds = [](int minutes) {
        return minutes * 60 * 1000;
    };
    return minutesToMicroseconds(m_settings->value(UPDATE_FAVOURITE_STATIONS, 240).toInt());
}

void Settings::updateFavouriteStations(int time)
{
    auto microsecondsToMinutes = [](int microseconds) {
        return microseconds / 60 / 1000;
    };

    m_settings->setValue(UPDATE_FAVOURITE_STATIONS, QVariant::fromValue(microsecondsToMinutes(time)));
    emit updateFavouriteStationsChanged();
}

QString Settings::license()
{
    QString licenseFile = SailfishApp::pathTo(QString("LICENSE")).toLocalFile();

    if (!QFile::exists(licenseFile)) {
        return "License not found.";
    }

    QFile file(licenseFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "Could not open: " + licenseFile;

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    return content;
}

QString Settings::providerLicense(QString providerName)
{
    QString licenseFile = SailfishApp::pathTo(QString("Licenses/") + providerName).toLocalFile();

    if (!QFile::exists(licenseFile)) {
        return "License not found.";
    }

    QFile file(licenseFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "Could not open: " + licenseFile;

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    return content;
}

void Settings::setGpsUpdateFrequency(unsigned short gpsFrequency)
{
    if (gpsUpdateFrequency() != gpsFrequency)
    {
        m_settings->setValue(GPS_FREQUENCY, gpsFrequency);
        emit gpsUpdateFrequencyChanged();
    }
}

QVariant Settings::providerSettings(QString name, QString key)
{
    QString path = "providers/" + name + "/" + key;
    return m_settings->value(path);
}

void Settings::setProviderSettings(QString name, QString key, QVariant value)
{
    if (providerSettings(name, key) != value)
    {
        QString path = "providers/" + name + "/" + key;
        m_settings->setValue(path, value);
        emit providerSettingsChanged(name, key);
    }
}

bool Settings::notifications() const
{
    return m_settings->value(NOTIFICATIONS_STATIONS, true).toBool();
}

void Settings::setNotifications(const bool &value)
{
    if (notifications() != value)
    {
        m_settings->setValue(NOTIFICATIONS_STATIONS, value);
        emit notificationsChanged();
    }
}

QDateTime Settings::gpsLocationPaused() const
{
    return m_settings->value(GPS_PAUSED, QDateTime::currentDateTime()).toDateTime();
}

void Settings::setGpsLocationPaused(const QDateTime &value)
{
    if (gpsLocationPaused() != value)
    {
        m_settings->setValue(GPS_PAUSED, value);
    }
}

unsigned short Settings::gpsUpdateFrequency()
{
    return m_settings->value(GPS_FREQUENCY, 3).toInt();
}

void Settings::bindToQml()
{
    qmlRegisterSingletonType<Settings>("Settings", 1, 0, "Settings", Settings::instance);
}

StationListPtr Settings::favouriteStations() const
{
    QVariantList stations = favouriteStationsData();
    StationListPtr stationList(new StationList());

    for (auto& stationData: stations)
    {
        StationPtr station = StationPtr(new Station());
        station->setStationData(stationData.value<StationData>());
        station->setFavourite(true);

        stationList->append(station);
    }

    return std::move(stationList);
}
