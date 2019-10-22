#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <iostream>

#include "Types/stationlist.h"
#include "Types/provincelist.h"
#include "Types/sensorlist.h"
#include "Types/countrylist.h"

class Connection;
class ModelsManager;

class Request : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        SUCCESS,
        ERROR
    };

    Request(const QUrl& url, Connection *connection);
    void run();

    void addHeader(const QByteArray& key, const QByteArray& value);

    ~Request()
    {
        networkReply->deleteLater();
    }

    int serial() const;
    void setSerial(int serial);

private:
    QNetworkRequest m_networkRequest;
    Connection *m_connection;
    QNetworkReply* networkReply;
    QByteArray responseArray;
    int m_serial;

signals:
    void finished(Status, const QByteArray&);
};

using PatameterList = std::map<QString, QString>;
using RequestPtr = std::unique_ptr<Request>;

class Connection
{
public:
    enum Errors {
        NoData = -1
    };

    Connection(ModelsManager* modelsManager);
    virtual ~Connection() {}

    virtual void getCountryList(std::function<void(CountryListPtr)> handler) = 0;
    virtual void getStationList(std::function<void(StationListPtr)> handler) = 0;
    virtual void getProvinceList(std::function<void(ProvinceListPtr)> handler) = 0;
    virtual void getSensorList(StationPtr station, std::function<void(SensorListPtr)> handler) = 0;
    virtual void getSensorData(SensorData sensor, std::function<void (SensorData)> handler) = 0;
    virtual void getStationIndex(StationPtr station, std::function<void(StationIndexPtr)> handler) = 0;
    virtual void getNearestStations(QGeoCoordinate coordinate, float distanceLimit, std::function<void(StationListPtr)> handler) = 0;

    QNetworkAccessManager *networkAccessManager();

    Request* request(const QUrl &requestUrl);
    void deleteRequest(int serial);

    int getCountryListFrequency() const;
    int getStationListFrequency() const;
    int getProvinceListFrequency() const;
    int getSensorListFrequency() const;
    int getStationIndexFrequency() const;

    int id() const;

    void clearRequests();

protected:
    int nextSerial();

    QString m_host;
    QString m_port;

    int m_id;

    ModelsManager* m_modelsManager;

    //Request frequency
    int m_getCountryListFrequency = 60 * 60 * 24; //one day
    int m_getStationListFrequency = 60 * 60 * 24; //one day
    int m_getProvinceListFrequency = 60 * 60 * 24; //one day
    int m_getSensorListFrequency  = 30 * 60; //30 minutes
    int m_getStationIndexFrequency  = 30 * 60; //30 minutes

    QDateTime m_lastCountryListRequestDate;
    QDateTime m_lastStationListRequestDate;
    QDateTime m_lastProvinceListRequestDate;

    std::mutex m_getStationListMutex;

    //Cach
    CountryListPtr m_cashedCountries;
    ProvinceListPtr m_cashedProvinces;
    StationListPtr m_cashedStations;
private:
    QNetworkAccessManager* m_networkAccessManager;
    std::map<int, RequestPtr> m_networkRequests;
    std::atomic<int> m_serial;

    std::mutex m_networkRequestsMutex;
};

#endif // CONNECTION_H
