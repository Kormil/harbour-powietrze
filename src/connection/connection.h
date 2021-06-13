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
#include <functional>
#include <atomic>

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
    using ResponseHeaders = QList<QPair<QByteArray, QByteArray>>;
    using RequestHandler = std::function<void(Request::Status status, const QByteArray& responseArray, const ResponseHeaders& headers)>;

    Request(const QUrl& url, Connection *connection);
    void run(RequestHandler handler);

    void addHeader(const QByteArray& key, const QByteArray& value);
    QList<QPair<QByteArray, QByteArray> > &getResponseHeaders();

    ~Request()
    {
        if (networkReply) {
            networkReply->deleteLater();
        }
    }

    int serial() const;
    void setSerial(int serial);

    bool shutdown() const;

private slots:
    void timeout();
    void responseFinished(QNetworkReply::NetworkError error, QString errorString);

private:
    QNetworkRequest m_networkRequest;
    QTimer m_requestTimer;
    Connection *m_connection;
    QNetworkReply* networkReply;
    QByteArray responseArray;
    ResponseHeaders responseHeaders;
    int m_serial;
    std::atomic_bool m_shutdown;

    RequestHandler m_handler;

signals:
    void finished(Status, const QByteArray&);
};

using PatameterList = std::map<QString, QString>;
using RequestPtr = std::shared_ptr<Request>;

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
    virtual void getSensorData(Pollution sensor, std::function<void (Pollution)> handler) = 0;
    virtual void getStationIndex(StationPtr station, std::function<void(StationIndexPtr)> handler) = 0;
    virtual void getNearestStations(QGeoCoordinate coordinate, float distanceLimit, std::function<void(StationListPtr)> handler) = 0;

    QNetworkAccessManager *networkAccessManager();

    RequestPtr createRequest(const QUrl &requestUrl);

    int getCountryListFrequency() const;
    int getStationListFrequency() const;
    int getProvinceListFrequency() const;
    int getSensorListFrequency() const;
    int getStationIndexFrequency() const;

    int id() const;
    QString indexName() const;

    void clearRequests();

protected:
    int nextSerial();

    QString m_host;
    QString m_port;

    int m_id;
    QString m_indexName;

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
    StationListPtr m_cashedStations;
private:
    std::unique_ptr<QNetworkAccessManager> m_networkAccessManager;
    std::map<int, RequestPtr> m_networkRequests;
    std::atomic<int> m_serial = 0;

    std::mutex m_networkRequestsMutex;
};

#endif // CONNECTION_H
