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

    // Start/restart connection timeout timer
    m_connectionTimeoutTimer->start();

    qDebug() << "NetworkManager: Sending POST request to" << url << "with data:" << data;
}

void NetworkManager::onNetworkReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    // Stop connection timeout timer since we got a response
    m_connectionTimeoutTimer->stop();

    // Get the requester for this reply
    QObject *requester = m_pendingRequests.take(reply);

    bool success = (reply->error() == QNetworkReply::NoError);
    QString errorString;

    if (!success) {
        errorString = reply->errorString();
        qDebug() << "NetworkManager: Network error:" << errorString;
        updateConnectionStatus(false);
    } else {
        qDebug() << "NetworkManager: Request successful";
        updateConnectionStatus(true);
    }

    // Notify the requester if specified
    if (requester) {
        emit requestFinished(requester, success, errorString);
    }

    reply->deleteLater();
}

void NetworkManager::onConnectionTimeout()
{
    qDebug() << "NetworkManager: Connection timeout";
    updateConnectionStatus(false);

    // Clean up any pending requests that might have timed out
    // Note: The actual replies will still complete, but we mark them as timed out
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
        QObject *requester = it.value();
        if (requester) {
            emit requestFinished(requester, false, "Connection timeout");
        }
    }
}

void NetworkManager::updateConnectionStatus(bool connected)
{
    if (m_isConnected != connected) {
        m_isConnected = connected;
        emit connectionStatusChanged();
        qDebug() << "NetworkManager: Connection status changed to" << (connected ? "connected" : "disconnected");
    }
}
