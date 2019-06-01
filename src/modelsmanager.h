#ifndef MODELSMANAGER_H
#define MODELSMANAGER_H

#include <QTimer>
#include "connection.h"

class QQuickView;

class StationListModel;
class SensorListModel;
class ProvinceListModel;

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

private:
    Connection m_connection;
    QTimer m_timer;
    StationListModel * m_stationListModel;
    SensorListModel * m_sensorListModel;
    ProvinceListModel * m_provinceListModel;

};

#endif // MODELSMANAGER_H
