#include "provincelistmodel.h"
#include "stationlistmodel.h"
#include "src/modelsmanager.h"

#include <QtQml>

ProvinceListModel::ProvinceListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_provinceList(nullptr)
{
}

int ProvinceListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || m_provinceList == nullptr)
        return 0;

    return m_provinceList->size();
}

QVariant ProvinceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_provinceList == nullptr)
        return QVariant();

    const ProvinceItemPtr province = m_provinceList->get(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
    {
        QString name = province->name.toLower();
        name[0] = name[0].toUpper();
        return QVariant(name);
    }
    case CountryCodeRole:
    {
        return QVariant(province->countryCode);
    }
    case ProviderRole:
        return QVariant(province->provider);
    }

    return QVariant();
}

QHash<int, QByteArray> ProvinceListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::DisplayRole] = "name";
    names[CountryCodeRole] = "countryCode";
    names[ProviderRole] = "provider";
    return names;
}

void ProvinceListModel::setProvinceList(ProvinceListPtr provinceList)
{
    beginResetModel();

    if (m_provinceList) {
        m_provinceList->disconnect(this);
    }

    m_provinceList = provinceList;

    if (m_provinceList)
    {
        connect(m_provinceList.get(), &ProvinceList::preItemAppended, this, [this]() {
            const int index = m_provinceList->size();
            beginInsertRows(QModelIndex(), index, index);
        });

        connect(m_provinceList.get(), &ProvinceList::postItemAppended, this, [this]() {
            endInsertRows();
        });
    }

    endResetModel();
    emit provinceLoaded();
}

void ProvinceListModel::onStationListModelLoaded()
{
    emit provinceLoaded();
}

void ProvinceListModel::onItemClicked(int index)
{
    if (index != -1)
    {
        m_selectedItem = m_provinceList->get(index).get();
    }
    else
    {
        m_selectedItem = nullptr;
    }

    emit selectedProvinceNameChanged();
}

ProvinceItem *ProvinceListModel::selectedItem() const
{
    return m_selectedItem;
}

QString ProvinceListModel::selectedProvinceName() const
{
    if (m_selectedItem)
        return m_selectedItem->name;

    return QStringLiteral("");
}

void ProvinceListModel::requestProvinceList()
{
    emit provinceListRequested();

    Connection* connection = m_modelsManager->providerListModel()->selectedProvider()->connection;
    connection->provinceListRequest([this](ProvinceListPtr provinceList) {
        if (!provinceList) {
            emit provinceLoaded();
            return ;
        }

        if (m_provinceList) {
            m_provinceList->appendList(provinceList);
        }
        else {
            setProvinceList(provinceList);
        }
    });
}

void ProvinceListModel::setModelsManager(ModelsManager *modelsManager)
{
    m_modelsManager = modelsManager;

    connect(m_modelsManager->stationListModel(), &StationListModel::stationListLoaded,
            this, &ProvinceListModel::onStationListModelLoaded);
}


ProvinceListProxyModel::ProvinceListProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{

}

void ProvinceListProxyModel::setCountryFilter(const QString &countryFilter)
{
    m_countryFilter = countryFilter;
    invalidateFilter();
}

void ProvinceListProxyModel::bindToQml()
{
    qmlRegisterType<ProvinceListProxyModel>("ProvinceListModel", 1, 0, "ProvinceListProxyModel");
}

bool ProvinceListProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);

    int providerId = sourceModel()->index(source_row, 0).data(ProvinceListModel::ProviderRole).toInt();
    if (providerId != m_providerFilter) {
        return false;
    }

    QString countryCode = sourceModel()->index(source_row, 0).data(ProvinceListModel::CountryCodeRole).toString();
    if (!m_countryFilter.isEmpty() && countryCode != m_countryFilter)
    {
        return false;
    }

    return true;
}

bool ProvinceListProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QVariant leftData = sourceModel()->data(source_left);
    QVariant rightData = sourceModel()->data(source_right);

    return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
}

int ProvinceListProxyModel::rowCount(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::rowCount(parent);
}

int ProvinceListProxyModel::provider() const
{
    return m_providerFilter;
}

void ProvinceListProxyModel::setProvider(int providerFilter)
{
    m_providerFilter = providerFilter;
    invalidateFilter();
}

QString ProvinceListProxyModel::countryFilter() const
{
    return m_countryFilter;
}

ProvinceListModel *ProvinceListProxyModel::provinceListModel() const
{
    return m_provinceListModel;
}

void ProvinceListProxyModel::setProvinceListModel(ProvinceListModel *provinceListModel)
{
    m_provinceListModel = provinceListModel;

    setSourceModel(m_provinceListModel);
    invalidateFilter();
    sort(0);
}

void ProvinceListProxyModel::onItemClicked(int index)
{
    if (!m_provinceListModel)
        return;

    QModelIndex modelIndex = mapToSource(this->index(index, 0));
    m_provinceListModel->onItemClicked(modelIndex.row());
}
