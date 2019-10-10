#ifndef OpenAQConnection_H
#define OpenAQConnection_H

#include <set>
#include "connection.h"
#include "powietrzeconnection.h"
#include "Types/stationlist.h"
#include "Types/provincelist.h"
#include "Types/sensorlist.h"
#include "Types/countrylist.h"

class ModelsManager;

class OpenAQConnection : public Connection
{
public:
    OpenAQConnection(ModelsManager* modelsManager);
    virtual ~OpenAQConnection();

    //Requests
    virtual void countryListRequest(std::function<void(CountryListPtr)> handler) override;
    virtual void stationListRequest(std::function<void(StationListPtr)> handler) override;
    virtual void provinceListRequest(std::function<void(ProvinceListPtr)> handler) override;
    virtual void sensorListRequest(StationPtr station, std::function<void(SensorListPtr)> handler) override;
    virtual void sensorDataRequest(SensorData sensor, std::function<void (SensorData)> handler) override;
    virtual void stationIndexRequest(StationPtr, std::function<void(StationIndexPtr)> handler) override;
    virtual void findNearestStationRequest(QGeoCoordinate coordinate, float distanceLimit, std::function<void(StationListPtr)> handler) override;

private:
    //JSON
    CountryListPtr readCountriesFromJson(const QJsonDocument &jsonDocument);
    ProvinceListPtr readProvincesFromJson(const QJsonDocument &jsonDocument);
    StationListPtr readStationsFromJson(const QJsonDocument &jsonDocument);
    SensorListPtr readSensorsFromJson(const QJsonDocument &jsonDocument);
    SensorData readSensorDataFromJson(const QJsonDocument &jsonDocument);

    std::set<QString> m_requestedProvince;
    std::set<QString> m_requestedStation;

    int m_recordLimits = 1000;
};

#endif // OpenAQConnection_H
