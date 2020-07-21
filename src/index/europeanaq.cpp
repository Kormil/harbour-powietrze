#include "europeanaq.h"
#include <QtConcurrent>
#include <algorithm>
#include <future>
#include "src/modelsmanager.h"
#include "src/connection/connection.h"

EuropeanAQ::EuropeanAQ()
{
    m_id = 1;

    auto PM25 = std::make_pair<QString, PollutionThresholds>("pm25", {24, {10, 20, 25, 50, 800}});
    auto PM10 = std::make_pair<QString, PollutionThresholds>("pm10", {24, {20, 35, 50, 100, 1200}});
    auto NO2  = std::make_pair<QString, PollutionThresholds>("no2",  {1, {40, 100, 200, 400, 1000}});
    auto O3   = std::make_pair<QString, PollutionThresholds>("o3",   {1, {80, 120, 180, 240, 600}});
    auto SO2  = std::make_pair<QString, PollutionThresholds>("so2",  {1, {100, 200, 350, 500, 1250}});

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
    Connection* connection = m_modelsManager->providerListModel()->provider(station->provider())->connection();
    connection->getSensorList(station, [=](SensorListPtr sensorList) {
        if(!sensorList && !station->sensorList()) {
            handler(std::make_shared<StationIndex>());
        }

        if (station->stationIndex() && !station->stationIndex()->shouldGetNewData(connection->getStationIndexFrequency()))
        {
            handler(nullptr);
            return;
        }

        if (!sensorList) {
            if (station->sensorList()->isAll()) {
                handler(recalculate(station->sensorList()));
                return ;
            }
        }

        m_modelsManager->sensorListModel()->setSensorList(sensorList, station);
        for (const auto& sensor: station->sensorList()->sensors())
        {
            connection->getSensorData(sensor, [handler, station, this](Pollution sensorData) {
                station->sensorList()->setData(sensorData);

                std::lock_guard<std::mutex> guard(m_sensorDataMutex);
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
        auto parameter = m_parametersThreshold.find(sensor.code);

        if (parameter != m_parametersThreshold.end()) {
            auto thresholds = parameter->second.thresholds;
            auto hours = parameter->second.hours;
            int indexId = std::distance(thresholds.begin(), std::upper_bound(thresholds.begin(), thresholds.end(), sensor.avg(hours)));

            if (indexId > worestIndexId) {
                worestIndexId = indexId;
                worestPollutionCode = sensor.code;
            }
        }
    }

    StationIndexData stationIndexData;
    if (worestIndexId >= 0)
        stationIndexData.m_name = m_names[worestIndexId];
    stationIndexData.m_id = worestIndexId;
    stationIndexData.m_date = QDateTime::currentDateTime();
    stationIndexData.m_calculationModeName = shortName();

    StationIndexPtr stationIndex = std::make_shared<StationIndex>();
    stationIndex->setData(stationIndexData);
    return stationIndex;
}

QString EuropeanAQ::name()
{
    return "EuropeanAQ";
}

QString EuropeanAQ::shortName() const
{
    return "EAQI";
}
