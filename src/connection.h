#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <string>
#include <map>
#include <memory>
#include <iostream>

#include "Types/stationlist.h"
#include "Types/provincelist.h"
#include "Types/sensorlist.h"

class Connection;

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

private:
    QNetworkReply* networkReply;
    QByteArray responseArray;
    int m_serial;

signals:
    void finished(Status, const QByteArray&);
};

using RequestPtr = std::unique_ptr<Request>;

class Connection
{
public:
    Connection();

    void stationListRequest(std::function<void(StationListPtr)> handler);
    void sensorListRequest( const int& stationId, std::function<void(SensorListPtr)> handler);
    void sensorDataRequest(const int& sensorId, std::function<void (float)> handler);
    void stationIndexRequest(const int& stationId, std::function<void(StationIndexPtr)> handler);

    QNetworkAccessManager &networkAccessManager();

    int nextSerial();
    void deleteRequest(int serial);
    int frequency() const;

private:
    QString m_host;
    QString m_port;
    int m_frequency;

    QNetworkAccessManager m_networkAccessManager;
    std::map<int, RequestPtr> m_networkRequests;
    int m_serial;
};

#endif // CONNECTION_H
