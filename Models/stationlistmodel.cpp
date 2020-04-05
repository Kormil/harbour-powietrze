#include "stationlistmodel.h"
#include <set>
#include <QtQml>
#include <QQuickView>
#include "src/connection/connection.h"
#include "sensorlistmodel.h"
#include "src/modelsmanager.h"
#include "src/providersmanager.h"
#include "src/settings.h"
#include "src/gpsmodule.h"
#include "src/utils.h"

StationListModel::StationListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_nearestStation(nullptr),
      m_beforeNearestStation(nullptr),
      m_stationList(nullptr)
{
    setStationDistanceLimit(8000);

    QObject::connect(GPSModule::instance(), &GPSModule::shouldRequest, this, &StationListModel::findNearestStation);
    QObject::connect(GPSModule::instance(), &GPSModule::positionUpdated, this, &StationListModel::onGPSPositionUpdate);
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

    const StationPtr station = m_stationList->station(index.row());
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
    case ProviderRole:
        return QVariant(station->provider());
    case IndexRole:
    {
        if (station->stationIndex())
            return QVariant(station->stationIndex()->name());
        else
            return QVariant("");
    }
    case ProviderNameRole:
        return station->providerName();
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
    std::lock_guard<std::mutex> guard(m_setStationListMutex);
    beginResetModel();

    if (m_stationList) {
        m_stationList->disconnect(this);
    }

    m_stationList = stationList;

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

void StationListModel::requestStationListData()
{
    Connection* connection = m_modelsManager->providerListModel()->selectedProvider()->connection();
    emit stationListRequested();

    connection->getStationList([this](StationListPtr stationList) {
        if (!stationList) {
            emit stationListLoaded();
            return;
        }

        if (m_stationList) {
            std::lock_guard<std::mutex> guard(m_setStationListMutex);
            m_stationList->appendList(stationList);
            emit stationListLoaded();
        }
        else {
            setStationList(stationList);
        }
    });
}

void StationListModel::requestStationIndexData(StationPtr station)
{
    if (!station)
        return;

    emit stationDataRequested();
    auto provider = m_modelsManager->providerListModel()->provider(station->provider());

    if (!provider->enabled()) {
        return;
    }

    auto airQualityIndex = m_modelsManager->airQualityIndexModel()->index(provider->airQualityIndexId());
    airQualityIndex->calculate(station, [=](StationIndexPtr stationIndex) {
        if (stationIndex != nullptr)
        {
            int row = m_stationList->row(station->hash());

            if (m_nearestStation && station->id() == m_nearestStation->id())
            {
                Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));

                if (settings->notifications() && m_beforeNearestStation && m_beforeNearestStation->stationIndex())
                {
                    if (m_beforeNearestStation->stationIndex()->id() == -1 || stationIndex->id() == -1)
                        return;

                    if (m_beforeNearestStation->stationIndex()->id() < stationIndex->id())
                    {
                        Utils::simpleNotification(tr("Nearest station"),
                                                  tr("Air pollution in your neighbour is getting worse: ").append(stationIndex->name()),
                                                  "openPage",
                                                  {QVariant(row)});
                    }
                    else if (m_beforeNearestStation->stationIndex()->id() > stationIndex->id())
                    {
                        Utils::simpleNotification(tr("Nearest station"),
                                                  tr("Air pollution in your neighbour is getting better: ").append(stationIndex->name()),
                                                  "openPage",
                                                  {QVariant(row)});
                    }
                }
            }

            station->setStationIndex(stationIndex);
            stationIndex->setDateToCurent();

            emit dataChanged(index(row), index(row), {IndexRole});

        } else {
            emit station->stationIndexChanged();
        }

        emit stationDataLoaded();
    });
}

QHash<int, QByteArray> StationListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::UserRole] = "favourite";
    names[Qt::DisplayRole] = "description";
    names[IndexRole] = "indexName";
    names[DistanceRole] = "distance";
    names[ProviderRole] = "provider";
    names[ProviderNameRole] = "providerName";
    return names;
}

void StationListModel::onStationClicked(Station* station)
{
    if (!station) {
        return;
    }
    onItemClicked(m_stationList->row(station->hash()));
}

void StationListModel::onItemClicked(int index)
{
    if (!m_modelsManager && m_modelsManager->sensorListModel())
        return;

    StationPtr station = m_stationList->station(index);
    if (station == nullptr) {
        return;
    }

    auto provider = m_modelsManager->providerListModel()->provider(station->provider());
    if (!provider->enabled()) {
        return;
    }

    auto coordinate = GPSModule::instance()->lastKnowPosition();
    double distance = station->coordinate().distanceTo(coordinate);
    station->setDistance(distance);

    m_modelsManager->sensorListModel()->setStation(station);
    setSelectedStation(station);

    if (m_modelsManager) {
        requestStationIndexData(m_selectedItem);
    }
}

void StationListModel::bindToQml(QQuickView * view)
{
    qmlRegisterType<SensorListModel>("SensorListModel", 1, 0, "sensorListModel");
    Station::bindToQml(view);
}

Station *StationListModel::nearestStation() const
{
    if (m_nearestStation) {
        return m_nearestStation.get();
    } else {
        return nullptr;
    }
}

Station *StationListModel::selectedStation() const
{
    return m_selectedItem.get();
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
    GPSModule::instance()->requestPosition();
}

void StationListModel::getIndexForFavourites()
{
    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
    if (!m_stationList) {
        return;
    }
    auto favourites = m_stationList->favourites();

    m_indexesToDownload = favourites.size();

    if (m_indexesToDownload != 0) {
        emit favourtiesUpdatingStarted();
    }

    for (const auto& hash: favourites)
    {
        StationPtr station = m_stationList->find(hash);
        if (!station) {
            return;
        }

        auto provider = m_modelsManager->providerListModel()->provider(station->provider());
        auto airQualityIndex = m_modelsManager->airQualityIndexModel()->index(provider->airQualityIndexId());

        airQualityIndex->calculate(station, [=](StationIndexPtr stationIndex) {
            m_indexesToDownload = m_indexesToDownload - 1;
            if (m_indexesToDownload == 0) {
                emit favourtiesUpdated();
            }

            if (!stationIndex) {
                return;
            }

            int row = m_stationList->row(station->hash());

            if (settings->notifications())
            {
                if (station->stationIndex()) {

                    if (station->stationIndex()->id() == -1 || stationIndex->id() == -1)
                        return;

                    if (station->stationIndex()->id() < stationIndex->id()) {
                        Utils::simpleNotification(station->name(),
                                                  tr("Air pollution is getting worse: ").append(stationIndex->name()),
                                                  "openPage",
                                                  {QVariant(row)});
                    }
                    else if (station->stationIndex()->id() > stationIndex->id()) {
                        Utils::simpleNotification(station->name(),
                                                  tr("Air pollution is getting better: ").append(stationIndex->name()),
                                                  "openPage",
                                                  {QVariant(row)});
                    }
                }
            }

            station->setStationIndex(std::move(stationIndex));
            station->stationIndex()->setDateToCurent();
            emit dataChanged(index(row), index(row), {IndexRole});
        });
    }
}

void StationListModel::onGPSPositionUpdate(QGeoCoordinate coordinate)
{
    if (!coordinate.isValid()) {
        emit nearestStationFounded();
        return ;
    }

    calculateDistances(coordinate);

    ProvidersManager::instance()->findNearestStation(coordinate, m_stationDistanceLimit, [this](StationListPtr stationList) {
        std::lock_guard<std::mutex> guard(m_findNearestStationMutex);
        if (m_stationList) {
            if (stationList) {
                m_stationList->appendList(stationList);
            }
        }
        else {
            setStationList(stationList);
        }

        if (m_stationList) {
            setNearestStation(m_stationList->findNearest());
        }
    });
}

float StationListModel::stationDistanceLimit() const
{
    return m_stationDistanceLimit;
}

void StationListModel::setStationDistanceLimit(float stationDistanceLimit)
{
    m_stationDistanceLimit = stationDistanceLimit;
    emit stationDistanceLimitChanged();
}

void StationListModel::setModelsManager(ModelsManager *modelsManager)
{
    m_modelsManager = modelsManager;
}

StationListPtr StationListModel::stationList() const
{
    return m_stationList;
}

void StationListModel::setNearestStation(StationPtr nearestStation)
{
    if (!nearestStation) {
        return;
    }

    auto coordinate = GPSModule::instance()->lastKnowPosition();

    if (m_nearestStation && m_nearestStation != nearestStation) {
        double oldDistance = m_nearestStation->coordinate().distanceTo(coordinate);
        double newDistance = nearestStation->coordinate().distanceTo(coordinate);

        if (newDistance > oldDistance)
            return;
    }

    m_beforeNearestStation = m_nearestStation;
    m_nearestStation = nearestStation;
    requestStationIndexData(m_nearestStation);
    emit nearestStationFounded();
}

void StationListModel::setSelectedStation(StationPtr selected)
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

    if (m_providerFilter)
    {
        int providerId = sourceModel()->index(source_row, 0).data(StationListModel::ProviderRole).toInt();
        if (providerId != m_providerFilter) {
            return false;
        }
    }

    if (m_sortedBy == SortStation::ByDistance && sourceModel()->index(source_row, 0).data(StationListModel::DistanceRole).toDouble() == 0) {
        return false;
    }

    if (m_distanceLimitFilter && sourceModel()->index(source_row, 0).data(StationListModel::DistanceRole).toDouble() * 1000 > m_distanceLimit) {
        return false;
    }

    if (m_favourites)
    {
        if (sourceModel()->index(source_row, 0).data(StationListModel::FavouriteRole).toBool() == false)
            return false;
    }

    QString provinceName = sourceModel()->index(source_row, 0).data(StationListModel::ProvinceNameRole).toString();
    if (!m_provinceNameFilter.isEmpty() && provinceName != m_provinceNameFilter) {
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
    if (parent.isValid() || m_stationListModel == nullptr) {
        return 0;
    }

    return QSortFilterProxyModel::rowCount(parent);
}

float StationListProxyModel::distanceLimit() const
{
    return m_distanceLimit;
}

void StationListProxyModel::setDistanceLimit(float distanceLimit)
{
    m_distanceLimit = distanceLimit;
    m_distanceLimitFilter = true;
    invalidateFilter();
}

SortStation::EnSortStation StationListProxyModel::sortedBy() const
{
    return m_sortedBy;
}

void StationListProxyModel::setSortedBy(const SortStation::EnSortStation &sortedBy)
{
    m_sortedBy = sortedBy;
    invalidate();
}

int StationListProxyModel::provider() const
{
    return m_providerFilter;
}

void StationListProxyModel::setProvider(const int &providerID)
{
    m_providerFilter = providerID;
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
