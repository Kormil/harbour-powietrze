#ifndef SENSORLIST_H
#define SENSORLIST_H

#include <QString>
#include <QObject>
#include <QDateTime>
#include <QJsonDocument>
#include <QVariant>
#include <vector>
#include "pollution.h"

class Station;

class SensorList : public QObject
{
    Q_OBJECT

public:
    explicit SensorList(QObject *parent = nullptr);

    std::vector<Pollution> sensors() const;

    size_t size() const;
    void setData(Pollution sensorData);
    void setStation(Station *value);

    bool shouldGetNewData(int frequency);
    void setDateToCurrent();

    void waitForAll();
    bool isAll();

signals:
    void valueChanged(int index);
    void preItemAppended();
    void postItemAppended();
    void allLoaded();

private:
    QDateTime m_createTime;
    std::vector<Pollution> m_sensors;
    QDateTime m_date;
    Station* m_station;
};

using SensorListPtr = std::shared_ptr<SensorList>;

#endif // SENSORLIST_H
