#ifndef AIRQUALITYINDEXMODEL_H
#define AIRQUALITYINDEXMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "src/index/airqualityindex.h"
#include "src/index/europeanaq.h"
#include "src/index/airindexfromserver.h"

class ModelsManager;
class Connection;

class AirQualityIndexModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AirQualityIndexModel(QObject *parent = nullptr);
    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    AirQualityIndexPtr index(int id);
    void setModelsManager(ModelsManager *modelsManager);

private:
    std::vector<AirQualityIndexPtr> m_airQualityIndexList;
    ModelsManager* m_modelsManager;
};

#endif // AIRQUALITYINDEXMODEL_H
