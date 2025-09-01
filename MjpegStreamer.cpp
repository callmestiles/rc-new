#include "MjpegStreamer.h"
#include <QNetworkRequest>
#include <QPixmap>
#include <QBuffer>
#include <QDebug>
#include <QMutex>

// Global image provider instance
static StreamImageProvider *g_imageProvider = nullptr;

MjpegStreamer::MjpegStreamer(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_reply(nullptr)
    , m_connected(false)
    , m_status("Disconnected")
    , m_frameCount(0)
    , m_frameRate(0)
    , m_boundaryFound(false)
{
    // Initialize frame rate timer
    m_frameRateTimer = new QTimer(this);
    connect(m_frameRateTimer, &QTimer::timeout, this, &MjpegStreamer::updateFrameRate);
    m_frameRateTimer->start(1000); // Update every second
}

MjpegStreamer::~MjpegStreamer()
{
    stopStream();
}

void MjpegStreamer::setUrl(const QString &url)
{
    if (m_url != url) {
        m_url = url;
        emit urlChanged();
    }
}

void MjpegStreamer::startStream()
{
    if (m_reply) {
        stopStream();
    }

    if (m_url.isEmpty()) {
        setStatus("Error: No URL specified");
        return;
    }

    setStatus("Connecting...");

    QNetworkRequest request(m_url);
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("Connection", "close");
    request.setRawHeader("User-Agent", "Qt MJPEG Streamer");

    m_reply = m_networkManager->get(request);

    connect(m_reply, &QNetworkReply::readyRead, this, &MjpegStreamer::handleNetworkData);
    connect(m_reply, &QNetworkReply::errorOccurred, this, &MjpegStreamer::handleNetworkError);
    connect(m_reply, &QNetworkReply::finished, this, &MjpegStreamer::handleNetworkFinished);

    // Reset frame counting
    m_frameCount = 0;
    m_buffer.clear();
    m_boundaryFound = false;
    m_boundary.clear();
}

void MjpegStreamer::stopStream()
{
    if (m_reply) {
        // Disconnect all signals first to prevent callbacks during destruction
        disconnect(m_reply, nullptr, this, nullptr);

        // Abort the request immediately
        m_reply->abort();

        // Delete immediately instead of using deleteLater for faster cleanup
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    setConnected(false);
    setStatus("Disconnected");
    m_buffer.clear();
    m_boundaryFound = false;
    m_boundary.clear();
}

void MjpegStreamer::reconnect()
{
    stopStream();
    QTimer::singleShot(1000, this, &MjpegStreamer::startStream);
}

void MjpegStreamer::handleNetworkData()
{
    if (!m_reply) return;

    QByteArray data = m_reply->readAll();
    m_buffer.append(data);

    if (!m_connected) {
        setConnected(true);
        setStatus("Connected - Receiving stream");
    }

    processBuffer();
}

void MjpegStreamer::handleNetworkError(QNetworkReply::NetworkError error)
{
    QString errorString = QString("Network Error: %1").arg(m_reply ? m_reply->errorString() : "Unknown");
    setStatus(errorString);
    setConnected(false);

    // Auto-reconnect after 3 seconds
    QTimer::singleShot(3000, this, &MjpegStreamer::reconnect);
}

void MjpegStreamer::handleNetworkFinished()
{
    setConnected(false);
    setStatus("Connection finished");

    // Auto-reconnect after 2 seconds
    QTimer::singleShot(2000, this, &MjpegStreamer::reconnect);
}

void MjpegStreamer::processBuffer()
{
    // Process multiple frames if available, but limit to prevent UI blocking
    int framesProcessed = 0;
    const int maxFramesPerCall = 3;

    while (m_buffer.size() > 1000 && framesProcessed < maxFramesPerCall) {
        // Look for JPEG start marker (0xFFD8)
        int jpegStart = -1;
        for (int i = 0; i < m_buffer.size() - 1; i++) {
            if ((unsigned char)m_buffer[i] == 0xFF && (unsigned char)m_buffer[i + 1] == 0xD8) {
                jpegStart = i;
                break;
            }
        }

        if (jpegStart < 0) {
            // Remove some data to prevent buffer from growing indefinitely
            if (m_buffer.size() > 100000) {
                m_buffer.remove(0, 50000);
            }
            break;
        }

        // Look for JPEG end marker (0xFFD9) after the start
        int jpegEnd = -1;
        for (int i = jpegStart + 2; i < m_buffer.size() - 1; i++) {
            if ((unsigned char)m_buffer[i] == 0xFF && (unsigned char)m_buffer[i + 1] == 0xD9) {
                jpegEnd = i + 2; // Include the end marker
                break;
            }
        }

        if (jpegEnd < 0) {
            break; // Wait for more data
        }

        // Extract JPEG data
        QByteArray jpegData = m_buffer.mid(jpegStart, jpegEnd - jpegStart);

        // Try to create pixmap
        QPixmap pixmap;
        if (pixmap.loadFromData(jpegData, "JPEG")) {
            m_latestFrame = pixmap;
            emit newFrame(pixmap);
            m_frameCount++;

            // Update image provider
            if (g_imageProvider) {
                g_imageProvider->updatePixmap(pixmap);
            }

            framesProcessed++;
        }

        // Remove processed data
        m_buffer.remove(0, jpegEnd);
    }

    // Only emit imageChanged once per processBuffer call to reduce flicker
    if (framesProcessed > 0) {
        emit imageChanged();
    }
}

QPixmap MjpegStreamer::extractFrame(const QByteArray &jpegData)
{
    QPixmap pixmap;
    pixmap.loadFromData(jpegData, "JPEG");
    return pixmap;
}

void MjpegStreamer::setConnected(bool connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        emit connectedChanged();
    }
}

void MjpegStreamer::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void MjpegStreamer::updateFrameRate()
{
    m_frameRate = m_frameCount;
    m_frameCount = 0;
    emit frameRateChanged();
}

// Static function to get the image provider instance
StreamImageProvider* MjpegStreamer::getImageProvider()
{
    if (!g_imageProvider) {
        g_imageProvider = new StreamImageProvider();
    }
    return g_imageProvider;
}

// ============================================================================

// StreamImageProvider implementation
StreamImageProvider::StreamImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QPixmap StreamImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QMutexLocker locker(&m_mutex);

    if (m_currentPixmap.isNull()) {
        // Return a placeholder image
        QPixmap placeholder(640, 480);
        placeholder.fill(Qt::darkGray);
        return placeholder;
    }

    if (size) {
        *size = m_currentPixmap.size();
    }

    if (requestedSize.isValid()) {
        return m_currentPixmap.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return m_currentPixmap;
}

void StreamImageProvider::updatePixmap(const QPixmap &pixmap)
{
    QMutexLocker locker(&m_mutex);
    m_currentPixmap = pixmap;
}
