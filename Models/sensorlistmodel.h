#ifndef SENSORLISTMODEL_H
#define SENSORLISTMODEL_H

#include <QAbstractItemModel>
#include "Types/sensorlist.h"

class ModelsManager;
class Station;

using StationPtr = std::shared_ptr<Station>;

class SensorListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum UnitsType
    {
        MICROGRAM = 0,
        MILLIGRAM
    };

    enum SensorsListRole
    {
        NAME = Qt::UserRole + 1,
        VALUE,

        LAST_COLUMN
    };

    explicit SensorListModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void requestData();

    void setSensorList(SensorList *sensorList);
    int rowCount(const QModelIndex &parent) const;
    void setStation(StationPtr station);

    float unitsConverter(UnitsType from, UnitsType to, float value) const;
    void setModelsManager(ModelsManager *modelsManager);

    void setSensorList(SensorListPtr sensorList, StationPtr station);
private:
    void requestSensorData();
    void connectModel();

signals:
    void modelLoaded();


private:
    ModelsManager *m_modelsManager;
    StationPtr m_station;
};

#endif // SENSORLISTMODEL_H
