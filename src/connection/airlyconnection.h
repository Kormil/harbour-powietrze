#ifndef AIRLYCONNECTION_H
#define AIRLYCONNECTION_H

#include <set>
#include "connection.h"
#include "powietrzeconnection.h"
#include "Types/stationlist.h"
#include "Types/provincelist.h"
#include "Types/sensorlist.h"
#include "Types/countrylist.h"

class AirlyConnection : public Connection
{
public:
    AirlyConnection(ModelsManager* modelsManager);
    virtual ~AirlyConnection();

    //Requests
    virtual void getCountryList(std::function<void(CountryListPtr)> handler) override;
    virtual void getStationList(std::function<void(StationListPtr)> handler) override;
    virtual void getProvinceList(std::function<void(ProvinceListPtr)> handler) override;
    virtual void getSensorList(StationPtr station, std::function<void(SensorListPtr)> handler) override;
    virtual void getSensorData(SensorData sensor, std::function<void (SensorData)> handler) override;
    virtual void getStationIndex(StationPtr station, std::function<void(StationIndexPtr)> handler) override;
    virtual void getNearestStations(QGeoCoordinate coordinate, float distanceLimit, std::function<void(StationListPtr)> handler) override;

private:
    //Requests
    void countryListRequest(ProvinceListPtr provinceList, std::function<void (CountryListPtr)> handler);
    void stationListRequest(std::function<void (StationListPtr)> handler);
    void provinceListRequest(StationListPtr stationList, std::function<void (ProvinceListPtr)> handler);

    //JSON
    StationListPtr readStationsFromJson(const QJsonDocument &jsonDocument);
    SensorListPtr readSensorsFromJson(const QJsonDocument &jsonDocument);
    StationIndexPtr readStationIndexFromJson(const QJsonDocument &jsonDocument);

    int m_recordLimits = -1;
};

#endif // AIRLYCONNECTION_H
