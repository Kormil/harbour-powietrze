#ifndef AIRINDEXFROMSERVER_H
#define AIRINDEXFROMSERVER_H

#include "airqualityindex.h"

class AirIndexFromServer: public AirQualityIndex
{
public:
    AirIndexFromServer();

    virtual void calculate(StationPtr station, std::function<void(StationIndexPtr)> handler) override;
    virtual QString findWorestPollution(SensorListPtr sensorList) override;
    virtual QString name() override;

};

#endif // AIRINDEXFROMSERVER_H
