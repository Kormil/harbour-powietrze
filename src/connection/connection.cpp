#include "connection.h"
#include <iostream>
#include <notification.h>
#include "../modelsmanager.h"

Request::Request(const QUrl &url, Connection *connection) :
    m_networkRequest(url),
    m_connection(connection)
{
}

void Request::run()
{
    networkReply = m_connection->networkAccessManager()->get(m_networkRequest);

    QObject::connect(networkReply, &QIODevice::readyRead, [this]() {
        responseArray.append(networkReply->readAll());
    });

    QObject::connect(networkReply, &QNetworkReply::finished, [this]() {
        if (networkReply->error() != QNetworkReply::NoError)
        {
            std::cout << networkReply->errorString().toStdString() << std::endl;
            Notification notification;
            notification.setPreviewBody(networkReply->errorString());
            notification.publish();
            emit finished(ERROR, QByteArray());
            return ;
        }

        emit finished(SUCCESS, responseArray);
    });
}

void Request::addHeader(const QByteArray &key, const QByteArray &value)
{
    m_networkRequest.setRawHeader(key, value);
}

int Request::serial() const
{
    return m_serial;
}

void Request::setSerial(int serial)
{
    m_serial = serial;
}

Connection::Connection(ModelsManager* modelsManager) :
    m_modelsManager(modelsManager)
{
    m_networkAccessManager = new QNetworkAccessManager;
}

void Connection::deleteRequest(int serial)
{
    std::lock_guard<std::mutex> lock(m_networkRequestsMutex);
    m_networkRequests.erase( m_networkRequests.find(serial) );
}

QNetworkAccessManager* Connection::networkAccessManager()
{
    return m_networkAccessManager;
}

int Connection::nextSerial()
{
    m_serial = m_serial + 1;
    return m_serial;
}

int Connection::getStationIndexFrequency() const
{
    return m_getStationIndexFrequency;
}

int Connection::id() const
{
    return m_id;
}

void Connection::clearRequests()
{
    m_networkAccessManager->deleteLater();
}

int Connection::getSensorListFrequency() const
{
    return m_getSensorListFrequency;
}

int Connection::getProvinceListFrequency() const
{
    return m_getProvinceListFrequency;
}

int Connection::getStationListFrequency() const
{
    return m_getStationListFrequency;
}

int Connection::getCountryListFrequency() const
{
    return m_getCountryListFrequency;
}

Request* Connection::request(const QUrl &requestUrl)
{
    std::lock_guard<std::mutex> lock(m_networkRequestsMutex);

    int serial = nextSerial();

    RequestPtr requestPtr (new Request(requestUrl, this));

    requestPtr->setSerial(serial);
    m_networkRequests[serial] = std::move(requestPtr);

    return m_networkRequests[serial].get();
}
