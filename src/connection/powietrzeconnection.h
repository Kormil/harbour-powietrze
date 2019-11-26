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

    virtual void getCountryList(std::function<void(CountryListPtr)> handler) override;
    virtual void getStationList(std::function<void(StationListPtr)> handler) override;
    virtual void getProvinceList(std::function<void(ProvinceListPtr)> handler) override;
    virtual void getSensorList(StationPtr station, std::function<void(SensorListPtr)> handler) override;
    virtual void getSensorData(Pollution sensor, std::function<void (Pollution)> handler) override;
    virtual void getStationIndex(StationPtr station, std::function<void(StationIndexPtr)> handler) override;
    virtual void getNearestStations(QGeoCoordinate coordinate, float, std::function<void(StationListPtr)> handler) override;

private:
    inline QString countryCode() const {
        return QStringLiteral("PL");
    }

    inline QString dateFormat() const {
        return "yyyy-MM-dd HH:mm:ss";
    }

    //Requests
    void stationListRequest(std::function<void(StationListPtr)> handler);
    void provinceListRequest(StationListPtr stationList, std::function<void(ProvinceListPtr)> handler);

    //JSON
    ProvinceListPtr readProvincesFromJson(const QJsonDocument &jsonDocument);
    StationListPtr readStationsFromJson(const QJsonDocument &jsonDocument);
    SensorListPtr readSensorsFromJson(const QJsonDocument &jsonDocument);
    Pollution readSensorDataFromJson(const QJsonDocument &jsonDocument);
    StationIndexPtr readStationIndexFromJson(const QJsonDocument &jsonDocument);
};

#endif // PowietrzeConnection_H
