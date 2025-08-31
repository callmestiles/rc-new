#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QHash>

class NetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStatusChanged)

public:
    static NetworkManager* instance();

    // Network request methods
    void sendPostRequest(const QString &url, const QByteArray &data,
                         const QString &contentType = "application/x-www-form-urlencoded",
                         QObject *requester = nullptr);

    bool isConnected() const { return m_isConnected; }

signals:
    void connectionStatusChanged();
    void requestFinished(QObject *requester, bool success, const QString &errorString = QString());

private slots:
    void onNetworkReply();
    void onConnectionTimeout();
    void handleReplyFinished(QNetworkReply* reply);

private:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager() = default;

    // Singleton - prevent copying
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    void updateConnectionStatus(bool connected);

    static NetworkManager* s_instance;
    QNetworkAccessManager *m_networkManager;
    QTimer *m_connectionTimeoutTimer;
    bool m_isConnected;

    // Track pending requests and their requesters
    QHash<QNetworkReply*, QObject*> m_pendingRequests;

    static const int CONNECTION_TIMEOUT = 5000; // ms - increased from 3000 to 5000 for better reliability
};

#endif // NETWORKMANAGER_H
