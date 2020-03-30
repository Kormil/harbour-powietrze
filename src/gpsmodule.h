#ifndef GPSMODULE_H
#define GPSMODULE_H

#include <QObject>
#include <QDateTime>
#include <QQuickView>
#include <QtQml>
#include <QtPositioning/QGeoPositionInfoSource>
#include <QtPositioning/QGeoPositionInfo>

class GPSModule;


class GPSModule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool paused READ isPaused WRITE removePauseFlag NOTIFY pausedChanged)
    Q_PROPERTY(bool knowAnyPosition READ knowAnyPosition NOTIFY positionUpdated)
public:
    static GPSModule *instance();
    static void bindToQml(QQuickView * view);

    Q_INVOKABLE void requestPosition();
    Q_INVOKABLE void stopLocating();
    Q_INVOKABLE void pauseLocating(int hours);
    bool isPaused();
    void removePauseFlag(bool);
    void init();
    bool knowAnyPosition();

    QGeoCoordinate lastKnowPosition() const;

    void stop();

private slots:
    void onPositionUpdate(const QGeoPositionInfo& positionInfo);
    void onGpsUpdateFrequencyChanged();
    void onUpdateTimeout();

signals:
    void shouldRequest();
    void positionRequested();
    void positionUpdated(QGeoCoordinate coordinate);
    void pausedChanged();

private:
    explicit GPSModule(QObject *parent = nullptr);
    int frequencyFromSettings();

    QGeoPositionInfoSource* m_positionSource;
    QGeoCoordinate m_lastKnowPosition;
    QDateTime m_timeLastKnowPosition;

    QTimer m_timer;

    int m_minimumRequestIntervalInSec;
    QDateTime m_pausedLocatingDateTime;
};

#endif // GPSMODULE_H
