#ifndef PROVIDERSMANAGER_H
#define PROVIDERSMANAGER_H

#include <QTimer>
#include "connection/connection.h"
#include "connection/powietrzeconnection.h"
#include "connection/openaqconnection.h"
#include "connection/airlyconnection.h"
#include "modelsmanager.h"

class ProvidersManager //Singleton
{
public:
    static ProvidersManager *instance();
    virtual ~ProvidersManager();

    void createProviders();
    void deleteProviders();

    Connection* connection(int providerId) const;
    ProviderDataPtr provider(int providerId) const;

    void createConenctions(ModelsManager *modelsManager);
    void findNearestStation(QGeoCoordinate coordinate, int distanceLimit, std::function<void(StationListPtr)> handler);

private:
    ProvidersManager();

    OpenAQConnection* m_openAq;
    PowietrzeConnection* m_powietrze;
    AirlyConnection* m_airly;

    ModelsManager* m_modelsManager;
};

#endif // PROVIDERSMANAGER_H
