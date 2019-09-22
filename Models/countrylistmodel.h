#ifndef COUNTRYLISTMODEL_H
#define COUNTRYLISTMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "Types/countrylist.h"

class StationListModel;
class CountryListModel;
class ModelsManager;
class Connection;

class CountryListProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int provider READ provider WRITE setProvider)
    Q_PROPERTY(CountryListModel* countryModel READ countryListModel WRITE setCountryListModel)

public:
    CountryListProxyModel(QObject *parent = nullptr);
    int provider() const;
    void setProvider(const int &providerID);

    CountryListModel *countryListModel() const;
    void setCountryListModel(CountryListModel *countryListModel);

    static void bindToQml();
    Q_INVOKABLE void onItemClicked(int index);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    int m_providerFilter;
    CountryListModel * m_countryListModel;
};

class CountryListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString selectedCountry READ selectedCountryCode NOTIFY selectedCountryCodeChanged)

public:
    enum CountryListRole
    {
        ProviderRole = Qt::UserRole + 1
    };

    explicit CountryListModel(QObject *parent = nullptr);
    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setCountryList(CountryListPtr countryList);

    Q_INVOKABLE void onItemClicked(int index);
    CountryItem *selectedItem() const;
    QString selectedCountryCode() const;

    Q_INVOKABLE void requestCountryList();

    void setModelsManager(ModelsManager *modelsManager);

    CountryListPtr countryList() const;

signals:
    void selectedCountryCodeChanged();
    void countryListLoaded();
    void countryListRequested();

private:
    CountryListPtr m_countryList;
    std::map<int, unsigned int> m_rowToCountryHash;
    ModelsManager* m_modelsManager;

    //3rd models
    CountryItem* m_selectedItem;
};

#endif // COUNTRYLISTMODEL_H
