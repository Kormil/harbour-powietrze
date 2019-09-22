#include "utils.h"
#include <notification.h>

void Utils::simpleNotification(QString header, QString body)
{
    Notification notification;
    notification.setBody(body);
    notification.setPreviewSummary(header);
    notification.setPreviewBody(body);
    notification.publish();
}

StationIndexPtr Utils::airQualityIndex(SensorListPtr sensorList)
{
    StationIndexPtr stationIndex(new StationIndex);


}
