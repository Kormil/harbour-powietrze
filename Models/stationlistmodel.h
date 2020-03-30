#ifndef STATIONLISTMODEL_H
#define STATIONLISTMODEL_H

#include <atomic>
#include <mutex>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QDateTime>
#include <QtPositioning/QGeoCoordinate>
#include "Types/stationlist.h"

class Connection;
class ProvinceItem;
class SensorListModel;
class StationListModel;
class ModelsManager;

class SortStation : public QObject {
    Q_OBJECT
public:
    enum EnSortStation
    {
        ByName,
        ByDistance
    };
    Q_ENUM(EnSortStation)
};

class StationListProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString provinceNameFilter READ provinceNameFilter WRITE setProvinceNameFilter)
    Q_PROPERTY(bool favourites READ favourites WRITE setFavourites)
    Q_PROPERTY(QStringList stationNameFilter READ stationNameFilter WRITE setStationNameFilter)
    Q_PROPERTY(StationListModel* stationModel READ stationListModel WRITE setStationListModel)
    Q_PROPERTY(SortStation::EnSortStation sort READ sortedBy WRITE setSortedBy)
    Q_PROPERTY(int distanceLimit READ distanceLimit WRITE setDistanceLimit)
    Q_PROPERTY(int provider READ provider WRITE setProvider)

public:
    StationListProxyModel(QObject *parent = nullptr);
    QString provinceNameFilter() const;
    void setProvinceNameFilter(const QString &provinceNameFilter);
    bool favourites() const;
    void setFavourites(bool value);
    StationListModel *stationListModel() const;
    void setStationListModel(StationListModel *stationListModel);
    QStringList stationNameFilter() const;
    void setStationNameFilter(const QStringList &stationNameFilter);
    SortStation::EnSortStation sortedBy() const;
    void setSortedBy(const SortStation::EnSortStation &sortedBy);
    int provider() const;
    void setProvider(const int &providerID);
    float distanceLimit() const;
    void setDistanceLimit(float distanceLimit);

    static void bindToQml();

    Q_INVOKABLE void onItemClicked(int index);


protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    int m_providerFilter = 0;
    QString m_provinceNameFilter = "";
    QStringList m_stationNameFilter;
    bool m_favourites = false;
    StationListModel * m_stationListModel;
    float m_distanceLimit = 8000.0f;
    bool m_distanceLimitFilter = false;
    SortStation::EnSortStation m_sortedBy = SortStation::EnSortStation::ByName;
};

class StationListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Station* nearestStation READ nearestStation NOTIFY nearestStationFounded)
    Q_PROPERTY(Station* selectedStation READ selectedStation NOTIFY selectedStationChanged)
    Q_PROPERTY(float stationDistanceLimit READ stationDistanceLimit WRITE setStationDistanceLimit NOTIFY stationDistanceLimitChanged)

public:
    enum StationListRole
    {
        NameRole = Qt::UserRole + 1,
        ProvinceNameRole,
        FavouriteRole,
        IndexRole,
        DistanceRole,
        ProviderRole,
        ProviderNameRole
    };

    explicit StationListModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<int, QByteArray> roleNames() const override;

    void setStationList(StationListPtr stationList);
    Q_INVOKABLE void requestStationListData();
    void requestStationIndexData(StationPtr station);

    Q_INVOKABLE void onStationClicked(Station* station);
    Q_INVOKABLE void onItemClicked(int index);
    static void bindToQml(QQuickView *view);

    Station *nearestStation() const;
    Station *selectedStation() const;
    void setSelectedStation(StationPtr selected);
    void setNearestStation(StationPtr nearestStation);

    void calculateDistances(QGeoCoordinate coordinate);

    StationListPtr stationList() const;

    void setModelsManager(ModelsManager *modelsManager);

    float stationDistanceLimit() const;
    void setStationDistanceLimit(float stationDistanceLimit);

public slots:
    void findNearestStation();
    void getIndexForFavourites();
    void onGPSPositionUpdate(QGeoCoordinate coordinate);

signals:
    void stationListLoaded();
    void stationDataRequested();
    void stationDataLoaded();
    void selectedStationChanged();
    void favourtiesUpdated();
    void favourtiesUpdatingStarted();
    void nearestStationFounded();
    void stationDistanceLimitChanged();

private:
    StationPtr m_selectedItem;
    StationPtr m_nearestStation;
    StationPtr m_beforeNearestStation;
    StationListPtr m_stationList;

    int m_stationDistanceLimit;

    ModelsManager* m_modelsManager;
    std::atomic_int m_indexesToDownload;
    std::mutex m_setStationListMutex;
    std::mutex m_findNearestStationMutex;
};

#endif // STATIONLISTMODEL_H
