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
public:
    static GPSModule *instance();
    static void bindToQml(QQuickView * view);

    Q_INVOKABLE void requestPosition();
    void init();

    QGeoCoordinate lastKnowPosition() const;

private slots:
    void onPositionUpdate(const QGeoPositionInfo& positionInfo);
    void onGpsUpdateFrequencyChanged();
    void onUpdateTimeout();

signals:
    void shouldRequest();
    void positionRequested();
    void positionFounded(QGeoCoordinate coordinate);
    void positionUpdated(QGeoCoordinate coordinate);

private:
    explicit GPSModule(QObject *parent = nullptr);
    int frequencyFromSettings();

    QGeoPositionInfoSource* m_positionSource;
    QGeoCoordinate m_lastKnowPosition;
    QDateTime m_timeLastKnowPosition;

    QTimer m_timer;

    int m_minimumRequestIntervalInSec;
    int m_timerFrequency;
};

#endif // GPSMODULE_H
