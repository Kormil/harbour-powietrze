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

SensorListPtr SensorList::getSensorsFromJson(const QJsonDocument &jsonDocument)
{
    if (jsonDocument.isNull())
        return SensorListPtr(nullptr);

    SensorListPtr sensorList(new SensorList());

    QJsonArray array = jsonDocument.array();

    for (const auto& sensor: array)
    {
        SensorData sensorData;

        sensorData.id = sensor.toObject()["id"].toInt();
        sensorData.name = sensor.toObject()["param"].toObject()["paramName"].toString();

        sensorList->m_sensors.push_back(sensorData);
    }

    return std::move(sensorList);
}

float SensorList::getSensorDataFromJson(const QJsonDocument &jsonDocument)
{
    QJsonArray array = jsonDocument.object()["values"].toArray();

    for (const auto& sensor: array)
    {
        if (sensor.toObject()["value"].isNull())
            continue;

        float value = sensor.toObject()["value"].toDouble();
        return value;
    }

    return 0.0f;
}

size_t SensorList::size() const
{
    return m_sensors.size();
}

void SensorList::setValue(int id, float value)
{
    auto it = std::find_if(m_sensors.begin(), m_sensors.end(), [id](const SensorData& sensorData) {
        return sensorData.id == id;
    });

    int index = std::distance(m_sensors.begin(), it);

    if (it != m_sensors.end())
    {
        it->value = value;
        emit valueChanged(index);
    }
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

void SensorList::setDateToCurent()
{
    m_date = QDateTime::currentDateTime();
}
