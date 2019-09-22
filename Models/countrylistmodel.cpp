#include "countrylistmodel.h"
#include "src/modelsmanager.h"

#include <QtQml>

CountryListModel::CountryListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_countryList(nullptr)
{
}

int CountryListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || m_countryList == nullptr)
        return 0;

    return m_countryList->size();
}

QVariant CountryListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_countryList == nullptr)
        return QVariant();

    const CountryItemPtr country = m_countryList->get(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
    {
        return QVariant(country->name);
    }
    case ProviderRole:
        return QVariant(country->provider);
    }

    return QVariant();
}

QHash<int, QByteArray> CountryListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::DisplayRole] = "name";
    names[ProviderRole] = "provider";
    return names;
}

void CountryListModel::setCountryList(CountryListPtr countryList)
{
    beginResetModel();

    if (m_countryList) {
        m_countryList->disconnect(this);
    }

    m_countryList = std::move(countryList);

    if (m_countryList)
    {
        connect(m_countryList.get(), &CountryList::preItemAppended, this, [this]() {
            const int index = m_countryList->size();
            beginInsertRows(QModelIndex(), index, index);
        });

        connect(m_countryList.get(), &CountryList::postItemAppended, this, [this]() {
            endInsertRows();
        });
    }

    endResetModel();
    emit countryListLoaded();
}

void CountryListModel::onItemClicked(int index)
{
    if (index != -1)
    {
        m_selectedItem = m_countryList->get(index).get();
    }
    else
    {
        m_selectedItem = nullptr;
    }

    emit selectedCountryCodeChanged();
}

CountryItem *CountryListModel::selectedItem() const
{
    return m_selectedItem;
}

QString CountryListModel::selectedCountryCode() const
{
    if (m_selectedItem)
        return m_selectedItem->code;

    return QStringLiteral("");
}

void CountryListModel::requestCountryList()
{
    emit countryListRequested();

    ProviderData * provider = m_modelsManager->providerListModel()->selectedProvider();
    Connection* connection = provider->connection;

    connection->countryListRequest([this](CountryListPtr countryList) {
        if (!countryList) {
            countryListLoaded();
            return;
        }

        if (m_countryList) {
            m_countryList->appendList(countryList);
        }
        else {
            setCountryList(countryList);
        }
    });
}

void CountryListModel::setModelsManager(ModelsManager *modelsManager)
{
    m_modelsManager = modelsManager;
}

CountryListPtr CountryListModel::countryList() const
{
    return m_countryList;
}


CountryListProxyModel::CountryListProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

int CountryListProxyModel::provider() const
{
    return m_providerFilter;
}

void CountryListProxyModel::setProvider(const int &providerID)
{
    m_providerFilter = providerID;
    invalidateFilter();
}

CountryListModel *CountryListProxyModel::countryListModel() const
{
    return m_countryListModel;
}

void CountryListProxyModel::setCountryListModel(CountryListModel *countryListModel)
{
    m_countryListModel = countryListModel;

    setSourceModel(m_countryListModel);
    invalidateFilter();
    sort(0);
}

void CountryListProxyModel::bindToQml()
{
    qmlRegisterType<CountryListProxyModel>("CountryListModel", 1, 0, "CountryListProxyModel");
}

void CountryListProxyModel::onItemClicked(int index)
{
    if (!m_countryListModel)
        return;

    QModelIndex modelIndex = mapToSource(this->index(index, 0));
    m_countryListModel->onItemClicked(modelIndex.row());
}

bool CountryListProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);

    QString name = sourceModel()->index(source_row, 0).data(Qt::DisplayRole).toString();
    if (name.isEmpty()) {
        return false;
    }

    int providerId = sourceModel()->index(source_row, 0).data(CountryListModel::ProviderRole).toInt();
    if (providerId != m_providerFilter) {
        return false;
    }

    return true;
}

int CountryListProxyModel::rowCount(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::rowCount(parent);
}
