#include "openaqconnection.h"
#include <QStringBuilder>
#include <iostream>
#include <notification.h>
#include "../modelsmanager.h"

OpenAQConnection::OpenAQConnection(ModelsManager* modelsManager) :
    Connection(modelsManager)
{
    m_host = "api.openaq.org";
    m_port = "";

    m_id = 2;
    m_indexName = "";
}

OpenAQConnection::~OpenAQConnection()
{

}

void OpenAQConnection::getCountryList(std::function<void(CountryListPtr)> handler)
{
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_cashedCountries && m_cashedCountries->size()
            && m_lastCountryListRequestDate.secsTo(currentTime) < m_getCountryListFrequency)
    {
        handler(m_cashedCountries);
        return ;
    }

    QString url = "https://" + m_host + m_port + "/v1/countries"
            + "?limit=" + QString::number(m_recordLimits);
    QUrl countryListURL(url);

    Request* requestRaw = request(countryListURL);

    QObject::connect(requestRaw, &Request::finished, [this, requestRaw, handler](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(CountryListPtr{});
        else
        {
            m_lastCountryListRequestDate = QDateTime::currentDateTime();
            CountryListPtr countryList = readCountriesFromJson(QJsonDocument::fromJson(responseArray));
            m_cashedCountries = countryList;
            handler(std::move(countryList));
        }

        deleteRequest(requestRaw->serial());
    });
    requestRaw->run();
}

void OpenAQConnection::getStationList(std::function<void(StationListPtr)> handler)
{
    QString countryCode = m_modelsManager->countryListModel()->selectedCountryCode();
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_requestMapStationDatetime[countryCode].isValid())
    {
        if (m_requestMapStationDatetime[countryCode].secsTo(currentTime) < m_getStationListFrequency) {
            handler(m_requestedStation[countryCode]);
            return ;
        }
    }

    QString url = "https://" + m_host + m_port
            + "/v1/locations?"
            + "&country=" + countryCode
            + "&limit=" + QString::number(m_recordLimits);
    QUrl stationListURL(url);

    Request* requestRaw = request(stationListURL);
    QObject::connect(requestRaw, &Request::finished, [this, countryCode, requestRaw, handler](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(m_requestedStation[countryCode]);
        else
        {
            m_requestMapStationDatetime[countryCode] = QDateTime::currentDateTime();
            StationListPtr stationList = readStationsFromJson(QJsonDocument::fromJson(responseArray));
            m_requestedStation[countryCode] = stationList;
            handler(stationList);
        }

        deleteRequest(requestRaw->serial());
    });

    requestRaw->run();
}

void OpenAQConnection::getProvinceList(std::function<void (ProvinceListPtr)> handler)
{
    getStationList([this, handler](StationListPtr stationList) {
        provinceListRequest(stationList, handler);
    });
}

void OpenAQConnection::provinceListRequest(StationListPtr stationList, std::function<void (ProvinceListPtr)> handler)
{
    if (!stationList) {
        ProvinceListPtr provinceList(new ProvinceList());
        handler(provinceList);
        return;
    }

    QString countryCode = m_modelsManager->countryListModel()->selectedCountryCode();

    auto cmp = [](const QString& a, const QString& b) {
        return QString::localeAwareCompare(a, b) < 0;
    };

    std::set<QString, decltype(cmp)> names(cmp);
    for (unsigned int i = 0; i < stationList->size(); ++i)
    {
        StationPtr station = stationList->station(i);
        QString name = station->province();
        names.insert(name);
    }

    ProvinceListPtr provinceList(new ProvinceList());
    for (const auto value: names)
    {
        ProvinceItemPtr province(new ProvinceItem());
        province->name = value;
        province->countryCode = countryCode;
        province->provider = id();
        provinceList->append(province);
    }

    handler(provinceList);
}

void OpenAQConnection::getSensorList(StationPtr station, std::function<void (SensorListPtr)> handler)
{
    if (station == nullptr)
    {
        handler(SensorListPtr{});
        return;
    }

    if (station->sensorList() && !station->sensorList()->shouldGetNewData(m_getSensorListFrequency))
    {
        handler(SensorListPtr{});
        return;
    }

    QString location = station->streetName();
    if (location.isEmpty()) {   //for backward compatybility (04.2020)
        location = station->cityName();
    }

    QString url = "https://" + m_host + m_port + "/v1/locations?location=" + location
            + "&limit=" + QString::number(m_recordLimits);
    QUrl provinceListURL(url);

    Request* requestRaw = request(provinceListURL);

    QObject::connect(requestRaw, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler( SensorListPtr() );
        else
        {
            SensorListPtr sensorList = readSensorsFromJson(QJsonDocument::fromJson(responseArray));
            sensorList->setDateToCurrent();
            handler(std::move(sensorList));
        }

        deleteRequest(requestRaw->serial());
    });
    requestRaw->run();
}

void OpenAQConnection::getSensorData(Pollution sensor, std::function<void (Pollution)> handler)
{
    QString url = "https://" + m_host + m_port + "/v1/measurements?location=" + sensor.id.toString()
            + "&parameter=" + sensor.code
            + "&limit=" + QString::number(m_recordLimits)
            + "&value_from=" + QString::number(m_minimumValue);
    QUrl sensorDataURL(url);

    Request* requestRaw = request(sensorDataURL);

    QObject::connect(requestRaw, &Request::finished, [this, requestRaw, handler, sensor](Request::Status status, const QByteArray& responseArray) {
        Pollution data = sensor;
        if (status != Request::ERROR) {
            data = readSensorDataFromJson(QJsonDocument::fromJson(responseArray));

            if (data.name.isEmpty()) {
                data.name = sensor.name;
                data.id = sensor.id;
            }
        }

        handler(data);
        deleteRequest(requestRaw->serial());
    });
    requestRaw->run();
}

void OpenAQConnection::getStationIndex(StationPtr, std::function<void (StationIndexPtr)> handler)
{
    StationIndexData stationIndexData;

    stationIndexData.m_id = -1;
    stationIndexData.m_name = "No index";

    StationIndexPtr stationIndex(new StationIndex);
    stationIndex->setData(stationIndexData);
    handler(stationIndex);
}

void OpenAQConnection::getNearestStations(QGeoCoordinate coordinate, float distanceLimit, std::function<void (StationListPtr)> handler)
{
    QString url = "https://" + m_host + m_port + "/v1/locations?coordinates="
            + QString::number(coordinate.latitude()) + "," + QString::number(coordinate.longitude())
            + "&order_by=distance&limit=" + QString::number(m_recordLimits)
            + "&radius=" + QString::number(distanceLimit);
    QUrl stationListURL(url);

    Request* requestRaw = request(stationListURL);

    QObject::connect(requestRaw, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(StationListPtr{});
        else
        {
            StationListPtr stationList = readStationsFromJson(QJsonDocument::fromJson(responseArray));

            if (stationList->size() >= 1) {
                stationList->calculateDistances(coordinate);
                handler(stationList);
            } else {
                getNearestStations(coordinate, distanceLimit * 2, handler);
            }
        }

        deleteRequest(requestRaw->serial());
    });
    requestRaw->run();
}

CountryListPtr OpenAQConnection::readCountriesFromJson(const QJsonDocument &jsonDocument)
{
    CountryListPtr countryList(new CountryList());


    auto response = jsonDocument.object()["results"];
    if (response.isUndefined()) {
        std::cout << "OpenAQ: no results in JSON" << std::endl;
        return countryList;
    }

    QJsonArray results = response.toArray();

    for (const auto& country: results)
    {
        CountryItemPtr item(new CountryItem);
        item->name = country.toObject()["name"].toString();
        item->code = country.toObject()["code"].toString();
        item->provider = id();
        countryList->append(item);
    }

    return std::move(countryList);
}

ProvinceListPtr OpenAQConnection::readProvincesFromJson(const QJsonDocument &jsonDocument)
{
    ProvinceListPtr provinceList(new ProvinceList());

    auto response = jsonDocument.object()["results"];
    if (response.isUndefined()) {
        std::cout << "OpenAQ: no results in JSON" << std::endl;
        return provinceList;
    }

    QJsonArray results = response.toArray();

    for (const auto& province: results)
    {
        ProvinceItemPtr item(new ProvinceItem);
        item->name = province.toObject()["city"].toString();
        item->countryCode = province.toObject()["country"].toString();
        item->provider = id();
        provinceList->append(item);
    }

    return std::move(provinceList);
}

StationListPtr OpenAQConnection::readStationsFromJson(const QJsonDocument &jsonDocument)
{
    StationListPtr stationList(new StationList());

    auto response = jsonDocument.object()["results"];
    if (response.isUndefined()) {
        std::cout << "OpenAQ: no results in JSON" << std::endl;
        return stationList;
    }

    QJsonArray results = response.toArray();

    for (const auto& station: results)
    {
        QDateTime lastUpdate = QDateTime::fromString(station.toObject()["lastUpdated"].toString(), Qt::ISODate);

        if (lastUpdate < QDateTime::currentDateTime().addYears(-1)) {
            continue;
        }

        StationPtr item = StationPtr(new Station());
        StationData stationData;
        stationData.cityName = station.toObject()["city"].toString();
        stationData.street = station.toObject()["location"].toString();
        stationData.provider = id();

        double lat = station.toObject()["coordinates"].toObject()["latitude"].toDouble();
        double lon = station.toObject()["coordinates"].toObject()["longitude"].toDouble();
        stationData.coordinate = QGeoCoordinate(lat, lon);

        stationData.province = station.toObject()["city"].toString();
        stationData.country = station.toObject()["country"].toString();

        item->setStationData(stationData);
        stationList->append(item);
    }

    return stationList;
}

SensorListPtr OpenAQConnection::readSensorsFromJson(const QJsonDocument &jsonDocument)
{
    if (jsonDocument.isNull())
        return SensorListPtr(nullptr);

    SensorListPtr sensorList(new SensorList());

    auto response = jsonDocument.object()["results"];
    if (response.isUndefined()) {
        return sensorList;
    }

    QJsonArray results = response.toArray();

    for (const auto& result: results)
    {
        QString location = result.toObject()["location"].toString();
        QJsonArray sensors = result.toObject()["parameters"].toArray();
        for (const auto& sensor: sensors)
        {
            Pollution sensorData;
            sensorData.id = location;
            sensorData.name = sensor.toString();
            sensorData.code = sensorData.name;

            if (sensorData.name == QStringLiteral("pm25"))
                sensorData.name = QStringLiteral("pm2.5");


            sensorList->setData(sensorData);
        }

        break;
    }

    return std::move(sensorList);
}

Pollution OpenAQConnection::readSensorDataFromJson(const QJsonDocument &jsonDocument)
{
    Pollution sensorData;

    auto response = jsonDocument.object()["results"];
    if (response.isUndefined()) {
        sensorData.setValues(PollutionValue{static_cast<int>(Errors::NoData), QDateTime::currentDateTime()});
        return sensorData;
    }

    QJsonArray results = response.toArray();

    QString dateString = results[0].toObject()["date"].toObject()["utc"].toString();

    sensorData.id = results[0].toObject()["location"].toString();
    sensorData.name = results[0].toObject()["parameter"].toString();
    sensorData.unit = results[0].toObject()["unit"].toString();
    sensorData.code = sensorData.name;
    sensorData.date = QDateTime::fromString(dateString, Qt::ISODate).toLocalTime();

    if (sensorData.name == QStringLiteral("pm25"))
        sensorData.name = QStringLiteral("pm2.5");


    std::vector<PollutionValue> values;
    for (const auto& result: results)
    {
        float value = result.toObject()["value"].toDouble();

        QString dateString = result.toObject()["date"].toObject()["utc"].toString();
        QDateTime sensorDate = QDateTime::fromString(dateString, Qt::ISODate).toLocalTime();

        values.push_back(PollutionValue{value, sensorDate});
    }
    sensorData.setValues(values);


    if (results.empty())
        sensorData.setValues(PollutionValue{static_cast<int>(Errors::NoData), QDateTime::currentDateTime()});

    return sensorData;
}
