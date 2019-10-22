#include "stationdata.h"

namespace {
    int DEFAULT_PROVIDER = 1;
}

QDataStream& operator<<(QDataStream& out, const StationData& v)
{
    out << v.id << v.province << v.cityName << v.street << v.coordinate.latitude() << v.coordinate.longitude() << v.country << v.provider;
    return out;
}

QDataStream& operator>>(QDataStream& in, StationData& v)
{
    in >> v.id;
    in >> v.province;
    in >> v.cityName;
    in >> v.street;

    double latitude;
    double longitude;
    in >> latitude;
    in >> longitude;

    v.coordinate.setLatitude(latitude);
    v.coordinate.setLongitude(longitude);

    in >> v.country;

    if (!in.atEnd()) {
        in >> v.provider;

    if (!v.provider)
        v.provider = DEFAULT_PROVIDER;
    }

    return in;
}
