#ifndef STATIONINDEX_H
#define STATIONINDEX_H

#include <QObject>
#include <QDateTime>
#include <QFont>
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
    QString m_calculationModeName;
};

struct StationIndex : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id NOTIFY dateChanged)
    Q_PROPERTY(QString name READ name NOTIFY dateChanged)
    Q_PROPERTY(QString calculationModeName READ calculationModeName NOTIFY dateChanged)
    Q_PROPERTY(QString date READ indexCalculateDate NOTIFY dateChanged)
public:

    int id() const;
    QString name() const;
    QString indexCalculateDate() const;
    QString calculationModeName() const;
    Q_INVOKABLE int nameWidth(QFont font) const;

    static void bindToQml(QQuickView * view);

    bool shouldGetNewData(int frequency);
    void setDateToCurent();
    void setStation(Station *station);

    void setData(const StationIndexData &data);

signals:
    void dateChanged();

private:
    StationIndexData m_data;
    QDateTime m_date;
};
#endif // STATIONINDEX_H
