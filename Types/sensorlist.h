#ifndef SENSORLIST_H
#define SENSORLIST_H

#include <QString>
#include <QObject>
#include <QDateTime>
#include <QJsonDocument>
#include <QVariant>
#include <vector>
#include <memory>
#include <numeric>
#include <atomic>

class Station;
class SensorList;

using SensorListPtr = std::shared_ptr<SensorList>;

struct SensorData
{
    QVariant id;
    QString name;
    QString pollutionCode;

    SensorData() {
        initialized = false;
    }

    SensorData(const SensorData& second) {
        id = second.id;
        name = second.name;
        pollutionCode = second.pollutionCode;
        values = second.values;
        initialized = second.isInitialized();
    }

    SensorData& operator=(const SensorData& second) {
        id = second.id;
        name = second.name;
        pollutionCode = second.pollutionCode;
        values = second.values;
        initialized = second.isInitialized();

        return *this;
    }

    float value() const {
        if (values.size())
            return values.front();

        return 0.0f;
    }

    float avg(size_t hours) const {
        hours = std::min(values.size(), hours);
        float sum = 0;

        for (size_t i = 0; i < hours; ++i) {
            sum += values[i];
        }

        return sum / hours;
    }

    bool isInitialized() const {
        return initialized;
    }


public:
    void setValues(float value);
    void setValues(const std::vector<float> &value);

private:
    std::atomic_bool initialized;
    std::vector<float> values;
};

class SensorList : public QObject
{
    Q_OBJECT

public:
    explicit SensorList(QObject *parent = nullptr);

    std::vector<SensorData> sensors() const;

    size_t size() const;
    void setData(SensorData sensorData);
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
    std::vector<SensorData> m_sensors;
    QDateTime m_date;
    Station* m_station;
};

#endif // SENSORLIST_H
