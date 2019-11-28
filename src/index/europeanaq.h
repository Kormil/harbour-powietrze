#ifndef EAUROPEANAQ_H
#define EAUROPEANAQ_H

#include <map>
#include <vector>
#include <mutex>
#include "airqualityindex.h"

class EuropeanAQ : public AirQualityIndex
{
public:
    EuropeanAQ();

    virtual void calculate(StationPtr station, std::function<void(StationIndexPtr)> handler) override;
    virtual QString name() override;

private:
    QString shortName() const;
    StationIndexPtr recalculate(SensorListPtr sensorList);

    std::map<QString, PollutionThresholds> m_parametersThreshold;
    std::vector<QString> m_names;

    std::mutex m_sensorDataMutex;
};

#endif // EAUROPEANAQ_H
