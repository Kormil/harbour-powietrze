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
    m_indexName = "PIJP";
}

PowietrzeConnection::~PowietrzeConnection()
{

}

void PowietrzeConnection::getCountryList(std::function<void (CountryListPtr)> handler)
{
    CountryListPtr countryList = std::make_shared<CountryList>();

    CountryItemPtr poland = std::make_shared<CountryItem>();
    poland->name = QObject::tr("Poland");
    poland->code = countryCode();
    poland->provider = id();

    countryList->append(poland);

    handler(std::move(countryList));
}

void PowietrzeConnection::getStationList(std::function<void(StationListPtr)> handler)
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

void PowietrzeConnection::stationListRequest(std::function<void (StationListPtr)> handler)
{
    QString url = "http://" + m_host + m_port + "/station/findAll";

    RequestPtr request = createRequest(url);

    auto requestHandler = [this, handler](Request::Status status, const QByteArray& responseArray, const Request::ResponseHeaders&) {
        if (status == Request::ERROR)
            handler(StationListPtr(nullptr));
        else
        {
            m_lastStationListRequestDate = QDateTime::currentDateTime();
            StationListPtr stationList = readStationsFromJson(QJsonDocument::fromJson(responseArray));
            m_cashedStations = stationList;
            handler(stationList);
        }
    };

    request->run(requestHandler);
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

    ProvinceListPtr provinceList = std::make_shared<ProvinceList>();
    for (const auto value: names)
    {
        ProvinceItemPtr province = std::make_shared<ProvinceItem>();
        province->name = value;
        province->countryCode = countryCode();
        province->provider = id();
        provinceList->append(province);
    }

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

    RequestPtr request = createRequest(provinceListURL);

    auto requestHandler = [this, handler](Request::Status status, const QByteArray& responseArray, const Request::ResponseHeaders&) {
        if (status == Request::ERROR) {
            handler( SensorListPtr{} );
        } else {
            SensorListPtr sensorList = readSensorsFromJson(QJsonDocument::fromJson(responseArray));
            sensorList->setDateToCurrent();
            handler(std::move(sensorList));
        }
    };

    request->run(requestHandler);
}

void PowietrzeConnection::getSensorData(Pollution sensor, std::function<void (Pollution)> handler)
{
    QString url = "http://" + m_host + m_port + "/data/getData/" + QString::number(sensor.id.toInt());
    QUrl sensorDataURL(url);

    RequestPtr request = createRequest(sensorDataURL);

    auto requestHandler = [this, handler, sensor](Request::Status status, const QByteArray& responseArray, const Request::ResponseHeaders&) {
        Pollution data;
        if (status != Request::ERROR) {
            data = readSensorDataFromJson(QJsonDocument::fromJson(responseArray));
        }

        data.id = sensor.id.toInt();
        data.name = sensor.name;
        data.code = sensor.code;

        handler(data);
    };

    request->run(requestHandler);
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

    RequestPtr request = createRequest(stationIndexURL);

    auto requestHandler = [this, handler](Request::Status status, const QByteArray& responseArray, const Request::ResponseHeaders&) {
        if (status == Request::ERROR) {
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
    };

    request->run(requestHandler);
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
    ProvinceListPtr provinceList = std::make_shared<ProvinceList>();

    QJsonArray array = jsonDocument.array();

    for (const auto& station: array)
    {
        ProvinceItemPtr item = std::make_shared<ProvinceItem>();
        item->name = station.toObject()["city"].toObject()["commune"].toObject()["provinceName"].toString();
        item->countryCode = countryCode();
        item->provider = id();

        provinceList->append(item);
    }

    return provinceList;
}

StationListPtr PowietrzeConnection::readStationsFromJson(const QJsonDocument &jsonDocument)
{
    StationListPtr stationList = std::make_shared<StationList>();

    QJsonArray array = jsonDocument.array();
    for (const auto& station: array)
    {
        auto stationObj = station.toObject();
        StationPtr item = std::make_shared<Station>();
        StationData stationData;
        stationData.id = stationObj["id"].toInt();
        stationData.cityName = stationObj["city"].toObject()["name"].toString();
        stationData.street = stationObj["addressStreet"].toString();
        stationData.provider = id();
        stationData.country = countryCode();

        double lat = stationObj["gegrLat"].toString().toDouble();
        double lon = stationObj["gegrLon"].toString().toDouble();
        stationData.coordinate = QGeoCoordinate(lat, lon);

        stationData.province = stationObj["city"].toObject()["commune"].toObject()["provinceName"].toString();

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

    SensorListPtr sensorList = std::make_shared<SensorList>();

    QJsonArray array = jsonDocument.array();

    for (const auto& sensor: array)
    {
        Pollution sensorData;

        auto sensorObj = sensor.toObject();
        auto sensorParamObj = sensorObj["param"].toObject();
        sensorData.id = sensorObj["id"].toInt();
        sensorData.name = sensorParamObj["paramName"].toString();
        sensorData.code = sensorParamObj["paramCode"].toString().toLower();


        if (sensorData.code == QStringLiteral("pm2.5"))
            sensorData.code = QStringLiteral("pm25");

        sensorList->setData(sensorData);
    }

    return sensorList;
}

Pollution PowietrzeConnection::readSensorDataFromJson(const QJsonDocument &jsonDocument)
{
    QJsonArray array = jsonDocument.object()["values"].toArray();

    Pollution sensorData;
    QDateTime mainDate;
    std::vector<PollutionValue> values;
    for (const auto& sensor: array)
    {
        auto sensorObj = sensor.toObject();
        if (sensorObj["value"].isNull())
            continue;

        float value = sensorObj["value"].toDouble();
        QString dateString = sensorObj["date"].toString();
        auto date = QDateTime::fromString(dateString, dateFormat());

        values.push_back(PollutionValue{value, date});

        if (mainDate.isNull()) {
            mainDate = date;
        }
    }
    sensorData.setValues(values);
    sensorData.date = mainDate;
    sensorData.unit = "µg/m³";

    return sensorData;
}

StationIndexPtr PowietrzeConnection::readStationIndexFromJson(const QJsonDocument &jsonDocument)
{
    if (jsonDocument.isNull())
        return StationIndexPtr(nullptr);

    QJsonObject object = jsonDocument.object()["stIndexLevel"].toObject();

    float id = object["id"].toInt();
    QString name = object["indexLevelName"].toString();
    QString dateString = jsonDocument.object()["stSourceDataDate"].toString();

    StationIndexData stationIndexData;

    if (!name.isEmpty())
        stationIndexData.m_id = id;
    stationIndexData.m_name = name;

    QDateTime date = QDateTime::fromString(dateString, dateFormat());
    stationIndexData.m_date = date;
    stationIndexData.m_calculationModeName = m_indexName;

    StationIndexPtr stationIndex = std::make_shared<StationIndex>();
    stationIndex->setData(stationIndexData);
    return stationIndex;
}
