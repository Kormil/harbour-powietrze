#ifndef AIRQUALITYINDEX_H
#define AIRQUALITYINDEX_H

#include <map>
#include <QString>
#include <memory>
#include "Types/station.h"
#include "Types/sensorlist.h"

class AirQualityIndex;
class ModelsManager;
using AirQualityIndexPtr = std::shared_ptr<AirQualityIndex>;

struct Pollution
{
    int hours;
    std::vector<float> thresholds;
};

class AirQualityIndex
{
public:
    virtual void calculate(StationPtr station, std::function<void(StationIndexPtr)> handler) = 0;
    virtual QString name() = 0;

    int id() const;

    static void setModelsManager(ModelsManager *modelsManager);

protected:
    int m_id;
    static ModelsManager* m_modelsManager;
};

#endif // AIRQUALITYINDEX_H
