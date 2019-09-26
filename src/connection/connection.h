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

    ~Request()
    {
        networkReply->deleteLater();
    }

    int serial() const;
    void setSerial(int serial);

private:
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

    virtual void countryListRequest(std::function<void(CountryListPtr)> handler) = 0;
    virtual void stationListRequest(std::function<void(StationListPtr)> handler) = 0;
    virtual void provinceListRequest(std::function<void(ProvinceListPtr)> handler) = 0;
    virtual void sensorListRequest(StationPtr station, std::function<void(SensorListPtr)> handler) = 0;
    virtual void sensorDataRequest(SensorData sensor, std::function<void (SensorData)> handler) = 0;
    virtual void stationIndexRequest(StationPtr station, std::function<void(StationIndexPtr)> handler) = 0;
    virtual void findNearestStationRequest(QGeoCoordinate coordinate, int limit, std::function<void(StationListPtr)> handler) = 0;

    QNetworkAccessManager *networkAccessManager();

    Request* request(const QUrl &requestUrl);
    void deleteRequest(int serial);

    int countryListRequestFrequency() const;
    int stationListRequestFrequency() const;
    int provinceListRequestFrequency() const;
    int sensorListRequestFrequency() const;

    int id() const;

    void clearRequests();
protected:
    int nextSerial();

    QString m_host;
    QString m_port;

    int m_id;

    ModelsManager* m_modelsManager;

    //Request frequency
    int m_countryListRequestFrequency = 60 * 60 * 24; //one day
    int m_stationListRequestFrequency = 60 * 60 * 24; //one day
    int m_provinceListRequestFrequency = 60 * 60 * 24; //one day
    int m_sensorListRequestFrequency  = 30 * 60; //30 minutes
    int m_stationIndexRequestFrequency  = 30 * 60; //30 minutes

    QDateTime m_lastCountryListRequestDate;
    QDateTime m_lastStationListRequestDate;
    QDateTime m_lastProvinceListRequestDate;
    QDateTime m_lastSensorListRequestDate;

    std::mutex m_stationListRequestMutex;

private:
    QNetworkAccessManager* m_networkAccessManager;
    std::map<int, RequestPtr> m_networkRequests;
    std::atomic<int> m_serial;

    std::mutex m_networkRequestsMutex;
};

#endif // CONNECTION_H
