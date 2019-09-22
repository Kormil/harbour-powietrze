#include "providersmanager.h"

ProvidersManager::ProvidersManager()
{

}

ProvidersManager *ProvidersManager::instance()
{
    static ProvidersManager instance;
    return &instance;
}

ProvidersManager::~ProvidersManager()
{
    deleteProviders();
}

void ProvidersManager::createProviders()
{
    if (!m_modelsManager)
        return;

    ProviderDataPtr powietrze(new ProviderData);
    powietrze->id = m_powietrze->id();
    powietrze->enabled = true;
    powietrze->name = "Powietrze";
    powietrze->shortName = "Powietrze";
    powietrze->connection = m_powietrze;
    powietrze->airQualityIndexId = 0;   //TODO zrobić wczytywanie z opcji

    ProviderDataPtr openaq(new ProviderData);
    openaq->id = m_openAq->id();
    openaq->enabled = true;
    openaq->name = "OpenAQ";
    openaq->shortName = "OpenAQ";
    openaq->connection = m_openAq;
    openaq->airQualityIndexId = 1;

    ProviderListModel* providerListModel = m_modelsManager->providerListModel();

    if (!providerListModel)
        return;

    providerListModel->addProvider(powietrze);
    providerListModel->addProvider(openaq);
}

Connection *ProvidersManager::connection(int providerId) const
{
    return m_modelsManager->providerListModel()->provider(providerId)->connection;
}

void ProvidersManager::createConenctions(ModelsManager *modelsManager)
{
    m_modelsManager = modelsManager;

    m_powietrze = new PowietrzeConnection(modelsManager);
    m_openAq = new OpenAQConnection(modelsManager);
}

void ProvidersManager::findNearestStation(QGeoCoordinate coordinate, int limit, std::function<void (StationListPtr)> handler)
{
    auto providerListModel = m_modelsManager->providerListModel();
    for (int i = 0; i < providerListModel->size(); ++i) {
        auto provider = providerListModel->provider(i);

        if (provider->enabled) {
            provider->connection->findNearestStationRequest(coordinate, limit, handler);
        }
    }
}

void ProvidersManager::deleteProviders()
{
    delete m_powietrze;
    delete m_openAq;
}
