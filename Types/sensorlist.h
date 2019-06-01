#ifndef SENSORLIST_H
#define SENSORLIST_H

#include <QString>
#include <QObject>
#include <QDateTime>
#include <QJsonDocument>
#include <vector>
#include <memory>

class Station;
class SensorList;

using SensorListPtr = std::unique_ptr<SensorList>;

struct SensorData
{
    int id;
    QString name;
    float value = 0.0f;
};

class SensorList : public QObject
{
    Q_OBJECT

public:
    explicit SensorList(QObject *parent = nullptr);

    std::vector<SensorData> sensors() const;
    static SensorListPtr getSensorsFromJson(const QJsonDocument& jsonDocument);
    static float getSensorDataFromJson(const QJsonDocument &jsonDocument);

    size_t size() const;
    void setValue(int id, float value);
    void setStation(Station *value);

    bool shouldGetNewData(int frequency);
    void setDateToCurent();
signals:
    void valueChanged(int index);
    void preItemAppended();
    void postItemAppended();

private:
    QDateTime m_createTime;
    std::vector<SensorData> m_sensors;
    QDateTime m_date;
    Station* m_station;
};

#endif // SENSORLIST_H
