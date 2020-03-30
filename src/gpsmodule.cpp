#include "gpsmodule.h"
#include <iostream>
#include <QtPositioning/QNmeaPositionInfoSource>
#include <QFile>
#include "settings.h"

namespace {
    const int firstAccuracyThreshold = 256; //in meters
    const int validPositionThreshold = 64; //in meters
}

GPSModule *GPSModule::instance()
{
    static GPSModule instance;
    return &instance;
}

void GPSModule::bindToQml(QQuickView *view)
{
    qmlRegisterUncreatableType<GPSModule>("GPSModule", 1, 0, "gps", "GPSModule is singleton");
    view->rootContext()->setContextProperty(QStringLiteral("gps"), GPSModule::instance());
}

void GPSModule::requestPosition()
{
    if (!m_positionSource)
        return;

    QDateTime currentTime = QDateTime::currentDateTime();
    emit positionRequested();

    if (currentTime >= m_timeLastKnowPosition.addSecs(m_minimumRequestIntervalInSec)) {
        std::cout << "GPS REQUEST" << std::endl;
        m_positionSource->startUpdates();
    } else {
        m_lastKnowPosition = m_positionSource->lastKnownPosition().coordinate();
        emit positionUpdated(m_lastKnowPosition);
    }
}

void GPSModule::stopLocating()
{
    if (m_positionSource) {
        m_positionSource->stopUpdates();
    }

    m_timeLastKnowPosition = QDateTime::currentDateTime();
    emit positionUpdated(m_lastKnowPosition);
}

void GPSModule::pauseLocating(int hours)
{
    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));

    if (hours == -1) {
        settings->setGpsUpdateFrequency(0);
        return;
    }

    m_pausedLocatingDateTime = QDateTime::currentDateTime().addSecs(hours * 60 * 60);
    stopLocating();

    settings->setGpsLocationPaused(m_pausedLocatingDateTime);
    emit pausedChanged();
}

bool GPSModule::isPaused()
{
    return QDateTime::currentDateTime() < m_pausedLocatingDateTime;
}

void GPSModule::removePauseFlag(bool)
{
    m_pausedLocatingDateTime = QDateTime::currentDateTime();
    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
    settings->setGpsLocationPaused(m_pausedLocatingDateTime);
}

void GPSModule::onPositionUpdate(const QGeoPositionInfo &positionInfo)
{
    std::cout << "GPS position: " << positionInfo.coordinate().latitude() << " : " << positionInfo.coordinate().longitude() << std::endl;

    if (!positionInfo.isValid())
        return;

    int horizontalAccuracy = positionInfo.attribute(QGeoPositionInfo::HorizontalAccuracy);
    int verticalAccuracy = positionInfo.attribute(QGeoPositionInfo::VerticalAccuracy);

    if (horizontalAccuracy <= firstAccuracyThreshold && verticalAccuracy <= firstAccuracyThreshold)
    {
        m_timeLastKnowPosition = QDateTime::currentDateTime();
        m_lastKnowPosition = positionInfo.coordinate();

        m_positionSource->stopUpdates();

        emit positionUpdated(m_lastKnowPosition);

        std::cout << "GPS FOUNDED" << std::endl;
    }
}

void GPSModule::onGpsUpdateFrequencyChanged()
{
    m_minimumRequestIntervalInSec = frequencyFromSettings() * 60;
    if (m_minimumRequestIntervalInSec) {

        QDateTime currentTime = QDateTime::currentDateTime();
        if (!m_timeLastKnowPosition.isValid() &&
                currentTime >= m_timeLastKnowPosition.addSecs(m_minimumRequestIntervalInSec)) {
            emit shouldRequest();
        }

        m_timer.start(60 * 1000);
    } else {
        m_positionSource->stopUpdates();

        emit positionUpdated(QGeoCoordinate());

        m_timer.stop();
    }
}

void GPSModule::onUpdateTimeout()
{
    std::cout << "GPS TIMEOUT" << std::endl;

    if (m_lastKnowPosition.isValid()) {
        m_positionSource->stopUpdates();
        emit positionUpdated(m_lastKnowPosition);
    }
}

GPSModule::GPSModule(QObject *parent) :
    QObject(parent)
{
    init();
    std::cout << "GPS created" << std::endl;
}

int GPSModule::frequencyFromSettings()
{
    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));

    switch (settings->gpsUpdateFrequency())
    {
    case 0: return 0; break;
    case 1: return 30; break;
    case 2: return 60; break;
    case 3: return 120; break;
    case 4: return 240; break;
    }

    return 0;
}

QGeoCoordinate GPSModule::lastKnowPosition() const
{
    return m_lastKnowPosition;
}

void GPSModule::stop()
{
    if (m_positionSource)
        m_positionSource->stopUpdates();

    m_timer.stop();

    onUpdateTimeout();

    std::cout << "GPS stopped" << std::endl;
}

void GPSModule::init()
{
    m_minimumRequestIntervalInSec = frequencyFromSettings() * 60;

    m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);

    if (!m_positionSource) {
        return;
    }

    QObject::connect(m_positionSource, &QGeoPositionInfoSource::positionUpdated, this, &GPSModule::onPositionUpdate);
    QObject::connect(m_positionSource, &QGeoPositionInfoSource::updateTimeout, this, &GPSModule::onUpdateTimeout);

    m_positionSource->setUpdateInterval(1000);

    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
    QObject::connect(settings, &Settings::gpsUpdateFrequencyChanged, this, &GPSModule::onGpsUpdateFrequencyChanged);

    m_pausedLocatingDateTime = settings->gpsLocationPaused();
    onGpsUpdateFrequencyChanged();

    QObject::connect(&m_timer, &QTimer::timeout, [this]() {
        if (!isPaused() &&
            QDateTime::currentDateTime() >= m_timeLastKnowPosition.addSecs(m_minimumRequestIntervalInSec))
            emit shouldRequest();
    });
}

bool GPSModule::knowAnyPosition()
{
    return m_lastKnowPosition.isValid();
}
