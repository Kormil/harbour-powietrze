#include "utils.h"
#include <notification.h>
#include <iostream>

void Utils::simpleNotification(QString header, QString body)
{
    Notification notification;
    notification.setIcon("qrc:///icons/172x172/harbour-powietrze.png");
    notification.setCategory("Powietrze.update");
    notification.setSummary(header);
    notification.setBody(body);
    notification.setPreviewSummary(header);
    notification.setPreviewBody(body);
    notification.publish();
}

float Utils::calculateWHONorms(const Pollution& sensorData) {
    if (!sensorData.isInitialized()) {
        return -1;
    }

    std::list<WHONorm> pollutions = {
        {"pm25", 25.f, 24},
        {"pm10", 50.f, 24},
        {"no2",  200.f, 1},
        {"o3",   100.f, 8},
        {"so2",  350.f, 1},
        {"co",  30000.f, 1}
    };

    for (auto pollution: pollutions) {
        if (pollution.name == sensorData.code) {
            float avg = sensorData.avg(pollution.hours);

            return avg / pollution.value * 100;
        }
    }


    return -1;
}
