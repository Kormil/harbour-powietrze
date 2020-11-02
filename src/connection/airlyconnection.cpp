#include "airlyconnection.h"
#include <QStringBuilder>
#include <utility>
#include <iostream>
#include "notification.h"
#include "../modelsmanager.h"

namespace {
    int DEFAULT_SENSOR_ID = 1;
}
AirlyConnection::AirlyConnection(ModelsManager *modelsManager) :
    Connection(modelsManager)
{
    m_host = "airapi.airly.eu";
    m_port = "";

    m_id = 3;
    m_indexName = "CAQI";
}

AirlyConnection::~AirlyConnection()
{

}

void AirlyConnection::getCountryList(std::function<void (CountryListPtr)> handler)
{
    getProvinceList([this, handler](ProvinceListPtr provinceList) {
        countryListRequest(provinceList, handler);
    });
}

void AirlyConnection::countryListRequest(ProvinceListPtr provinceList, std::function<void (CountryListPtr)> handler)
{
    if (provinceList == nullptr) {
        handler(m_cashedCountries);
        return;
    }

    auto cmp = [](const QString& a, const QString& b) {
        return QString::localeAwareCompare(a, b) < 0;
    };

    std::set<QString, decltype(cmp)> names(cmp);
    for (unsigned int i = 0; i < provinceList->size(); ++i)
    {
        ProvinceItemPtr province = provinceList->get(i);
        QString name = province->countryCode;
        names.insert(name);
    }

    CountryListPtr countryList = std::make_shared<CountryList>();
    for (const auto value: names)
    {
        CountryItemPtr country = std::make_shared<CountryItem>();
        country->name = value;
        country->code = value;
        country->provider = id();
        countryList->append(country);
    }

    if (countryList->size() > 0) {
        m_cashedCountries = countryList;
    }
    handler(countryList);
}

void AirlyConnection::getStationList(std::function<void (StationListPtr)> handler)
{
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_cashedStations && m_cashedStations->size()
            && m_lastStationListRequestDate.secsTo(currentTime) < m_getStationListFrequency)
    {
        handler(m_cashedStations);
        return ;
    }

    stationListRequest(handler);
}

void AirlyConnection::stationListRequest(std::function<void (StationListPtr)> handler)
{
    QString apiKey = m_modelsManager->providerListModel()->provider(id())->apiKey();
    QString url = "https://" + m_host + m_port + "/v2/installations/nearest?apikey=" + apiKey
            + "&lat=50.0&lng=20.0&maxDistanceKM=-1&maxResults=-1";
    QUrl stationListURL(url);

    Request* requestRaw = request(stationListURL);
    requestRaw->addHeader(QByteArray("Accept-Language"), QByteArray("en"));

    QObject::connect(requestRaw, &Request::finished, [this, requestRaw, handler](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(StationListPtr(nullptr));
        else
        {
            m_lastStationListRequestDate = QDateTime::currentDateTime();
            parseHeaders(requestRaw->getResponseHeaders());
            StationListPtr stationList = readStationsFromJson(QJsonDocument::fromJson(responseArray));
            m_cashedStations = stationList;
            handler(stationList);
        }

        deleteRequest(requestRaw->serial());
    });

    requestRaw->run();
}

void AirlyConnection::getProvinceList(std::function<void (ProvinceListPtr)> handler)
{
    getStationList([this, handler](StationListPtr stationList) {
        provinceListRequest(stationList, handler);
    });
}

void AirlyConnection::provinceListRequest(StationListPtr stationList, std::function<void (ProvinceListPtr)> handler)
{
    if (stationList == nullptr) {
        handler(m_cashedProvinces);
        return;
    }

    auto cmp = [](const std::pair<QString, QString>& a, const std::pair<QString, QString>& b) {
        return QString::localeAwareCompare(a.first, b.first) < 0;
    };

    std::set<std::pair<QString, QString>, decltype(cmp)> names(cmp);
    for (unsigned int i = 0; i < stationList->size(); ++i)
    {
        StationPtr station = stationList->station(i);
        QString name = station->province();
        QString countryCode = station->country();
        names.insert(std::make_pair(name, countryCode));
    }

    ProvinceListPtr provinceList = std::make_shared<ProvinceList>();
    for (const auto value: names)
    {
        ProvinceItemPtr province = std::make_shared<ProvinceItem>();
        province->name = value.first;
        province->countryCode = value.second;
        province->provider = id();
        provinceList->append(province);
    }

    m_cashedProvinces = provinceList;
    handler(provinceList);
}

void AirlyConnection::parameterUnitsRequest(std::function<void (void)> handler)
{
    if (!m_parametersUnits.empty()) {
        handler();
        return ;
    }

    QString apiKey = m_modelsManager->providerListModel()->provider(id())->apiKey();
    QString url = "https://" + m_host + m_port + "/v2/meta/measurements?apikey=" + apiKey;
    QUrl provinceListURL(url);

    Request* requestRaw = request(provinceListURL);
    requestRaw->addHeader(QByteArray("Accept-Language"), QByteArray("en"));

    QObject::connect(requestRaw, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR) {
            handler();
        } else {
            parseHeaders(requestRaw->getResponseHeaders());
            m_parametersUnits = readParametersUnitsFromJson(QJsonDocument::fromJson(responseArray));
            handler();
        }

        deleteRequest(requestRaw->serial());
    });
    requestRaw->run();
}

void AirlyConnection::getSensorList(StationPtr station, std::function<void (SensorListPtr)> handler)
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

    QString apiKey = m_modelsManager->providerListModel()->provider(id())->apiKey();
    QString url = "https://" + m_host + m_port + "/v2/measurements/installation?apikey=" + apiKey
            + "&installationId=" + QString::number(station->id());
    QUrl provinceListURL(url);

    Request* requestRaw = request(provinceListURL);
    requestRaw->addHeader(QByteArray("Accept-Language"), QByteArray("en"));

    QObject::connect(requestRaw, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler( SensorListPtr() );
        else
        {
            parseHeaders(requestRaw->getResponseHeaders());
            parameterUnitsRequest([this, responseArray, handler]() {
                SensorListPtr sensorList = readSensorsFromJson(QJsonDocument::fromJson(responseArray));
                sensorList->setDateToCurrent();
                handler(std::move(sensorList));
            });
        }

        deleteRequest(requestRaw->serial());
    });
    requestRaw->run();
}

void AirlyConnection::getSensorData(Pollution sensor, std::function<void (Pollution)> handler)
{
    sensor.id = DEFAULT_SENSOR_ID;

    for (auto& pollution: m_parametersUnits) {
        if (pollution.pollutionCode == sensor.code) {
            sensor.unit = pollution.unit;
            break ;
        }
    }

    sensor.setInitialized(true);
    handler(sensor);
}

void AirlyConnection::getStationIndex(StationPtr station, std::function<void (StationIndexPtr)> handler)
{
    if (station == nullptr)
    {
        handler(StationIndexPtr(nullptr));
        return;
    }

    if (station->stationIndex() && !station->stationIndex()->shouldGetNewData(m_getStationIndexFrequency))
    {
        handler(StationIndexPtr(nullptr));
        return;
    }

    QString apiKey = m_modelsManager->providerListModel()->provider(id())->apiKey();
    QString url = "https://" + m_host + m_port + "/v2/measurements/installation?apikey=" + apiKey
            + "&indexType=AIRLY_CAQI&installationId=" + QString::number(station->id());
    QUrl stationIndexURL(url);

    Request* requestRaw = request(stationIndexURL);
    requestRaw->addHeader(QByteArray("Accept-Language"), QByteArray("en"));

    QObject::connect(requestRaw, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR) {
            parseHeaders(requestRaw->getResponseHeaders());
            StationIndexData stationIndexData;

            stationIndexData.m_id = -1;
            stationIndexData.m_name = "No index";

            StationIndexPtr stationIndex = std::make_shared<StationIndex>();
            stationIndex->setData(stationIndexData);
            handler(stationIndex);
        } else {
            StationIndexPtr stationIndex = readStationIndexFromJson(QJsonDocument::fromJson(responseArray));
            stationIndex->setDateToCurent();
            handler(stationIndex);
        }

        deleteRequest(requestRaw->serial());
    });
    requestRaw->run();
}

void AirlyConnection::getNearestStations(QGeoCoordinate coordinate, float distanceLimit, std::function<void (StationListPtr)> handler)
{
    QString apiKey = m_modelsManager->providerListModel()->provider(id())->apiKey();
    QString url = "https://" + m_host + m_port + "/v2/installations/nearest?apikey=" + apiKey
            + "&lat=" + QString::number(coordinate.latitude()) + "&lng=" + QString::number(coordinate.longitude())
            + "&maxDistanceKM=" + QString::number(distanceLimit / 1000) + "&maxResults=-1";
    QUrl stationListURL(url);

    Request* requestRaw = request(stationListURL);
    requestRaw->addHeader(QByteArray("Accept-Language"), QByteArray("en"));

    QObject::connect(requestRaw, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(StationListPtr{});
        else
        {
            parseHeaders(requestRaw->getResponseHeaders());
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

void AirlyConnection::parseHeaders(QList<QPair<QByteArray, QByteArray>>& headers)
{
    auto provider = m_modelsManager->providerListModel()->provider(id());

    for (auto header: headers) {
        QString headerString = header.first.toLower();

        if (headerString == "x-ratelimit-remaining-day") {
            provider->setRequestRemaining(header.second.toInt());
        } else if (headerString == "x-ratelimit-limit-day") {
            provider->setRequestLimit(header.second.toInt());
        }
    }
}

StationListPtr AirlyConnection::readStationsFromJson(const QJsonDocument &jsonDocument)
{
    StationListPtr stationList = std::make_shared<StationList>();

    QJsonArray array = jsonDocument.array();

    for (const auto& station: array)
    {
        auto stationObj = station.toObject();
        StationPtr item = std::make_shared<Station>();
        StationData stationData;
        stationData.id = stationObj["id"].toInt();

        auto adressObj = stationObj["address"].toObject();
        stationData.cityName = adressObj["city"].toString();
        stationData.street = adressObj["street"].toString();
        stationData.country = adressObj["country"].toString();
        stationData.provider = id();

        auto locationObj = stationObj["location"].toObject();
        double lat = locationObj["latitude"].toDouble();
        double lon = locationObj["longitude"].toDouble();
        stationData.coordinate = QGeoCoordinate(lat, lon);

        stationData.province = stationData.cityName;

        if (stationData.province.isNull())
            stationData.province = QObject::tr("OTHER");

        item->setStationData(stationData);
        stationList->append(item);
    }

    return stationList;
}

SensorListPtr AirlyConnection::readSensorsFromJson(const QJsonDocument &jsonDocument)
{
    if (jsonDocument.isNull())
        return SensorListPtr(nullptr);

    SensorListPtr sensorList = std::make_shared<SensorList>();

    auto current = jsonDocument.object()["current"];
    if (current.isUndefined()) {
        return sensorList;
    }
    auto currentObj = current.toObject();

    QJsonArray results = currentObj["values"].toArray();

    std::map<QString, Pollution> nameToSensorData;
    QDateTime currentResultDate;
    for (const auto& result: results) {
        auto resultObj = result.toObject();
        QString name = resultObj["name"].toString();
        float value = resultObj["value"].toDouble();

        QString dateString = currentObj["tillDateTime"].toString();
        currentResultDate = QDateTime::fromString(dateString, Qt::ISODate).toLocalTime();

        Pollution sensorData;
        sensorData.id = DEFAULT_SENSOR_ID;
        sensorData.name = name;
        sensorData.date = currentResultDate;

        sensorData.setValues(PollutionValue{value, currentResultDate});
        nameToSensorData[sensorData.name] = sensorData;
    }

    auto history = jsonDocument.object()["history"];
    if (history.isUndefined()) {
        return sensorList;
    }

    QJsonArray historyResults = history.toArray();
    for (const auto& historyResult: historyResults) {
        results = historyResult.toObject()["values"].toArray();
        for (const auto& result: results) {
            auto resultObj = result.toObject();
            QString name = resultObj["name"].toString();
            float value = resultObj["value"].toDouble();

            QString dateString = currentObj["tillDateTime"].toString();
            QDateTime date = QDateTime::fromString(dateString, Qt::ISODate).toLocalTime();

            if (date > currentResultDate) {
                continue;
            }

            auto sensorDataIt = nameToSensorData.find(name);
            if (sensorDataIt != nameToSensorData.end()) {
                sensorDataIt->second.setValues({value, date});
            }
        }
    }

    for (auto& sensorDataPair: nameToSensorData) {
        auto sensorData = sensorDataPair.second;
        sensorData.name = sensorData.name.toLower();
        sensorData.code = sensorData.name;

        if (sensorData.name == QStringLiteral("pm25"))
            sensorData.name = QStringLiteral("pm2.5");

        sensorData.name.replace(0, 1, sensorData.name[0].toUpper());

        sensorData.setInitialized(false);
        sensorList->setData(sensorData);
    }

    return std::move(sensorList);
}

StationIndexPtr AirlyConnection::readStationIndexFromJson(const QJsonDocument &jsonDocument)
{
    if (jsonDocument.isNull())
        return StationIndexPtr(nullptr);

    auto current = jsonDocument.object()["current"];
    if (current.isUndefined()) {
        std::cout << "Error in Airly Index json" << std::endl;
        return StationIndexPtr(nullptr);
    }
    auto currentObj = current.toObject();
    QString dateString = currentObj["tillDateTime"].toString();
    QJsonArray results = currentObj["indexes"].toArray();

    float value = 100.f;
    float id;
    QString name;

    for (const auto& result: results) {
        auto resultObj = result.toObject();
        value = resultObj["value"].toDouble();
        name = resultObj["description"].toString();
        break;
    }

    if (value < 25) {
        id = 0;
    } else if (value < 50) {
        id = 1;
    } else if (value < 75) {
        id = 2;
    } else if (value < 100) {
        id = 3;
    } else {
        id = 5;
    }

    QDateTime date = QDateTime::fromString(dateString, Qt::ISODate).toLocalTime();
    StationIndexData stationIndexData;

    if (!name.isEmpty())
        stationIndexData.m_id = id;
    stationIndexData.m_name = name;
    stationIndexData.m_date = date;
    stationIndexData.m_calculationModeName = m_indexName;

    StationIndexPtr stationIndex = std::make_shared<StationIndex>();
    stationIndex->setData(stationIndexData);
    return stationIndex;
}

PollutionUnitList AirlyConnection::readParametersUnitsFromJson(const QJsonDocument &jsonDocument)
{
    PollutionUnitList pollutionUnitList;

    QJsonArray array = jsonDocument.array();

    for (const auto& station: array)
    {
        auto stationObj = station.toObject();
        QString name = stationObj["name"].toString().toLower();
        QString unit = stationObj["unit"].toString();

        pollutionUnitList.push_back({name, unit});
    }

    return pollutionUnitList;
}
