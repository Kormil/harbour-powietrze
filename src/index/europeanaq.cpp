#include "europeanaq.h"
#include <QtConcurrent>
#include <algorithm>
#include <future>
#include "src/modelsmanager.h"
#include "src/connection/connection.h"

EuropeanAQ::EuropeanAQ()
{
    m_id = 1;

    auto PM25 = std::make_pair<QString, Pollution>("pm2.5", {24, {10, 20, 25, 50, 800}});
    auto PM10 = std::make_pair<QString, Pollution>("pm10", {24, {20, 35, 50, 100, 1200}});
    auto NO2  = std::make_pair<QString, Pollution>("no2",  {1, {40, 100, 200, 400, 1000}});
    auto O3   = std::make_pair<QString, Pollution>("o3",   {1, {80, 120, 180, 240, 600}});
    auto SO2  = std::make_pair<QString, Pollution>("so2",  {1, {100, 200, 350, 500, 1250}});

    m_parametersThreshold.insert(PM25);
    m_parametersThreshold.insert(PM10);
    m_parametersThreshold.insert(NO2);
    m_parametersThreshold.insert(O3);
    m_parametersThreshold.insert(SO2);

    m_names = {QObject::tr("Good"),
               QObject::tr("Fair"),
               QObject::tr("Moderate"),
               QObject::tr("Poor"),
               QObject::tr("Very poor")};
}

void EuropeanAQ::calculate(StationPtr station, std::function<void (StationIndexPtr)> handler)
{
    Connection* connection = m_modelsManager->providerListModel()->provider(station->provider())->connection;
    connection->sensorListRequest(station, [=](SensorListPtr sensorList) {
        if (!sensorList) {
            if (station->sensorList()->isAll()) {
                handler(recalculate(station->sensorList()));
            }
        }

        m_modelsManager->sensorListModel()->setSensorList(sensorList, station);
        for (const auto& sensor: station->sensorList()->sensors())
        {
            connection->sensorDataRequest(sensor, [handler, station, this](SensorData sensorData) {
                station->sensorList()->setData(sensorData);

                if (station->sensorList()->isAll()) {
                    handler(recalculate(station->sensorList()));
                }
            });
        }
    });
}

StationIndexPtr EuropeanAQ::recalculate(SensorListPtr sensorList)
{
    int worestIndexId = -1;
    QString worestPollutionCode;

    for (const auto& sensor: sensorList->sensors()) {
        auto parameter = m_parametersThreshold.find(sensor.pollutionCode);

        if (parameter != m_parametersThreshold.end()) {
            auto thresholds = parameter->second.thresholds;
            auto hours = parameter->second.hours;
            int indexId = std::distance(thresholds.begin(), std::upper_bound(thresholds.begin(), thresholds.end(), sensor.avg(hours)));

            if (indexId > worestIndexId) {
                worestIndexId = indexId;
                worestPollutionCode = sensor.pollutionCode;
            }
        }
    }

    StationIndexPtr stationIndex(new StationIndex);
    if (worestIndexId > 0)
        stationIndex->setName(m_names[worestIndexId]);
    stationIndex->setId(worestIndexId);
    return stationIndex;
}

QString EuropeanAQ::findWorestPollution(SensorListPtr sensorList)
{
    int worestId = 0;
    auto sensors = sensorList->sensors();

    for (int i = 0; i < sensors.size(); ++i) {
        if (sensors[i].value() > sensors[worestId].value()) {
            worestId = i;
        }
    }

    return sensors[worestId].name;
}

QString EuropeanAQ::name()
{
    return "EuropeanAQ";
}
