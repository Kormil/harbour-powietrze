#ifndef PROVINCELISTMODEL_H
#define PROVINCELISTMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "Types/provincelist.h"

class StationListModel;
class ProvinceListModel;
class ModelsManager;
class Connection;

class ProvinceListProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int provider READ provider WRITE setProvider)
    Q_PROPERTY(QString countryFilter READ countryFilter WRITE setCountryFilter)
    Q_PROPERTY(ProvinceListModel* provinceModel READ provinceListModel WRITE setProvinceListModel)

public:
    ProvinceListProxyModel(QObject *parent = nullptr);
    QString countryFilter() const;
    void setCountryFilter(const QString &countryFilter);

    ProvinceListModel *provinceListModel() const;
    void setProvinceListModel(ProvinceListModel *provinceListModel);

    static void bindToQml();
    Q_INVOKABLE void onItemClicked(int index);

    int provider() const;
    void setProvider(int provider);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    int m_providerFilter;
    QString m_countryFilter = "";
    ProvinceListModel * m_provinceListModel;
};

class ProvinceListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString selectedProvince READ selectedProvinceName NOTIFY selectedProvinceNameChanged)

public:
    enum ProvinceListRole
    {
        CountryCodeRole = Qt::UserRole + 1,
        ProviderRole
    };

    explicit ProvinceListModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setProvinceList(ProvinceListPtr provinceList);

    void onStationListModelLoaded();

    Q_INVOKABLE void onItemClicked(int index);
    ProvinceItem *selectedItem() const;
    QString selectedProvinceName() const;
    Q_INVOKABLE void requestProvinceList();

    void setModelsManager(ModelsManager *modelsManager);

signals:
    void selectedProvinceNameChanged();
    void provinceListRequested();
    void provinceLoaded();

private:
    ProvinceListPtr m_provinceList;
    ProvinceItem* m_selectedItem;

    //3rd models
    ModelsManager* m_modelsManager;
};

#endif // PROVINCELISTMODEL_H
