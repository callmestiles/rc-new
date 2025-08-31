#include "NetworkManager.h"
#include <QDebug>

NetworkManager* NetworkManager::s_instance = nullptr;

NetworkManager* NetworkManager::instance()
{
    if (!s_instance) {
        s_instance = new NetworkManager();
    }
    return s_instance;
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_connectionTimeoutTimer(new QTimer(this))
    , m_isConnected(false)
{
    // Setup connection timeout timer
    m_connectionTimeoutTimer->setSingleShot(true);
    m_connectionTimeoutTimer->setInterval(CONNECTION_TIMEOUT);
    connect(m_connectionTimeoutTimer, &QTimer::timeout, this, &NetworkManager::onConnectionTimeout);

    // Connect network manager signals
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &NetworkManager::onNetworkReply);
}

void NetworkManager::sendPostRequest(const QString &url, const QByteArray &data,
                                     const QString &contentType, QObject *requester)
{
    if (url.isEmpty()) {
        qDebug() << "NetworkManager: Empty URL provided";
        if (requester) {
            emit requestFinished(requester, false, "Empty URL");
        }
        return;
    }

    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);

    QNetworkReply *reply = m_networkManager->post(request, data);

    // Track the request and its requester
    if (requester) {
        m_pendingRequests[reply] = requester;
    }

    // IMPORTANT: Connect the individual reply's finished signal
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        this->handleReplyFinished(reply);
    });

    // Start/restart connection timeout timer only if we have pending requests
    if (!m_pendingRequests.isEmpty()) {
        m_connectionTimeoutTimer->start();
    }

    qDebug() << "NetworkManager: Sending POST request to" << url << "with data:" << data;
}

void NetworkManager::handleReplyFinished(QNetworkReply* reply)
{
    qDebug() << "NetworkManager: handleReplyFinished() called - ENTRY POINT";

    if (!reply) {
        qDebug() << "NetworkManager: ERROR - No reply object in handleReplyFinished()";
        return;
    }

    qDebug() << "NetworkManager: Processing reply for" << reply->url();
    qDebug() << "NetworkManager: Reply finished:" << reply->isFinished();
    qDebug() << "NetworkManager: Reply error code:" << reply->error();
    qDebug() << "NetworkManager: Reply error string:" << reply->errorString();

    // Get HTTP status code
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "NetworkManager: HTTP status code:" << httpStatus;

    // Read all response data
    QByteArray responseData = reply->readAll();
    qDebug() << "NetworkManager: Response data size:" << responseData.size();
    qDebug() << "NetworkManager: Response data:" << responseData;

    // Check if this reply is still in our pending requests
    if (!m_pendingRequests.contains(reply)) {
        qDebug() << "NetworkManager: Reply not in pending requests (likely timed out)";
        reply->deleteLater();
        return;
    }

    // Stop connection timeout timer since we got a response
    qDebug() << "NetworkManager: Stopping connection timeout timer";
    m_connectionTimeoutTimer->stop();

    // Get the requester for this reply
    QObject *requester = m_pendingRequests.take(reply);
    qDebug() << "NetworkManager: Removed request from pending list. Remaining:" << m_pendingRequests.size();

    bool success = (reply->error() == QNetworkReply::NoError);
    QString errorString;

    if (!success) {
        errorString = reply->errorString();
        qDebug() << "NetworkManager: Request FAILED:" << errorString;
        updateConnectionStatus(false);
    } else {
        qDebug() << "NetworkManager: Request SUCCESS";
        updateConnectionStatus(true);
    }

    // Notify the requester if specified
    if (requester) {
        qDebug() << "NetworkManager: Emitting requestFinished for requester - success:" << success;
        emit requestFinished(requester, success, errorString);
    }

    // Restart timeout timer if there are still pending requests
    if (!m_pendingRequests.isEmpty()) {
        qDebug() << "NetworkManager: Restarting timeout timer for remaining" << m_pendingRequests.size() << "requests";
        m_connectionTimeoutTimer->start();
    }

    reply->deleteLater();
    qDebug() << "NetworkManager: handleReplyFinished() completed";
}

// In NetworkManager.cpp, modify the onNetworkReply method:

void NetworkManager::onNetworkReply()
{
    qDebug() << "NetworkManager: onNetworkReply() called - ENTRY POINT";
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qDebug() << "NetworkManager: ERROR - No reply object in onNetworkReply()";
        return;
    }

    qDebug() << "NetworkManager: Processing reply for" << reply->url();
    qDebug() << "NetworkManager: Reply finished:" << reply->isFinished();
    qDebug() << "NetworkManager: Reply running:" << reply->isRunning();
    qDebug() << "NetworkManager: Reply error code:" << reply->error();
    qDebug() << "NetworkManager: Reply error string:" << reply->errorString();

    // Get HTTP status code
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "NetworkManager: HTTP status code:" << httpStatus;

    // Read all response data
    QByteArray responseData = reply->readAll();
    qDebug() << "NetworkManager: Response data size:" << responseData.size();
    qDebug() << "NetworkManager: Response data:" << responseData;

    // Check if this reply is still in our pending requests
    if (!m_pendingRequests.contains(reply)) {
        qDebug() << "NetworkManager: Reply not in pending requests (likely timed out)";
        reply->deleteLater();
        return;
    }

    // Stop connection timeout timer since we got a response
    qDebug() << "NetworkManager: Stopping connection timeout timer";
    m_connectionTimeoutTimer->stop();

    // Get the requester for this reply
    QObject *requester = m_pendingRequests.take(reply);
    qDebug() << "NetworkManager: Removed request from pending list. Remaining:" << m_pendingRequests.size();

    bool success = (reply->error() == QNetworkReply::NoError);
    QString errorString;

    if (!success) {
        errorString = reply->errorString();
        qDebug() << "NetworkManager: Request FAILED:" << errorString;
        updateConnectionStatus(false);
    } else {
        qDebug() << "NetworkManager: Request SUCCESS";
        qDebug() << "NetworkManager: Response content type:"
                 << reply->header(QNetworkRequest::ContentTypeHeader).toString();
        updateConnectionStatus(true);
    }

    // Notify the requester if specified
    if (requester) {
        qDebug() << "NetworkManager: Emitting requestFinished for requester - success:" << success;
        emit requestFinished(requester, success, errorString);
    } else {
        qDebug() << "NetworkManager: No requester to notify";
    }

    // Restart timeout timer if there are still pending requests
    if (!m_pendingRequests.isEmpty()) {
        qDebug() << "NetworkManager: Restarting timeout timer for remaining" << m_pendingRequests.size() << "requests";
        m_connectionTimeoutTimer->start();
    } else {
        qDebug() << "NetworkManager: No more pending requests";
    }

    reply->deleteLater();
    qDebug() << "NetworkManager: onNetworkReply() completed";
}

void NetworkManager::onConnectionTimeout()
{
    qDebug() << "NetworkManager: Connection timeout - clearing" << m_pendingRequests.size() << "pending requests";
    updateConnectionStatus(false);

    // Notify all requesters about the timeout and clear pending requests
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
        QObject *requester = it.value();
        if (requester) {
            emit requestFinished(requester, false, "Connection timeout");
        }
    }

    // Clear all pending requests - this is crucial!
    m_pendingRequests.clear();

    // Don't restart the timer since we have no pending requests
}

void NetworkManager::updateConnectionStatus(bool connected)
{
    if (m_isConnected != connected) {
        m_isConnected = connected;
        emit connectionStatusChanged();
        qDebug() << "NetworkManager: Connection status changed to" << (connected ? "connected" : "disconnected");
    }
}
