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
    m_stationListModel = new StationListModel();
    m_provinceListModel = new ProvinceListModel();
    m_sensorListModel = new SensorListModel();
    m_countryListModel = new CountryListModel();
    m_providerListModel = new ProviderListModel();
    m_airQualityIndexModel = new AirQualityIndexModel();

    m_airQualityIndexModel->setModelsManager(this);
    m_stationListModel->setModelsManager(this);
    m_provinceListModel->setModelsManager(this);
    m_sensorListModel->setModelsManager(this);
    m_countryListModel->setModelsManager(this);

    std::cout << "Models created" << std::endl;
}

void ModelsManager::deleteModels()
{
    delete m_stationListModel;
    delete m_provinceListModel;
    delete m_sensorListModel;
    delete m_countryListModel;
    delete m_providerListModel;
    delete m_airQualityIndexModel;
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

    view->rootContext()->setContextProperty(QStringLiteral("stationListModel"), m_stationListModel);
    view->rootContext()->setContextProperty(QStringLiteral("provinceListModel"), m_provinceListModel);
    view->rootContext()->setContextProperty(QStringLiteral("sensorListModel"), m_sensorListModel);
    view->rootContext()->setContextProperty(QStringLiteral("countryListModel"), m_countryListModel);
    view->rootContext()->setContextProperty(QStringLiteral("providerListModel"), m_providerListModel);
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
    return m_provinceListModel;
}

CountryListModel *ModelsManager::countryListModel() const
{
    return m_countryListModel;
}

ProviderListModel *ModelsManager::providerListModel() const
{
    return m_providerListModel;
}

AirQualityIndexModel *ModelsManager::airQualityIndexModel() const
{
    return m_airQualityIndexModel;
}

void ModelsManager::updateModels()
{
    if (m_stationListModel) {
        m_stationListModel->getIndexForFavourites();
    }
}

SensorListModel *ModelsManager::sensorListModel() const
{
    return m_sensorListModel;
}

StationListModel *ModelsManager::stationListModel() const
{
    return m_stationListModel;
}
