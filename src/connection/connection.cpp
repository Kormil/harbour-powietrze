#include "connection.h"
#include <iostream>
#include <notification.h>
#include "../modelsmanager.h"

namespace {
    int REQUEST_TIMEOUT = 20000;
}

Request::Request(const QUrl &url, Connection *connection) :
    m_networkRequest(url),
    m_connection(connection),
    m_shutdown(false)
{
    m_requestTimer.setSingleShot(true);

    QObject::connect(&m_requestTimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void Request::run(RequestHandler handler)
{
    m_handler = handler;
    networkReply = m_connection->networkAccessManager()->get(m_networkRequest);
    m_requestTimer.start(REQUEST_TIMEOUT);

    QObject::connect(networkReply, &QIODevice::readyRead, [this]() {
        responseArray.append(networkReply->readAll());
    });

    QObject::connect(networkReply, &QNetworkReply::finished, [this]() {
        responseFinished(networkReply->error(), networkReply->errorString());
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

void Request::timeout()
{
    networkReply->abort();
}

void Request::responseFinished(QNetworkReply::NetworkError error, QString errorString)
{
    m_requestTimer.stop();

    if (error != QNetworkReply::NoError)
    {
        //Notification notification;
        //notification.setPreviewBody(errorString);
        //notification.publish();
        qDebug() << errorString;
        m_handler(ERROR, QByteArray(), ResponseHeaders());
    } else {
        responseHeaders = networkReply->rawHeaderPairs();
        m_handler(SUCCESS, responseArray, responseHeaders);
    }

    m_shutdown = true;
}

bool Request::shutdown() const
{
    return m_shutdown;
}

QList<QPair<QByteArray, QByteArray>>& Request::getResponseHeaders()
{
    return responseHeaders;
}

Connection::Connection(ModelsManager* modelsManager) :
    m_modelsManager(modelsManager)
{
    m_networkAccessManager = std::make_unique<QNetworkAccessManager>();
}

QNetworkAccessManager* Connection::networkAccessManager()
{
    return m_networkAccessManager.get();
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

QString Connection::indexName() const
{
    return m_indexName;
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

RequestPtr Connection::createRequest(const QUrl &requestUrl)
{
    std::lock_guard<std::mutex> lock(m_networkRequestsMutex);

    // remove old requests:
    for (auto request_it = m_networkRequests.begin(); request_it != m_networkRequests.end();) {
        if (request_it->second && request_it->second->shutdown()) {
            request_it = m_networkRequests.erase(request_it);
        } else {
            ++request_it;
        }
    }

    // create new request
    int serial = nextSerial();

    RequestPtr requestPtr = std::make_shared<Request>(requestUrl, this);

    requestPtr->setSerial(serial);
    m_networkRequests[serial] = requestPtr;

    return requestPtr;
}
