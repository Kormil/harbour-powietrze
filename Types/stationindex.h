#ifndef STATIONINDEX_H
#define STATIONINDEX_H

#include <QObject>
#include <QDateTime>
#include <memory>

class Station;
class StationIndex;
class QQuickView;

using StationIndexPtr = std::shared_ptr<StationIndex>;

struct StationIndexData
{
    int m_id = -1;
    QString m_name;
    Station* m_station = nullptr;
    QDateTime m_date;
};

struct StationIndex : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id NOTIFY idChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
public:

    int id() const;
    QString name() const;

    static void bindToQml(QQuickView * view);

    void setId(int id);
    void setName(const QString &name);

    bool shouldGetNewData(int frequency);
    void setDateToCurent();
    void setStation(Station *station);

    void setData(const StationIndexData &data);

signals:
    void idChanged();
    void nameChanged();

private:
    StationIndexData m_data;
};
#endif // STATIONINDEX_H
