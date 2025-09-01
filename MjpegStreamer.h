// mjpegstreamer.h
#ifndef MJPEGSTREAMER_H
#define MJPEGSTREAMER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QTimer>
#include <QMutex>
#include <QQuickImageProvider>
#include <QQmlEngine>

// Forward declaration
class StreamImageProvider;

class MjpegStreamer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(int frameRate READ frameRate NOTIFY frameRateChanged)

public:
    explicit MjpegStreamer(QObject *parent = nullptr);
    ~MjpegStreamer();

    QString url() const { return m_url; }
    void setUrl(const QString &url);

    bool connected() const { return m_connected; }
    QString status() const { return m_status; }
    int frameRate() const { return m_frameRate; }

    Q_INVOKABLE void startStream();
    Q_INVOKABLE void stopStream();
    Q_INVOKABLE void reconnect();

    // Static function to get the global image provider
    static StreamImageProvider* getImageProvider();

signals:
    void urlChanged();
    void connectedChanged();
    void statusChanged();
    void frameRateChanged();
    void newFrame(const QPixmap &pixmap);
    void imageChanged(); // Signal for QML Image to update

private slots:
    void handleNetworkData();
    void handleNetworkError(QNetworkReply::NetworkError error);
    void handleNetworkFinished();
    void updateFrameRate();

private:
    void setConnected(bool connected);
    void setStatus(const QString &status);
    void processBuffer();
    QPixmap extractFrame(const QByteArray &jpegData);

    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_reply;
    QByteArray m_buffer;
    QString m_url;
    bool m_connected;
    QString m_status;

    // Frame rate calculation
    QTimer *m_frameRateTimer;
    int m_frameCount;
    int m_frameRate;

    // MJPEG boundary detection
    QByteArray m_boundary;
    bool m_boundaryFound;

    // Latest frame storage
    QPixmap m_latestFrame;
};

// Custom image provider for QML
class StreamImageProvider : public QQuickImageProvider
{
public:
    StreamImageProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
    void updatePixmap(const QPixmap &pixmap);

private:
    QPixmap m_currentPixmap;
    QMutex m_mutex;
};

#endif // MJPEGSTREAMER_H
