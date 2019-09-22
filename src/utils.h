#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include "Types/sensorlist.h"
#include "Types/station.h"

namespace Utils
{
    void simpleNotification(QString header, QString body);
    StationIndexPtr airQualityIndex(SensorListPtr sensorList);
}

#endif // UTILS_H
