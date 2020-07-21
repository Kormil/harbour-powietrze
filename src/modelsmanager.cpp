#include "modelsmanager.h"

#include <QtCore>
#include <QObject>
#include <QQuickView>
#include <QtQml>
#include "src/settings.h"

ModelsManager::ModelsManager() :
    m_stationListModel(nullptr),
    m_sensorListModel(nullptr),
    m_provinceListModel(nullptr)
{
    QObject::connect(&m_timer, &QTimer::timeout, [this](){
        updateModels();
    });
}

ModelsManager::~ModelsManager()
{
    deleteModels();
}

void ModelsManager::createModels()
{
    m_stationListModel = std::make_unique<StationListModel>();
    m_provinceListModel = std::make_unique<ProvinceListModel>();
    m_sensorListModel = std::make_unique<SensorListModel>();
    m_countryListModel = std::make_unique<CountryListModel>();
    m_providerListModel = std::make_unique<ProviderListModel>();
    m_airQualityIndexModel = std::make_unique<AirQualityIndexModel>();

    m_airQualityIndexModel->setModelsManager(this);
    m_stationListModel->setModelsManager(this);
    m_provinceListModel->setModelsManager(this);
    m_sensorListModel->setModelsManager(this);
    m_countryListModel->setModelsManager(this);

    std::cout << "Models created" << std::endl;
}

void ModelsManager::deleteModels()
{
    std::cout << "Models deleted" << std::endl;
}

void ModelsManager::bindToQml(QQuickView * view)
{
    qmlRegisterType<ProvinceListModel>("ProvinceListModel", 1, 0, "ProvinceListModel");
    qmlRegisterType<CountryListModel>("CountryListModel", 1, 0, "CountryListModel");
    qmlRegisterType<ProviderListModel>("ProviderListModel", 1, 0, "ProviderListModel");

    ProvinceListProxyModel::bindToQml();
    StationListProxyModel::bindToQml();
    ProviderListProxyModel::bindToQml();
    CountryListProxyModel::bindToQml();
    StationListModel::bindToQml(view);

    view->rootContext()->setContextProperty(QStringLiteral("stationListModel"), m_stationListModel.get());
    view->rootContext()->setContextProperty(QStringLiteral("provinceListModel"), m_provinceListModel.get());
    view->rootContext()->setContextProperty(QStringLiteral("sensorListModel"), m_sensorListModel.get());
    view->rootContext()->setContextProperty(QStringLiteral("countryListModel"), m_countryListModel.get());
    view->rootContext()->setContextProperty(QStringLiteral("providerListModel"), m_providerListModel.get());
}

void ModelsManager::loadSettings()
{
    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));

    StationListPtr stationList = settings->favouriteStations();

    m_stationListModel->setStationList(stationList);
    m_stationListModel->getIndexForFavourites();

    m_timer.start(60 * 60 * 1000);

    std::cout << "Models settings loaded" << std::endl;
}

ProvinceListModel *ModelsManager::provinceListModel() const
{
    return m_provinceListModel.get();
}

CountryListModel *ModelsManager::countryListModel() const
{
    return m_countryListModel.get();
}

ProviderListModel *ModelsManager::providerListModel() const
{
    return m_providerListModel.get();
}

AirQualityIndexModel *ModelsManager::airQualityIndexModel() const
{
    return m_airQualityIndexModel.get();
}

void ModelsManager::updateModels()
{
    if (m_stationListModel) {
        m_stationListModel->getIndexForFavourites();
    }
}

SensorListModel *ModelsManager::sensorListModel() const
{
    return m_sensorListModel.get();
}

StationListModel *ModelsManager::stationListModel() const
{
    return m_stationListModel.get();
}
