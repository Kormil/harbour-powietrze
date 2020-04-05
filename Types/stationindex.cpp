#include "stationindex.h"
#include <QQuickView>
#include <QFontMetrics>
#include "station.h"

int StationIndex::id() const
{
    return m_data.m_id;
}

QString StationIndex::name() const
{
    return m_data.m_name;
}

QString StationIndex::indexCalculateDate() const
{
    return m_data.m_date.toString("HH:mm");
}

QString StationIndex::calculationModeName() const
{
    return m_data.m_calculationModeName;
}

int StationIndex::nameWidth(QFont font) const
{
    QFontMetrics metric(font);
    return metric.width(name());
}

void StationIndex::bindToQml(QQuickView * view)
{
    Q_UNUSED(view);
    qmlRegisterType<Station>("StationListModel", 1, 0, "StationIndex");
}

bool StationIndex::shouldGetNewData(int frequency)
{
    if (m_data.m_station == nullptr)
        return false;

    QDateTime currentTime = QDateTime::currentDateTime();

    if (currentTime.time().hour() < m_date.time().hour())
        return true;

    QDateTime nextDataTime = m_date.addSecs(frequency);
    return currentTime > nextDataTime;
}

void StationIndex::setStation(Station *station)
{
    m_data.m_station = station;
}

void StationIndex::setData(const StationIndexData &data)
{
    m_data = data;
    emit dateChanged();
}

void StationIndex::setDateToCurent()
{
    m_date = QDateTime::currentDateTime();
}
