#include "stationlistmodel.h"
#include <set>
#include <QtQml>
#include <QQuickView>
#include "src/connection.h"
#include "sensorlistmodel.h"
#include "src/modelsmanager.h"
#include "src/settings.h"

StationListModel::StationListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_stationList(nullptr)
{
    //m_provider = std::move(Provider::getFromJson("local.json"));
}

int StationListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid() || m_stationList == nullptr)
        return 0;

    return m_stationList->size();
}

QVariant StationListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_stationList == nullptr)
        return QVariant();

    const Station * station = m_stationList->station(index.row());
    switch (role)
    {
    case Qt::UserRole:
        return QVariant(station->id());
    case Qt::DisplayRole:
        return QVariant(station->name());
    case NameRole:
        return QVariant(station->name());
    case ProvinceNameRole:
        return QVariant(station->province());
    case FavouriteRole:
        return QVariant(station->favourite());
    case IndexRole:
    {
        if (station->stationIndex())
            return QVariant(station->stationIndex()->name());
        else
            return QVariant("");
    }
    }
    return QVariant();
}

Qt::ItemFlags StationListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable;
}

void StationListModel::setStationList(StationListPtr stationList)
{
    beginResetModel();

    if (m_stationList)
        m_stationList->disconnect(this);

    m_stationList = std::move(stationList);

    if (m_stationList)
    {
        connect(m_stationList.get(), &StationList::preItemAppended, this, [this]() {
            const int index = m_stationList->size();
            beginInsertRows(QModelIndex(), index, index);
        });

        connect(m_stationList.get(), &StationList::postItemAppended, this, [this]() {
            endInsertRows();
        });

        connect(m_stationList.get(), &StationList::itemChanged, this, [this](int index) {
            QModelIndex topLeft = this->index(index, 0);
            QModelIndex bottomRight = this->index(index, 0);
            dataChanged(topLeft, bottomRight);
        });
    }

    endResetModel();
    emit stationListLoaded();
}

void StationListModel::appendStationList(StationListPtr& stationList)
{
    if (!m_stationList)
    {
        setStationList(std::move(stationList));
        return;
    }

    m_stationList->appendList(stationList);
}

void StationListModel::requestStationListData()
{
//    if (m_stationList != nullptr && m_stationList->size() != 0)
//        return;

    m_connection->stationListRequest([this](StationListPtr stationList) {
        if (stationList)
        {
            stationList->appendList(m_stationList);
            setStationList(std::move(stationList));
        }
    });
}

void StationListModel::requestStationIndexData(Station *station)
{
    if (!station)
        return;

    if (station->stationIndex() != nullptr && !station->stationIndex()->shouldGetNewData(m_connection->frequency()))
    {
        emit stationDataLoaded();
        return;
    }

    m_connection->stationIndexRequest(station->id(), [=](StationIndexPtr stationIndex) {
        if (stationIndex == nullptr)
            return;

        int row = m_stationList->row(station->id());
        station->setStationIndex(std::move(stationIndex));
        station->stationIndex()->setDateToCurent();

        emit dataChanged(index(row), index(row), {IndexRole});
        emit stationDataLoaded();
    });
}

void StationListModel::setModels(ModelsManager *modelsManager)
{
    setSensorListModel(modelsManager->sensorListModel());
}

void StationListModel::setSensorListModel(SensorListModel *sensorListModel)
{
    m_sensorListModel = sensorListModel;
}

QHash<int, QByteArray> StationListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::UserRole] = "favourite";
    names[Qt::DisplayRole] = "description";
    names[IndexRole] = "indexName";
    return names;
}

std::vector<QString> StationListModel::provinceNames() const
{
    if (m_stationList == nullptr)
        return std::vector<QString>();

    auto cmp = [](const QString& a, const QString& b) {
        return QString::localeAwareCompare(a, b) < 0;
    };

    std::set<QString, decltype(cmp)> names(cmp);
    for (unsigned int i = 0; i < m_stationList->size(); ++i)
    {
        Station* station = m_stationList->station(i);
        QString name = station->province();
        names.insert(name);
    }

    return std::vector<QString>(names.begin(), names.end());
}

void StationListModel::onItemClicked(int index)
{
    if (!m_sensorListModel)
        return;

    Station* station = m_stationList->station(index);

    if (station == nullptr)
        return;

    m_sensorListModel->setStation(station);
    setSelectedStation(station);

    if (m_connection)
        requestStationIndexData(m_selectedItem);
}

void StationListModel::bindToQml(QQuickView * view)
{
    Station::bindToQml(view);
}

Station *StationListModel::selectedStation() const
{
    return m_selectedItem;
}

void StationListModel::setConnection(Connection *connection)
{
    m_connection = connection;
}

void StationListModel::getIndexForFavourites()
{
    std::vector<int> favourites = m_stationList->favouriteIds();

    m_indexesToDownload = favourites.size();

    if (m_indexesToDownload != 0)
    {
        emit favourtiesUpdatingStarted();
    }

    for (const auto& id: favourites)
    {
        Station* station = m_stationList->find(id);

        if (station->stationIndex() && !station->stationIndex()->shouldGetNewData(m_connection->frequency()))
        {
            --m_indexesToDownload;

            if (m_indexesToDownload == 0)
                emit favourtiesUpdated();

            continue;
        }

        m_connection->stationIndexRequest(station->id(), [=](StationIndexPtr stationIndex) {
            if (stationIndex == nullptr)
                return;

            int row = m_stationList->row(station->id());
            station->setStationIndex(std::move(stationIndex));
            station->stationIndex()->setDateToCurent();

            --m_indexesToDownload;
            if (m_indexesToDownload == 0)
                emit favourtiesUpdated();

            emit dataChanged(index(row), index(row), {IndexRole});
        });
    }
}

void StationListModel::setSelectedStation(int id)
{
    if (!m_stationList)
        return;

    Station* selectedStation = m_stationList->find(id);
    setSelectedStation(selectedStation);
}

void StationListModel::setSelectedStation(Station *selected)
{
    m_selectedItem = selected;

    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
    settings->setLastViewStation(m_selectedItem->stationData());

    emit selectedStationChanged();
}

StationListProxyModel::StationListProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{

}

void StationListProxyModel::setProvinceNameFilter(const QString &provinceNameFilter)
{
    m_provinceNameFilter = provinceNameFilter;
    invalidateFilter();
}

void StationListProxyModel::registerToQml()
{
    qmlRegisterType<StationListProxyModel>("StationListModel", 1, 0, "StationListProxyModel");
}

bool StationListProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    QString provinceName = sourceModel()->index(source_row, 0).data(StationListModel::ProvinceNameRole).toString();

    if (m_favourites)
    {
        if (sourceModel()->index(source_row, 0).data(StationListModel::FavouriteRole).toBool() == false)
            return false;
    }

    if (provinceName.contains(m_provinceNameFilter))
    {
        return true;
    }

    return false;
}

bool StationListProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (source_right.isValid() && source_left.isValid())
    {
        QVariant leftData = sourceModel()->data(source_left);
        QVariant rightData = sourceModel()->data(source_right);

        return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
    }

    return false;
}

QString StationListProxyModel::provinceNameFilter() const
{
    return m_provinceNameFilter;
}

bool StationListProxyModel::favourites() const
{
    return m_favourites;
}

StationListModel *StationListProxyModel::stationListModel() const
{
    return m_stationListModel;
}

void StationListProxyModel::setStationListModel(StationListModel *stationListModel)
{
    m_stationListModel = stationListModel;

//    connect(sourceModel, &StationListModel::dataChanged, this, [this](){
//        invalidateFilter();
//    });

    setSourceModel(m_stationListModel);
    invalidateFilter();
    sort(0);
}

void StationListProxyModel::onItemClicked(int index)
{
    if (!m_stationListModel)
        return;

    QModelIndex modelIndex = mapToSource(this->index(index, 0));
    m_stationListModel->onItemClicked(modelIndex.row());
}

void StationListProxyModel::setFavourites(bool value)
{
    m_favourites = value;
    invalidateFilter();
}
