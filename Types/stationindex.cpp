#include "stationindex.h"
#include <QQuickView>
#include "station.h"

int StationIndex::id() const
{
    return m_data.m_id;
}

QString StationIndex::name() const
{
    return m_data.m_name;
}

void StationIndex::bindToQml(QQuickView * view)
{
    Q_UNUSED(view);
    qmlRegisterType<Station>("StationListModel", 1, 0, "StationIndex");
}

void StationIndex::setId(int id)
{
    m_data.m_id = id;
    emit idChanged();
}

void StationIndex::setName(const QString &name)
{
    m_data.m_name = name;
    emit nameChanged();
}

bool StationIndex::shouldGetNewData(int frequency)
{
    if (m_data.m_station == nullptr)
        return false;

    QDateTime currentTime = QDateTime::currentDateTime();

    if (currentTime.time().hour() < m_data.m_date.time().hour())
        return true;

    QDateTime nextDataTime = m_data.m_date.addSecs( frequency * 60 );
    return currentTime > nextDataTime;
}

void StationIndex::setStation(Station *station)
{
    m_data.m_station = station;
}

void StationIndex::setData(const StationIndexData &data)
{
    m_data = data;
}

void StationIndex::setDateToCurent()
{
    m_data.m_date = QDateTime::currentDateTime();
}
