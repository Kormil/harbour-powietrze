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

//    m_host = "192.168.1.106";
//    m_port = ":12335";

    m_id = 2;
}

OpenAQConnection::~OpenAQConnection()
{

}

void OpenAQConnection::countryListRequest(std::function<void(CountryListPtr)> handler)
{
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_lastCountryListRequestDate.isValid()
            && currentTime.secsTo(m_lastCountryListRequestDate) < m_countryListRequestFrequency)
    {
        handler(CountryListPtr{});
        return ;
    }

    m_lastCountryListRequestDate = currentTime;

    QString url = "https://" + m_host + m_port + "/v1/countries" + "?limit=" + QString::number(m_recordLimits);
    QUrl countryListURL(url);

    Request* requestRaw = request(countryListURL);
    QObject::connect(requestRaw, &Request::finished, [this, requestRaw, handler](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(CountryListPtr{});
        else
        {
            CountryListPtr countryList = readCountriesFromJson(QJsonDocument::fromJson(responseArray));
            handler(std::move(countryList));
        }

        deleteRequest(requestRaw->serial());
    });
}

void OpenAQConnection::stationListRequest(std::function<void(StationListPtr)> handler)
{
    QString province = m_modelsManager->provinceListModel()->selectedProvinceName();
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_lastStationListRequestDate.isValid())
    {
        if (currentTime.secsTo(m_lastStationListRequestDate) < m_stationListRequestFrequency) {
            if (m_requestedStation.find(province) != m_requestedStation.end()) {
                handler(StationListPtr{});
                return ;
            }
        } else {
            m_lastStationListRequestDate = currentTime;
            m_requestedStation.clear();
        }
    } else {
        m_lastStationListRequestDate = currentTime;
    }

    m_requestedStation.insert(province);

    QString url = "https://" + m_host + m_port + "/v1/locations?city=" + province + "&limit=" + QString::number(m_recordLimits);
    QUrl stationListURL(url);

    Request* requestRaw = request(stationListURL);
    QObject::connect(requestRaw, &Request::finished, [this, requestRaw, handler](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(StationListPtr{});
        else
        {
            StationListPtr stationList = readStationsFromJson(QJsonDocument::fromJson(responseArray));
            handler(stationList);
        }

        deleteRequest(requestRaw->serial());
    });
}

void OpenAQConnection::provinceListRequest(std::function<void(ProvinceListPtr)> handler)
{
    QString countryCode = m_modelsManager->countryListModel()->selectedCountryCode();
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_lastProvinceListRequestDate.isValid())
    {
        if (currentTime.secsTo(m_lastProvinceListRequestDate) < m_provinceListRequestFrequency) {
            if (m_requestedProvince.find(countryCode) != m_requestedProvince.end()) {
                handler(ProvinceListPtr{});
                return ;
            }
        } else {
            m_lastProvinceListRequestDate = currentTime;
            m_requestedProvince.clear();
        }
    } else {
        m_lastProvinceListRequestDate = currentTime;
    }

    m_requestedProvince.insert(countryCode);

    QString url = "https://" + m_host + m_port + "/v1/cities?country=" + countryCode + "&limit=" + QString::number(m_recordLimits);
    QUrl provinceListURL(url);

    Request* requestRaw = request(provinceListURL);
    QObject::connect(requestRaw, &Request::finished, [this, requestRaw, handler](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(ProvinceListPtr{});
        else
        {
            ProvinceListPtr provinceList = readProvincesFromJson(QJsonDocument::fromJson(responseArray));
            handler(provinceList);
        }

        deleteRequest(requestRaw->serial());
    });
}

void OpenAQConnection::sensorListRequest(StationPtr station, std::function<void (SensorListPtr)> handler)
{
    if (station == nullptr)
    {
        handler(SensorListPtr{});
        return;
    }

    if (station->sensorList() && !station->sensorList()->shouldGetNewData(m_sensorListRequestFrequency))
    {
        handler(SensorListPtr{});
        return;
    }

    QString location = station->cityName();
    QString url = "https://" + m_host + m_port + "/v1/locations?location=" + location + "&limit=" + QString::number(m_recordLimits);
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
}

void OpenAQConnection::sensorDataRequest(SensorData sensor, std::function<void (SensorData)> handler)
{
    QString url = "https://" + m_host + m_port + "/v1/measurements?location=" + sensor.id.toString() + "&parameter=" + sensor.pollutionCode
            + "&limit=" + QString::number(m_recordLimits);
    QUrl sensorDataURL(url);

    Request* requestRaw = request(sensorDataURL);
    QObject::connect(requestRaw, &Request::finished, [this, requestRaw, handler, sensor](Request::Status status, const QByteArray& responseArray) {
        SensorData data;
        if (status != Request::ERROR) {
            data = readSensorDataFromJson(QJsonDocument::fromJson(responseArray));
        }

        handler(data);
        deleteRequest(requestRaw->serial());
    });
}

void OpenAQConnection::stationIndexRequest(StationPtr, std::function<void (StationIndexPtr)> handler)
{
    StationIndexPtr stationIndex(new StationIndex);

    stationIndex->setId(-1);
    stationIndex->setName("No index");

    handler(stationIndex);
}

void OpenAQConnection::findNearestStationRequest(QGeoCoordinate coordinate, float distanceLimit, std::function<void (StationListPtr)> handler)
{
    QString url = "https://" + m_host + m_port + "/v1/locations?coordinates="
            + QString::number(coordinate.latitude()) + "," + QString::number(coordinate.longitude())
            + "&order_by=distance&limit=" + QString::number(m_recordLimits) + "&radius=" + QString::number(distanceLimit);
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
                findNearestStationRequest(coordinate, distanceLimit * 2, handler);
            }
        }

        deleteRequest(requestRaw->serial());
    });
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
        StationPtr item = StationPtr(new Station());
        StationData stationData;
        stationData.cityName = station.toObject()["location"].toString();
        stationData.provider = id();

        double lat = station.toObject()["coordinates"].toObject()["latitude"].toDouble();
        double lon = station.toObject()["coordinates"].toObject()["longitude"].toDouble();
        stationData.coordinate = QGeoCoordinate(lat, lon);

        stationData.province = station.toObject()["city"].toString();

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
            SensorData sensorData;
            sensorData.id = location;
            sensorData.name = sensor.toString();
            sensorData.pollutionCode = sensorData.name;

            if (sensorData.name == QStringLiteral("pm25"))
                sensorData.name = QStringLiteral("pm2.5");


            sensorList->setData(sensorData);
        }

        break;
    }

    return std::move(sensorList);
}

SensorData OpenAQConnection::readSensorDataFromJson(const QJsonDocument &jsonDocument)
{
    SensorData sensorData;

    auto response = jsonDocument.object()["results"];
    if (response.isUndefined()) {
        return sensorData;
    }

    QJsonArray results = response.toArray();


    sensorData.id = results[0].toObject()["location"].toString();
    sensorData.name = results[0].toObject()["parameter"].toString();
    sensorData.pollutionCode = sensorData.name;

    if (sensorData.name == QStringLiteral("pm25"))
        sensorData.name = QStringLiteral("pm2.5");


    std::vector<float> values;
    for (const auto& result: results)
    {
        double value = result.toObject()["value"].toDouble();
        values.push_back(value);
    }
    sensorData.setValues(values);

    if (results.empty())
        sensorData.setValues(static_cast<int>(Errors::NoData));

    return sensorData;
}

