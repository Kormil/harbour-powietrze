#include "providerlistmodel.h"
#include "src/modelsmanager.h"

#include <QtQml>

namespace {
    int DEFAULT_PROVIDER = 1;
}

ProviderListModel::ProviderListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ProviderListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_providerList.size();
}

QVariant ProviderListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_providerList.size() < index.row())
        return QVariant();

    const ProviderData* provider = m_providerList[index.row()].get();
    switch (role)
    {
    case Qt::DisplayRole:
    {
        QString name = provider->shortName;
        return QVariant(name);
    }
    }

    return QVariant();
}

QHash<int, QByteArray> ProviderListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::DisplayRole] = "name";
    return names;
}

void ProviderListModel::addProvider(ProviderDataPtr provider)
{
    const int index = m_providerList.size();
    beginInsertRows(QModelIndex(), index, index);

    m_providerList.push_back(provider);

    endInsertRows();
}

ProviderDataPtr ProviderListModel::provider(int providerId)
{
    for (const auto& provider: m_providerList) {
        if (provider->id == providerId)
            return provider;
    }

    //Default provider with id = 1
    for (const auto& provider: m_providerList) {
        if (provider->id == DEFAULT_PROVIDER)
            return provider;
    }

    return nullptr;
}

void ProviderListModel::onItemClicked(int index)
{
    m_selectedItem = index;

    emit selectedProviderChanged();
}

ProviderData *ProviderListModel::selectedProvider()
{
    return m_providerList[m_selectedItem].get();
}

int ProviderListModel::selectedProviderId()
{
    return selectedProvider()->id;
}

int ProviderListModel::size() const
{
    return m_providerList.size();
}

ProviderListProxyModel::ProviderListProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{

}

void ProviderListProxyModel::bindToQml()
{
    qmlRegisterType<ProviderListProxyModel>("ProviderListModel", 1, 0, "ProviderListProxyModel");
}

bool ProviderListProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);

    bool isEnabled = sourceModel()->index(source_row, 0).data(ProviderListModel::IsEnabledRole).toBool();
    if (!m_enabledFilter && !isEnabled)
    {
        return false;
    }

    return true;
}

bool ProviderListProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QVariant leftData = sourceModel()->data(source_left);
    QVariant rightData = sourceModel()->data(source_right);

    return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
}

int ProviderListProxyModel::rowCount(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::rowCount(parent);
}

bool ProviderListProxyModel::enabledFilter() const
{
    return m_enabledFilter;
}

void ProviderListProxyModel::setEnabledFilter(bool value)
{
    m_enabledFilter = value;
    invalidateFilter();
}

ProviderListModel *ProviderListProxyModel::providerListModel() const
{
    return m_providerListModel;
}

void ProviderListProxyModel::setProviderListModel(ProviderListModel *providerListModel)
{
    m_providerListModel = providerListModel;

    setSourceModel(m_providerListModel);
    invalidateFilter();
    sort(0);
}

void ProviderListProxyModel::onItemClicked(int index)
{
    if (!m_providerListModel)
        return;

    QModelIndex modelIndex = mapToSource(this->index(index, 0));
    m_providerListModel->onItemClicked(modelIndex.row());
}
