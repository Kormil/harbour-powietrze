#include "stationlistmodel.h"
#include <set>
#include <QtQml>
#include <QQuickView>
#include "src/connection.h"
#include "sensorlistmodel.h"
#include "src/modelsmanager.h"
#include "src/settings.h"
#include "src/gpsmodule.h"
#include "src/utils.h"

StationListModel::StationListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_nearestStation(nullptr),
      m_beforeNearestStation(nullptr),
      m_stationList(nullptr)
{
    QObject::connect(GPSModule::instance(), &GPSModule::shouldRequest, this, &StationListModel::findNearestStation);
    QObject::connect(GPSModule::instance(), &GPSModule::positionUpdated, this, [this](QGeoCoordinate coordinate) {

        //TODO sprawdzić czy jest potrzeba wysyłania requestu
        m_connection->findNearestStationRequest(coordinate, [this](StationListPtr stationList) {
            if (!stationList) {
                setNearestStation(m_stationList->findNearest());
            }
        });
    });
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
    case DistanceRole:
        return QVariant(station->distanceString());
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
    if (!stationList)
    {
        emit stationListLoaded();
        return;
    }

    beginResetModel();

    if (m_stationList)
    {
        m_stationList->disconnect(this);
        stationList->appendList(m_stationList);
    }

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
    emit stationListRequested();

    m_connection->stationListRequest([this](StationListPtr stationList) {
        setStationList(std::move(stationList));
    });
}

void StationListModel::requestStationIndexData(Station *station)
{
    if (!station)
        return;

    m_connection->stationIndexRequest(station->id(), [=](StationIndexPtr stationIndex) {
        if (stationIndex != nullptr)
        {
            if (m_nearestStation && station->id() == m_nearestStation->id())
            {
                Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));

                if (settings->notifications() && m_beforeNearestStation && m_beforeNearestStation->stationIndex())
                {
                    if (m_beforeNearestStation->stationIndex()->id() == -1 || stationIndex->id() == -1)
                        return;

                    if (m_beforeNearestStation->stationIndex()->id() < stationIndex->id())
                    {
                        Utils::SimpleNotification(tr("Nearest station"),
                                                  tr("Air pollution in your neighbour is getting worse: ").append(stationIndex->name()));
                    }
                    else if (m_beforeNearestStation->stationIndex()->id() > stationIndex->id())
                    {
                        Utils::SimpleNotification(tr("Nearest station"),
                                                  tr("Air pollution in your neighbour is getting better: ").append(stationIndex->name()));
                    }
                }
            }

            if (stationIndex != station->stationIndexPtr())
            {
                int row = m_stationList->row(station->id());
                station->setStationIndex(std::move(stationIndex));
                station->stationIndex()->setDateToCurent();

                emit dataChanged(index(row), index(row), {IndexRole});
            }

        } else {
            emit station->stationIndexChanged();
        }

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
    names[DistanceRole] = "distance";
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

void StationListModel::onStationClicked(Station* station)
{
    onItemClicked(m_stationList->row(station->id()));
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
    qmlRegisterType<SensorListModel>("SensorListModel", 1, 0, "sensorListModel");
    Station::bindToQml(view);
}

Station *StationListModel::station(int id) const
{
    return m_stationList->find(id);
}

Station *StationListModel::nearestStation() const
{
    return m_nearestStation;
}

Station *StationListModel::selectedStation() const
{
    return m_selectedItem;
}

void StationListModel::setConnection(Connection *connection)
{
    m_connection = connection;
}

bool StationListModel::shouldGetNewData()
{
    if (m_stationList == nullptr)
        return true;

    QDateTime currentTime = QDateTime::currentDateTime();

    if (!m_lastRequestDate.isValid() || currentTime.daysTo(m_lastRequestDate))
        return true;

    return false;
}

void StationListModel::setDateToCurrent()
{
    m_lastRequestDate = QDateTime::currentDateTime();
}

void StationListModel::calculateDistances(QGeoCoordinate coordinate)
{
    if (!m_stationList) {
        return;
    }

    m_stationList->calculateDistances(coordinate);
}

void StationListModel::findNearestStation()
{
    emit nearestStationRequested();

    if (shouldGetNewData())
    {
        emit stationListRequested();

        m_connection->stationListRequest([this](StationListPtr stationList) {
            setStationList(std::move(stationList));
            GPSModule::instance()->requestPosition();
        });
    }
    else
    {
        GPSModule::instance()->requestPosition();
    }
}

void StationListModel::getIndexForFavourites()
{
    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
    std::vector<int> favourites = m_stationList->favouriteIds();

    m_indexesToDownload = favourites.size();

    if (m_indexesToDownload != 0) {
        emit favourtiesUpdatingStarted();
    }

    for (const auto& id: favourites)
    {
        Station* station = m_stationList->find(id);

        m_connection->stationIndexRequest(station->id(), [=](StationIndexPtr stationIndex) {
            --m_indexesToDownload;
            if (m_indexesToDownload == 0) {
                emit favourtiesUpdated();
            }

            if (stationIndex == nullptr) {
                return;
            }

            if (settings->notifications())
            {
                if (station->stationIndex()) {

                    if (m_beforeNearestStation->stationIndex()->id() == -1 || stationIndex->id() == -1)
                        return;

                    if (station->stationIndex()->id() < stationIndex->id()) {
                        Utils::SimpleNotification(station->name(),
                                                  tr("Air pollution is getting worse: ").append(stationIndex->name()));
                    }
                    else if (station->stationIndex()->id() > stationIndex->id()) {
                        Utils::SimpleNotification(station->name(),
                                                  tr("Air pollution is getting better: ").append(stationIndex->name()));
                    }
                }
            }

            int row = m_stationList->row(station->id());
            station->setStationIndex(std::move(stationIndex));
            station->stationIndex()->setDateToCurent();

            emit dataChanged(index(row), index(row), {IndexRole});
        });
    }
}

void StationListModel::setNearestStation(Station *nearestStation)
{
    if (!nearestStation) {
        return;
    }

    m_beforeNearestStation = m_nearestStation;
    m_nearestStation = nearestStation;
    requestStationIndexData(m_nearestStation);
    emit nearestStationFounded();
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

void StationListProxyModel::bindToQml()
{
    qmlRegisterType<StationListProxyModel>("StationListModel", 1, 0, "StationListProxyModel");
    qmlRegisterUncreatableType<SortStation>("StationListModel", 1, 0, "SortStation", "Cannot create SortStation in QML");
}

bool StationListProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);

    if (m_sortedBy == SortStation::ByDistance && sourceModel()->index(source_row, 0).data(StationListModel::DistanceRole).toDouble() == 0)
        return false;

    QString provinceName = sourceModel()->index(source_row, 0).data(StationListModel::ProvinceNameRole).toString();

    if (m_favourites)
    {
        if (sourceModel()->index(source_row, 0).data(StationListModel::FavouriteRole).toBool() == false)
            return false;
    }

    if (!m_provinceNameFilter.isEmpty() && provinceName != m_provinceNameFilter)
    {
        return false;
    }

    if (!m_stationNameFilter.isEmpty())
    {
        QString stationName = sourceModel()->index(source_row, 0).data(StationListModel::NameRole).toString();

        if (!m_stationNameFilter.contains(stationName))
            return false;
    }

    return true;
}

bool StationListProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_sortedBy == SortStation::ByName && source_right.isValid() && source_left.isValid())
    {
        QVariant leftData = sourceModel()->data(source_left);
        QVariant rightData = sourceModel()->data(source_right);

        return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
    }

    if (m_sortedBy == SortStation::ByDistance && source_right.isValid() && source_left.isValid())
    {
        QVariant leftData = sourceModel()->data(source_left, StationListModel::DistanceRole);
        QVariant rightData = sourceModel()->data(source_right, StationListModel::DistanceRole);

        return leftData.toDouble() < rightData.toDouble();
    }

    return false;
}

int StationListProxyModel::rowCount(const QModelIndex &parent) const
{
    if (m_limit)
        return std::min(m_limit, QSortFilterProxyModel::rowCount(parent));

    return QSortFilterProxyModel::rowCount(parent);
}

SortStation::EnSortStation StationListProxyModel::sortedBy() const
{
    return m_sortedBy;
}

void StationListProxyModel::setSortedBy(const SortStation::EnSortStation &sortedBy)
{
    m_sortedBy = sortedBy;
    invalidateFilter();
    sort(0);
}

int StationListProxyModel::limit() const
{
    return m_limit;
}

void StationListProxyModel::setLimit(int value)
{
    m_limit = value;
    invalidateFilter();
}

QStringList StationListProxyModel::stationNameFilter() const
{
    return m_stationNameFilter;
}

void StationListProxyModel::setStationNameFilter(const QStringList &stationNameFilter)
{
    m_stationNameFilter = stationNameFilter;
    invalidateFilter();
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
