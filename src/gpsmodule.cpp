#include "gpsmodule.h"
#include <iostream>
#include <QtPositioning/QNmeaPositionInfoSource>
#include <QFile>
#include "settings.h"

namespace {
    const int firstAccuracyThreshold = 200; //in meters
    const int secondAccuracyThreshold = 75; //in meters
    const int validPositionThreshold = 50; //in meters
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

    if (currentTime >= m_timeLastKnowPosition.addSecs(m_minimumRequestIntervalInSec))
        m_positionSource->startUpdates();
    else
    {
        m_lastKnowPosition = m_positionSource->lastKnownPosition().coordinate();
        emit positionUpdated(m_lastKnowPosition);
        emit positionFounded(m_lastKnowPosition);
    }
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
        bool isPositionFounded = false;

        if (m_lastKnowPosition.isValid() &&
                positionInfo.coordinate().distanceTo(m_lastKnowPosition) < validPositionThreshold)
        {
            isPositionFounded = true;
        }

        if (horizontalAccuracy <= secondAccuracyThreshold && verticalAccuracy <= secondAccuracyThreshold)
        {
            isPositionFounded = true;
            m_lastKnowPosition = positionInfo.coordinate();
        }

        if (isPositionFounded)
        {
            m_timeLastKnowPosition = QDateTime::currentDateTime();
            m_positionSource->stopUpdates();

            emit positionUpdated(m_lastKnowPosition);
            emit positionFounded(m_lastKnowPosition);

            std::cout << "GPS FOUNDED" << std::endl;
        } else {
            emit positionUpdated(positionInfo.coordinate());
        }
    }
}

void GPSModule::onGpsUpdateFrequencyChanged()
{
    int timerFrequency = frequencyFromSettings();
    if (timerFrequency) {

        QDateTime currentTime = QDateTime::currentDateTime();
        if (!m_timeLastKnowPosition.isValid() &&
                currentTime >= m_timeLastKnowPosition.addSecs(timerFrequency * 60)) {
            emit shouldRequest();
        }

        m_timer.start(timerFrequency * 60 * 1000);
    } else {
        m_positionSource->stopUpdates();

        emit positionUpdated(QGeoCoordinate());
        emit positionFounded(QGeoCoordinate());

        m_timer.stop();
    }
}

void GPSModule::onUpdateTimeout()
{
    std::cout << "GPS TIMEOUT" << std::endl;

    if (m_lastKnowPosition.isValid())
    {
        m_positionSource->stopUpdates();

        emit positionUpdated(m_lastKnowPosition);
        emit positionFounded(m_lastKnowPosition);
    }
}

GPSModule::GPSModule(QObject *parent) :
    QObject(parent)
{
    init();

    QObject::connect(&m_timer, &QTimer::timeout, this, &GPSModule::shouldRequest);
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

void GPSModule::init()
{
    m_minimumRequestIntervalInSec = 300; //five minutes

    m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);

    QObject::connect(m_positionSource, &QNmeaPositionInfoSource::positionUpdated, this, &GPSModule::onPositionUpdate);
    QObject::connect(m_positionSource, &QNmeaPositionInfoSource::updateTimeout, this, &GPSModule::onUpdateTimeout);

    m_positionSource->setUpdateInterval(1000);

    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
    QObject::connect(settings, &Settings::gpsUpdateFrequencyChanged, this, &GPSModule::onGpsUpdateFrequencyChanged);

    onGpsUpdateFrequencyChanged();
}
