#include "airlyconnection.h"
#include <QStringBuilder>
#include <utility>
#include <notification.h>
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

    CountryListPtr countryList(new CountryList());
    for (const auto value: names)
    {
        CountryItemPtr country(new CountryItem());
        country->name = value;
        country->code = value;
        country->provider = id();
        countryList->append(country);
    }

    m_cashedCountries = countryList;
    handler(countryList);
}

void AirlyConnection::getStationList(std::function<void (StationListPtr)> handler)
{
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_lastStationListRequestDate.isValid()
            && currentTime.secsTo(m_lastStationListRequestDate) < m_getStationListFrequency)
    {
        handler(m_cashedStations);
        return ;
    }

    m_lastStationListRequestDate = currentTime;
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
    requestRaw->run();

    QObject::connect(requestRaw, &Request::finished, [this, requestRaw, handler](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(StationListPtr(nullptr));
        else
        {
            StationListPtr stationList = readStationsFromJson(QJsonDocument::fromJson(responseArray));
            m_cashedStations = stationList;
            handler(stationList);
        }

        deleteRequest(requestRaw->serial());
    });
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

    ProvinceListPtr provinceList(new ProvinceList());
    for (const auto value: names)
    {
        ProvinceItemPtr province(new ProvinceItem());
        province->name = value.first;
        province->countryCode = value.second;
        province->provider = id();
        provinceList->append(province);
    }

    m_cashedProvinces = provinceList;
    handler(provinceList);
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
    requestRaw->run();

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

void AirlyConnection::getSensorData(SensorData sensor, std::function<void (SensorData)> handler)
{
    sensor.id = DEFAULT_SENSOR_ID;
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
    requestRaw->run();

    QObject::connect(requestRaw, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR) {
            StationIndexPtr stationIndex(new StationIndex);

            stationIndex->setId(-1);
            stationIndex->setName("No index");

            handler(stationIndex);
        } else {
            StationIndexPtr stationIndex = readStationIndexFromJson(QJsonDocument::fromJson(responseArray));
            stationIndex->setDateToCurent();
            handler(stationIndex);
        }

        deleteRequest(requestRaw->serial());
    });
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
    requestRaw->run();

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
}

StationListPtr AirlyConnection::readStationsFromJson(const QJsonDocument &jsonDocument)
{
    StationListPtr stationList(new StationList());

    QJsonArray array = jsonDocument.array();

    for (const auto& station: array)
    {
        StationPtr item = StationPtr(new Station());
        StationData stationData;
        stationData.id = station.toObject()["id"].toInt();
        stationData.cityName = station.toObject()["address"].toObject()["city"].toString();
        stationData.street = station.toObject()["address"].toObject()["street"].toString();
        stationData.country = station.toObject()["address"].toObject()["country"].toString();
        stationData.provider = id();

        double lat = station.toObject()["location"].toObject()["latitude"].toDouble();
        double lon = station.toObject()["location"].toObject()["longitude"].toDouble();
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

    SensorListPtr sensorList(new SensorList());

    auto current = jsonDocument.object()["current"];
    if (current.isUndefined()) {
        return sensorList;
    }

    QJsonArray results = current.toObject()["values"].toArray();

    std::map<QString, SensorData> nameToSensorData;
    for (const auto& result: results) {
        QString name = result.toObject()["name"].toString();
        float value = result.toObject()["value"].toDouble();

        SensorData sensorData;
        sensorData.id = DEFAULT_SENSOR_ID;
        sensorData.name = name;

        sensorData.setValues(value);
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
            QString name = result.toObject()["name"].toString();
            float value = result.toObject()["value"].toDouble();

            auto sensorDataIt = nameToSensorData.find(name);
            if (sensorDataIt != nameToSensorData.end()) {
                sensorDataIt->second.setValues(value);
            }
        }
    }

    for (auto& sensorDataPair: nameToSensorData) {
        auto sensorData = sensorDataPair.second;
        sensorData.name = sensorData.name.toLower();

        if (sensorData.name == QStringLiteral("pm25"))
            sensorData.name = QStringLiteral("pm2.5");

        sensorData.pollutionCode = sensorData.name;
        sensorData.name.replace(0, 1, sensorData.name[0].toUpper());

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
        return StationIndexPtr(nullptr);
    }

    QJsonArray results = current.toObject()["indexes"].toArray();

    float value;
    float id;
    QString name;

    for (const auto& result: results) {
        value = result.toObject()["value"].toDouble();
        name = result.toObject()["description"].toString();
        break;
    }

    if (value < 25) {
        id = 0;
    } else if (value < 50) {
        id = 1;
    } else if (value < 75) {
        id = 3;
    } else if (value < 100) {
        id = 4;
    } else {
        id = 5;
    }

    StationIndex* stationIndex = new StationIndex;

    if (!name.isEmpty())
        stationIndex->setId(id);
    stationIndex->setName(name);
    return StationIndexPtr( stationIndex );
}
