#ifndef SENSORLISTMODEL_H
#define SENSORLISTMODEL_H

#include <QAbstractItemModel>
#include "Types/sensorlist.h"

class Connection;
class Station;

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
    void requestData(Connection *connection);

    void setSensorList(SensorList *sensorList);
    int rowCount(const QModelIndex &parent) const;
    void setStation(Station *station);

    void setConnection(Connection *value);

    float unitsConverter(UnitsType from, UnitsType to, float value) const;
private:
    void requestSensorData(Connection *connection);
    void connectModel();

signals:
    void modelLoaded();


private:
    Connection *m_connection;
    Station* m_station;
};

#endif // SENSORLISTMODEL_H
