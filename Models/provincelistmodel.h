#ifndef PROVINCELISTMODEL_H
#define PROVINCELISTMODEL_H

#include <QAbstractItemModel>
#include "Types/provincelist.h"

class StationListModel;
class ModelsManager;

class ProvinceListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString selectedProvince READ selectedProvinceName NOTIFY selectedProvinceNameChanged)

public:
    explicit ProvinceListModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setModels(ModelsManager *modelsManager);
    void setProvinceList(ProvinceListPtr provinceList);
    void setStationListModel(StationListModel *value);

    void onStationListModelLoaded();

    Q_INVOKABLE void onItemClicked(int index);
    ProvinceItem *selectedItem() const;
    QString selectedProvinceName() const;

signals:
    void selectedProvinceNameChanged();
    void provinceLoaded();

private:
    ProvinceListPtr m_provinceList;

    //3rd models
    StationListModel *m_stationListModel;
    ProvinceItem* m_selectedItem;
};

#endif // PROVINCELISTMODEL_H
