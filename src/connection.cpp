#include "connection.h"
#include <iostream>
#include <notification.h>
#include "modelsmanager.h"

Request::Request(const QUrl &url, Connection *connection)
{
    m_serial = connection->nextSerial();
    networkReply = connection->networkAccessManager().get(QNetworkRequest(url));

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

int Request::serial() const
{
    return m_serial;
}

Connection::Connection(ModelsManager* modelsManager) :
    m_modelsManager(modelsManager)
{
    m_host = "api.gios.gov.pl/pjp-api/rest";
    m_port = "";

    m_frequency = 30;
}

void Connection::deleteRequest(int serial)
{
    m_networkRequests.erase( m_networkRequests.find(serial) );
}

int Connection::frequency() const
{
    return m_frequency;
}

QNetworkAccessManager &Connection::networkAccessManager()
{
    return m_networkAccessManager;
}

int Connection::nextSerial()
{
    ++m_serial;
    return m_serial;
}

void Connection::stationListRequest(std::function<void(StationListPtr)> handler)
{
    if (!m_modelsManager->stationListModel() || !m_modelsManager->stationListModel()->shouldGetNewData())
    {
        handler(StationListPtr{});
        return ;
    }

    m_modelsManager->stationListModel()->setDateToCurrent();

    QString url = "http://" + m_host + m_port + "/station/findAll";
    QUrl stationListURL(url);

    Request* request = new Request(stationListURL, this);
    m_networkRequests[m_serial] = RequestPtr(request);

    QObject::connect(request, &Request::finished, [this, request, handler](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler(StationListPtr(nullptr));
        else
        {
            StationListPtr stationList = StationList::getFromJson(QJsonDocument::fromJson(responseArray));
            handler(std::move(stationList));
        }

        deleteRequest(request->serial());
    });
}

void Connection::sensorListRequest(const int& stationId, std::function<void (SensorListPtr)> handler)
{
    Station* station = m_modelsManager->stationListModel()->station(stationId);

    if (station == nullptr)
    {
        handler(SensorListPtr{});
        return;
    }

    if (station->sensorList() != nullptr && !station->sensorList()->shouldGetNewData(frequency()))
    {
        handler(SensorListPtr{});
        return;
    }

    QString url = "http://" + m_host + m_port + "/station/sensors/" + QString::number(stationId);
    QUrl provinceListURL(url);

    Request* request = new Request(provinceListURL, this);
    m_networkRequests[m_serial] = RequestPtr(request);

    QObject::connect(request, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler( SensorListPtr() );
        else
        {
            SensorListPtr sensorList = SensorList::getSensorsFromJson(QJsonDocument::fromJson(responseArray));
            sensorList->setDateToCurent();
            handler(std::move(sensorList));
        }

        deleteRequest(request->serial());
    });
}

void Connection::sensorDataRequest(const int& sensorId, std::function<void (float)> handler)
{
    QString url = "http://" + m_host + m_port + "/data/getData/" + QString::number(sensorId);
    QUrl sensorDataURL(url);

    Request* request = new Request(sensorDataURL, this);
    m_networkRequests[m_serial] = RequestPtr(request);

    QObject::connect(request, &Request::finished, [this, request, handler](Request::Status status, const QByteArray& responseArray) {
        float sensorValue;

        if (status == Request::ERROR)
            sensorValue = 0.0f;
        else
            sensorValue = SensorList::getSensorDataFromJson(QJsonDocument::fromJson(responseArray));

        handler(sensorValue);
        deleteRequest(request->serial());
    });
}

void Connection::stationIndexRequest(const int& stationId, std::function<void (StationIndexPtr)> handler)
{
    const Station* station = m_modelsManager->stationListModel()->station(stationId);

    if (station == nullptr)
    {
        handler(StationIndexPtr(nullptr));
        return;
    }

    if (station->stationIndex() && !station->stationIndex()->shouldGetNewData(frequency()))
    {
        handler(station->stationIndexPtr());
        return;
    }

    QString url = "http://" + m_host + m_port + "/aqindex/getIndex/" + QString::number(stationId);
    QUrl stationIndexURL(url);

    Request* request = new Request(stationIndexURL, this);
    m_networkRequests[m_serial] = RequestPtr(request);

    QObject::connect(request, &Request::finished, [=](Request::Status status, const QByteArray& responseArray) {
        if (status == Request::ERROR)
            handler( StationIndexPtr(nullptr) );
        else
        {
            StationIndexPtr stationIndex = StationIndex::getFromJson(QJsonDocument::fromJson(responseArray));
            stationIndex->setDateToCurent();
            handler(std::move(stationIndex));
        }

        deleteRequest(request->serial());
    });
}

void Connection::findNearestStationRequest(QGeoCoordinate coordinate, std::function<void (StationListPtr)> handler)
{
    m_modelsManager->stationListModel()->calculateDistances(coordinate);
    handler(StationListPtr());
}
