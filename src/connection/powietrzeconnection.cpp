#include "powietrzeconnection.h"
#include <iostream>
#include <set>
#include <notification.h>
#include "src/modelsmanager.h"
#include "Types/stationindex.h"

PowietrzeConnection::PowietrzeConnection(ModelsManager* modelsManager) :
    Connection(modelsManager)
{
    m_host = "api.gios.gov.pl/pjp-api/rest";
    m_port = "";

    m_id = 1;
}

PowietrzeConnection::~PowietrzeConnection()
{

}

void PowietrzeConnection::getCountryList(std::function<void (CountryListPtr)> handler)
{
    CountryListPtr countryList(new CountryList());

    CountryItemPtr poland(new CountryItem);
    poland->name = QObject::tr("Poland");
    poland->code = countryCode();
    poland->provider = id();

    countryList->append(poland);

    m_cashedCountries = countryList;
    handler(std::move(countryList));
}

void PowietrzeConnection::getStationList(std::function<void(StationListPtr)> handler)
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

void PowietrzeConnection::stationListRequest(std::function<void (StationListPtr)> handler)
{
    QString url = "http://" + m_host + m_port + "/station/findAll";

    Request* requestRaw = request(url);
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

void PowietrzeConnection::getProvinceList(std::function<void(ProvinceListPtr)> handler)
{
    getStationList([this, handler](StationListPtr stationList) {
        provinceListRequest(stationList, handler);
    });
}

void PowietrzeConnection::provinceListRequest(StationListPtr stationList, std::function<void(ProvinceListPtr)> handler)
{
    if (stationList == nullptr) {
        return;
    }

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
        province->countryCode = countryCode();
        province->provider = id();
        provinceList->append(province);
    }

    m_cashedProvinces = provinceList;
    handler(provinceList);
}

void PowietrzeConnection::getSensorList(StationPtr station, std::function<void(SensorListPtr)> handler)
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

    QString url = "http://" + m_host + m_port + "/station/sensors/" + QString::number(station->id());
    QUrl provinceListURL(url);

    Request* requestRaw = request(provinceListURL);
    requestRaw->run();

    QObject::connect(requestRaw, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR) {
            handler( SensorListPtr{} );
        } else {
            SensorListPtr sensorList = readSensorsFromJson(QJsonDocument::fromJson(responseArray));
            sensorList->setDateToCurrent();
            handler(std::move(sensorList));
        }

        deleteRequest(requestRaw->serial());
    });
}

void PowietrzeConnection::getSensorData(SensorData sensor, std::function<void (SensorData)> handler)
{
    QString url = "http://" + m_host + m_port + "/data/getData/" + QString::number(sensor.id.toInt());
    QUrl sensorDataURL(url);

    Request* requestRaw = request(sensorDataURL);
    requestRaw->run();

    QObject::connect(requestRaw, &Request::finished, [this, requestRaw, handler, sensor](Request::Status status, const QByteArray& responseArray) {
        SensorData data;
        if (status != Request::ERROR) {
            data = readSensorDataFromJson(QJsonDocument::fromJson(responseArray));
        }

        data.id = sensor.id.toInt();
        data.name = sensor.name;
        data.pollutionCode = sensor.pollutionCode;

        handler(data);
        deleteRequest(requestRaw->serial());
    });
}

void PowietrzeConnection::getStationIndex(StationPtr station, std::function<void (StationIndexPtr)> handler)
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

    QString url = "http://" + m_host + m_port + "/aqindex/getIndex/" + QString::number(station->id());
    QUrl stationIndexURL(url);

    Request* requestRaw = request(stationIndexURL);
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

void PowietrzeConnection::getNearestStations(QGeoCoordinate coordinate, float, std::function<void (StationListPtr)> handler)
{
    getStationList([=](StationListPtr stationList) {
        if (!stationList) {
            m_modelsManager->stationListModel()->calculateDistances(coordinate);
        } else {
            stationList->calculateDistances(coordinate);
        }

        handler(stationList);
    });
}

ProvinceListPtr PowietrzeConnection::readProvincesFromJson(const QJsonDocument &jsonDocument)
{
    ProvinceListPtr provinceList(new ProvinceList());

    QJsonArray array = jsonDocument.array();

    for (const auto& station: array)
    {
        ProvinceItemPtr item(new ProvinceItem);
        item->name = station.toObject()["city"].toObject()["commune"].toObject()["provinceName"].toString();
        item->countryCode = countryCode();
        item->provider = id();

        provinceList->append(item);
    }

    return std::move(provinceList);
}

StationListPtr PowietrzeConnection::readStationsFromJson(const QJsonDocument &jsonDocument)
{
    StationListPtr stationList(new StationList());

    QJsonArray array = jsonDocument.array();

    for (const auto& station: array)
    {
        StationPtr item = StationPtr(new Station());
        StationData stationData;
        stationData.id = station.toObject()["id"].toInt();
        stationData.cityName = station.toObject()["city"].toObject()["name"].toString();
        stationData.street = station.toObject()["addressStreet"].toString();
        stationData.provider = id();
        stationData.country = countryCode();

        double lat = station.toObject()["gegrLat"].toString().toDouble();
        double lon = station.toObject()["gegrLon"].toString().toDouble();
        stationData.coordinate = QGeoCoordinate(lat, lon);

        stationData.province = station.toObject()["city"].toObject()["commune"].toObject()["provinceName"].toString();

        if (stationData.province.isNull())
            stationData.province = QObject::tr("OTHER");

        item->setStationData(stationData);
        stationList->append(item);
    }

    return stationList;
}

SensorListPtr PowietrzeConnection::readSensorsFromJson(const QJsonDocument &jsonDocument)
{
    if (jsonDocument.isNull())
        return SensorListPtr(nullptr);

    SensorListPtr sensorList(new SensorList());

    QJsonArray array = jsonDocument.array();

    for (const auto& sensor: array)
    {
        SensorData sensorData;

        sensorData.id = sensor.toObject()["id"].toInt();
        sensorData.name = sensor.toObject()["param"].toObject()["paramName"].toString();
        sensorData.pollutionCode = sensor.toObject()["param"].toObject()["paramCode"].toString().toLower();


        if (sensorData.pollutionCode == QStringLiteral("pm2.5"))
            sensorData.pollutionCode = QStringLiteral("pm25");

        sensorList->setData(sensorData);
    }

    return std::move(sensorList);
}

SensorData PowietrzeConnection::readSensorDataFromJson(const QJsonDocument &jsonDocument)
{
    QJsonArray array = jsonDocument.object()["values"].toArray();

    SensorData sensorData;
    std::vector<float> values;
    for (const auto& sensor: array)
    {
        if (sensor.toObject()["value"].isNull())
            continue;

        values.push_back(sensor.toObject()["value"].toDouble());
    }
    sensorData.setValues(values);

    return sensorData;
}

StationIndexPtr PowietrzeConnection::readStationIndexFromJson(const QJsonDocument &jsonDocument)
{
    if (jsonDocument.isNull())
        return StationIndexPtr(nullptr);

    QJsonObject object = jsonDocument.object()["stIndexLevel"].toObject();

    float id = object["id"].toInt();
    QString name = object["indexLevelName"].toString();

    StationIndex* stationIndex = new StationIndex;

    if (!name.isEmpty())
        stationIndex->setId(id);
    stationIndex->setName(name);
    return StationIndexPtr( stationIndex );
}
