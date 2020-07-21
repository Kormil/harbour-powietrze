#ifndef MODELSMANAGER_H
#define MODELSMANAGER_H

#include <QTimer>
#include "connection/connection.h"
#include "connection/powietrzeconnection.h"
#include "connection/openaqconnection.h"
#include "Models/stationlistmodel.h"
#include "Models/provincelistmodel.h"
#include "Models/sensorlistmodel.h"
#include "Models/countrylistmodel.h"
#include "Models/providerlistmodel.h"
#include "Models/airqualityindexmodel.h"

class QQuickView;

class ModelsManager
{
public:
    ModelsManager();
    ~ModelsManager();

    void createModels();
    void updateModels();
    void deleteModels();
    void bindToQml(QQuickView *view);
    void loadSettings();

    StationListModel *stationListModel() const;
    SensorListModel *sensorListModel() const;
    ProvinceListModel *provinceListModel() const;
    CountryListModel *countryListModel() const;
    ProviderListModel *providerListModel() const;
    AirQualityIndexModel *airQualityIndexModel() const;

private:
    QTimer m_timer;
    std::unique_ptr<StationListModel> m_stationListModel;
    std::unique_ptr<SensorListModel> m_sensorListModel;
    std::unique_ptr<ProvinceListModel> m_provinceListModel;
    std::unique_ptr<CountryListModel> m_countryListModel;
    std::unique_ptr<ProviderListModel> m_providerListModel;
    std::unique_ptr<AirQualityIndexModel> m_airQualityIndexModel;

};

#endif // MODELSMANAGER_H
