#ifndef PowietrzeConnection_H
#define PowietrzeConnection_H

#include "connection.h"
#include "Types/stationlist.h"
#include "Types/provincelist.h"
#include "Types/sensorlist.h"
#include "Types/countrylist.h"

class ModelsManager;

class PowietrzeConnection : public Connection
{
public:
    PowietrzeConnection(ModelsManager* modelsManager);
    virtual ~PowietrzeConnection();

    virtual void countryListRequest(std::function<void(CountryListPtr)> handler) override;
    virtual void stationListRequest(std::function<void(StationListPtr)> handler) override;
    virtual void provinceListRequest(std::function<void(ProvinceListPtr)> handler) override;
    virtual void sensorListRequest(StationPtr station, std::function<void(SensorListPtr)> handler) override;
    virtual void sensorDataRequest(SensorData sensor, std::function<void (SensorData)> handler) override;
    virtual void stationIndexRequest(StationPtr station, std::function<void(StationIndexPtr)> handler) override;
    virtual void findNearestStationRequest(QGeoCoordinate coordinate, float, std::function<void(StationListPtr)> handler) override;


private:
    inline QString countryCode() const {
        return QStringLiteral("PL");
    }

    //JSON
    ProvinceListPtr readProvincesFromJson(const QJsonDocument &jsonDocument);
    StationListPtr readStationsFromJson(const QJsonDocument &jsonDocument);
    SensorListPtr readSensorsFromJson(const QJsonDocument &jsonDocument);
    SensorData readSensorDataFromJson(const QJsonDocument &jsonDocument);
    StationIndexPtr readStationIndexFromJson(const QJsonDocument &jsonDocument);
};

#endif // PowietrzeConnection_H
