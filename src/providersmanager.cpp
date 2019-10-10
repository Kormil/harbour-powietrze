#include "providersmanager.h"
#include "src/settings.h"

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

    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));

    ProviderDataPtr powietrze(new ProviderData);
    powietrze->setId(m_powietrze->id());
    powietrze->setName("Powietrze");
    powietrze->setShortName("Powietrze");
    powietrze->setSite("powietrze.gios.gov.pl");
    powietrze->setIcon("powietrze.png");
    powietrze->setConnection(m_powietrze);
    powietrze->setAirQualityIndexId(settings->providerSettings(powietrze->name(), "aqi").toInt());
    QVariant enabled = settings->providerSettings(powietrze->name(), "enabled");
    powietrze->setEnabled(enabled.isValid() ? enabled.toBool() : true);
    powietrze->setNameVariant(settings->providerSettings(powietrze->name(), "nameVariant").toInt());

    ProviderDataPtr openaq(new ProviderData);
    openaq->setId(m_openAq->id());
    openaq->setName("OpenAQ");
    openaq->setShortName("OpenAQ");
    openaq->setSite("openaq.org");
    openaq->setIcon("openaq.jpeg");
    openaq->setConnection(m_openAq);
    openaq->setAirQualityIndexId(1);
    enabled = settings->providerSettings(openaq->name(), "enabled");
    openaq->setEnabled(enabled.isValid() ? enabled.toBool() : true);

    ProviderListModel* providerListModel = m_modelsManager->providerListModel();

    if (!providerListModel)
        return;

    providerListModel->addProvider(powietrze);
    providerListModel->addProvider(openaq);
}

Connection *ProvidersManager::connection(int providerId) const
{
    return m_modelsManager->providerListModel()->provider(providerId)->connection();
}

void ProvidersManager::createConenctions(ModelsManager *modelsManager)
{
    m_modelsManager = modelsManager;

    m_powietrze = new PowietrzeConnection(modelsManager);
    m_openAq = new OpenAQConnection(modelsManager);
}

void ProvidersManager::findNearestStation(QGeoCoordinate coordinate, int distanceLimit, std::function<void (StationListPtr)> handler)
{
    if (!coordinate.isValid()) {
        handler(nullptr);
    }

    auto providerListModel = m_modelsManager->providerListModel();
    for (int i = 1; i < providerListModel->size() + 1; ++i) {
        auto provider = providerListModel->provider(i);

        if (provider->enabled()) {
            provider->connection()->findNearestStationRequest(coordinate, distanceLimit, handler);
        }
    }
}

void ProvidersManager::deleteProviders()
{
    if (m_powietrze) {
        m_powietrze->clearRequests();
        delete m_powietrze;
        m_powietrze = nullptr;
    }

    if (m_openAq) {
        m_openAq->clearRequests();
        delete m_openAq;
        m_openAq = nullptr;
    }
}
