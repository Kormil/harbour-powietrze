#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include "Types/sensorlist.h"
#include "Types/station.h"

struct WHONorm {
    QString name;
    float value;
    int hours;
};

namespace Utils
{
    void simpleNotification(QString header, QString body, QString function, QVariantList parameters);
    float calculateWHONorms(const Pollution& sensorData);
}

#endif // UTILS_H
