#ifndef STATIONLISTMODEL_H
#define STATIONLISTMODEL_H

#include <atomic>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include "Types/stationlist.h"

class Connection;
class ProvinceItem;
class SensorListModel;
class StationListModel;
class ModelsManager;

class StationListProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString provinceNameFilter READ provinceNameFilter WRITE setProvinceNameFilter)
    Q_PROPERTY(bool favourites READ favourites WRITE setFavourites)
    Q_PROPERTY(StationListModel* stationModel READ stationListModel WRITE setStationListModel)

public:
    StationListProxyModel(QObject *parent = nullptr);
    QString provinceNameFilter() const;
    void setProvinceNameFilter(const QString &provinceNameFilter);
    bool favourites() const;
    void setFavourites(bool value);
    StationListModel *stationListModel() const;
    void setStationListModel(StationListModel *stationListModel);

    static void registerToQml();

    Q_INVOKABLE void onItemClicked(int index);


protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

private:
    QString m_provinceNameFilter = "";
    bool m_favourites = false;
    StationListModel * m_stationListModel;
};

class StationListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Station* selectedStation READ selectedStation WRITE setSelectedStation NOTIFY selectedStationChanged)

public:
    enum StationListRole
    {
        NameRole = Qt::UserRole + 1,
        ProvinceNameRole,
        FavouriteRole,
        IndexRole
    };

    explicit StationListModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<int, QByteArray> roleNames() const override;
    std::vector<QString> provinceNames() const;

    void setModels(ModelsManager *modelsManager);
    void setStationList(StationListPtr stationList);
    void appendStationList(StationListPtr &stationList);
    void setSensorListModel(SensorListModel *sensorListModel);
    Q_INVOKABLE void requestStationListData();
    void requestStationIndexData(Station* station);

    Q_INVOKABLE void onItemClicked(int index);
    static void bindToQml(QQuickView *view);

    Station *selectedStation() const;
    void setSelectedStation(int id);
    void setSelectedStation(Station *selected);

    void setConnection(Connection *connection);

public slots:
    void getIndexForFavourites();

signals:
    void stationListLoaded();
    void stationDataLoaded();
    void selectedStationChanged();
    void favourtiesUpdated();
    void favourtiesUpdatingStarted();

private:
    Connection *m_connection;
    Station* m_selectedItem;
    StationListPtr m_stationList;

    SensorListModel* m_sensorListModel;
    std::atomic_int m_indexesToDownload;
};

#endif // STATIONLISTMODEL_H
