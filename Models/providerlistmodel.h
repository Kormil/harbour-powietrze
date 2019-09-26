#ifndef PROVIDERLISTMODEL_H
#define PROVIDERLISTMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <memory>

class ModelsManager;
class Connection;
class ProviderListModel;

struct ProviderData {
    int id;
    Connection* connection;
    bool enabled;
    QString name;
    QString shortName;
    QString site;
    int airQualityIndexId;
};

using ProviderDataPtr = std::shared_ptr<ProviderData>;

class ProviderListProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool enabledFilter READ enabledFilter WRITE setEnabledFilter)
    Q_PROPERTY(ProviderListModel* providerModel READ providerListModel WRITE setProviderListModel)

public:
    ProviderListProxyModel(QObject *parent = nullptr);

    ProviderListModel *providerListModel() const;
    void setProviderListModel(ProviderListModel *providerListModel);

    static void bindToQml();
    Q_INVOKABLE void onItemClicked(int index);

    bool enabledFilter() const;
    void setEnabledFilter(bool enabled);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    bool m_enabledFilter;
    ProviderListModel * m_providerListModel;
};

class ProviderListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int selectedProviderId READ selectedProviderId NOTIFY selectedProviderChanged)

public:
    enum ProviderListRole
    {
        IsEnabledRole = Qt::UserRole + 1
    };

    explicit ProviderListModel(QObject *parent = nullptr);
    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addProvider(ProviderDataPtr provider);
    ProviderDataPtr provider(int providerId) const;

    Q_INVOKABLE void onItemClicked(int index);
    ProviderData *selectedProvider();
    int selectedProviderId();

    int size() const;

    Q_INVOKABLE QString site(int provider) const;
signals:
    void selectedProviderChanged();
    void siteChanged();

private:
    std::vector<ProviderDataPtr> m_providerList;
    int m_selectedItem;
};

#endif // PROVIDERLISTMODEL_H
