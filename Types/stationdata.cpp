#include "stationdata.h"

QDataStream& operator<<(QDataStream& out, const StationData& v)
{
    out << v.id << v.province << v.cityName << v.street;
    return out;
}

QDataStream& operator>>(QDataStream& in, StationData& v)
{
    in >> v.id;
    in >> v.province;
    in >> v.cityName;
    in >> v.street;
    return in;
}
