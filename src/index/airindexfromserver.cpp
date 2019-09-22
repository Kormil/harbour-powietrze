#include "airindexfromserver.h"
#include "src/modelsmanager.h"
#include "src/connection/connection.h"

AirIndexFromServer::AirIndexFromServer()
{
    m_id = 0;
}

void AirIndexFromServer::calculate(StationPtr station, std::function<void(StationIndexPtr)> handler)
{
    if (m_modelsManager == nullptr) {
        handler(nullptr);
    }

    Connection* connection = m_modelsManager->providerListModel()->provider(station->provider())->connection;
    connection->stationIndexRequest(station, handler);
}

QString AirIndexFromServer::findWorestPollution(SensorListPtr sensorList)
{

}

QString AirIndexFromServer::name()
{
    return "Default";
}
