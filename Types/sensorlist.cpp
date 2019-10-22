#include "sensorlist.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include "station.h"

SensorList::SensorList(QObject *parent) :
    QObject(parent)
{
}

std::vector<SensorData> SensorList::sensors() const
{
    return m_sensors;
}

size_t SensorList::size() const
{
    return m_sensors.size();
}

bool SensorList::isAll()
{
    bool allDone = true;
    for (const auto& sensor: m_sensors) {
        if (!sensor.isInitialized()) {
            allDone = false;
            break;
        }
    }

    return allDone;
}

void SensorList::setData(SensorData sensorData)
{
    if (sensorData.id.isNull() || sensorData.name.isEmpty())
        return;

    auto sensorIt = std::find_if(m_sensors.begin(), m_sensors.end(), [&sensorData](const SensorData& b) {
        if (sensorData.name == b.name)
            return true;

        return false;
    });

    int index;

    if (sensorIt == m_sensors.end()) {
        index = m_sensors.size();
        m_sensors.push_back(sensorData);
    } else {
        index = std::distance(m_sensors.begin(), sensorIt);
        *sensorIt = sensorData;
    }

    emit valueChanged(index);

    if (isAll())
        emit allLoaded();
}

void SensorList::setStation(Station *value)
{
    m_station = value;
}

bool SensorList::shouldGetNewData(int frequency)
{
    if (m_station == nullptr)
        return false;

    QDateTime currentTime = QDateTime::currentDateTime();

    if (currentTime.time().hour() < m_date.time().hour())
        return true;

    QDateTime nextDataTime = m_date.addSecs(frequency * 60);
    return currentTime > nextDataTime;
}

void SensorList::setDateToCurrent()
{
    m_date = QDateTime::currentDateTime();
}

void SensorList::waitForAll()
{
    for (const auto& sensor: m_sensors) {
        while (!sensor.isInitialized()) {}
    }
}

void SensorData::setValues(float value)
{
    values.push_back(value);
    initialized = true;
}

void SensorData::setValues(const std::vector<float> &value)
{
    values = value;
    initialized = true;
}
