#include "CarController.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QtMath>

CarController::CarController(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_steeringCenterTimer(new QTimer(this))
    , m_commandTimer(new QTimer(this))
    , m_connectionTimeoutTimer(new QTimer(this))
    , m_speedValue(0)
    , m_turnValue(0)
    , m_processedSpeed(0)
    , m_processedTurn(0)
    , m_serverUrl("http://192.168.4.1/setSpeed")
    , m_speedDeadZone(10)  // Dead zone for speed slider
    , m_turnDeadZone(5)    // Dead zone for turn slider
    , m_isConnected(false)
    , m_steeringPressed(false)
    , m_speedPressed(false)
    , m_leftMotorSpeed(0)
    , m_rightMotorSpeed(0)
{
    // Setup steering auto-center timer
    m_steeringCenterTimer->setSingleShot(true);
    m_steeringCenterTimer->setInterval(STEERING_CENTER_TIMEOUT);
    connect(m_steeringCenterTimer, &QTimer::timeout, this, &CarController::onSteeringCenterTimer);

    // Setup command sending timer - increased interval to reduce requests
    m_commandTimer->setInterval(COMMAND_SEND_INTERVAL);
    connect(m_commandTimer, &QTimer::timeout, this, &CarController::sendControlCommand);
    m_commandTimer->start();

    // Setup connection timeout timer
    m_connectionTimeoutTimer->setSingleShot(true);
    m_connectionTimeoutTimer->setInterval(CONNECTION_TIMEOUT);
    connect(m_connectionTimeoutTimer, &QTimer::timeout, this, &CarController::onConnectionTimeout);

    // Connect network manager
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &CarController::onNetworkReply);
}

void CarController::setSpeedValue(int speed)
{
    if (m_speedValue != speed) {
        m_speedValue = speed;
        emit speedValueChanged();

        applyDeadZones();
        updateMotorSpeeds();
    }
}

void CarController::setTurnValue(int turn)
{
    if (m_turnValue != turn) {
        m_turnValue = turn;
        emit turnValueChanged();

        applyDeadZones();
        updateMotorSpeeds();

        // Only start auto-center timer if not currently pressed and value is not 0
        if (!m_steeringPressed && turn != 0) {
            m_steeringCenterTimer->stop();
            m_steeringCenterTimer->start();
        } else if (turn == 0) {
            m_steeringCenterTimer->stop();
        }
    }
}

void CarController::setSteeringPressed(bool pressed)
{
    m_steeringPressed = pressed;
    if (!pressed && m_turnValue != 0) {
        // Start auto-center timer when user releases steering
        m_steeringCenterTimer->start();
    } else if (pressed) {
        // Stop auto-center timer when user presses steering
        m_steeringCenterTimer->stop();
    }
}

void CarController::setSpeedPressed(bool pressed)
{
    m_speedPressed = pressed;
}

void CarController::setServerUrl(const QString &url)
{
    if (m_serverUrl != url) {
        m_serverUrl = url;
        emit serverUrlChanged();
    }
}

void CarController::setSpeedDeadZone(int deadZone)
{
    if (m_speedDeadZone != deadZone) {
        m_speedDeadZone = deadZone;
        emit speedDeadZoneChanged();
        applyDeadZones();
    }
}

void CarController::setTurnDeadZone(int deadZone)
{
    if (m_turnDeadZone != deadZone) {
        m_turnDeadZone = deadZone;
        emit turnDeadZoneChanged();
        applyDeadZones();
    }
}

void CarController::applyDeadZones()
{
    // Apply speed dead zone
    if (qAbs(m_speedValue) <= m_speedDeadZone) {
        m_processedSpeed = 0;
    } else {
        m_processedSpeed = m_speedValue;
    }

    // Apply turn dead zone
    if (qAbs(m_turnValue) <= m_turnDeadZone) {
        m_processedTurn = 0;
    } else {
        m_processedTurn = m_turnValue;
    }
}

void CarController::updateMotorSpeeds()
{
    int leftSpeed, rightSpeed;
    calculateMotorSpeeds(leftSpeed, rightSpeed);

    if (m_leftMotorSpeed != leftSpeed || m_rightMotorSpeed != rightSpeed) {
        m_leftMotorSpeed = leftSpeed;
        m_rightMotorSpeed = rightSpeed;
        emit motorSpeedsChanged();
    }
}

void CarController::calculateMotorSpeeds(int &leftSpeed, int &rightSpeed)
{
    // Check if we're doing a turn-on-spot (low/no speed but significant turn)
    bool isTurnOnSpot = (qAbs(m_processedSpeed) <= 10) && (qAbs(m_processedTurn) > 0);

    if (isTurnOnSpot) {
        // Turn on spot: opposite motor speeds
        double turnFactor = static_cast<double>(m_processedTurn) / 50.0; // Normalize to -1.0 to 1.0
        int turnSpeed = static_cast<int>(100 * qAbs(turnFactor)); // Base turn speed (adjust as needed)

        if (turnFactor > 0) {
            // Turning right: left forward, right backward
            leftSpeed = turnSpeed;
            rightSpeed = -turnSpeed;
        } else {
            // Turning left: left backward, right forward
            leftSpeed = -turnSpeed;
            rightSpeed = turnSpeed;
        }
    } else {
        // Normal movement with steering adjustment
        int baseSpeed = m_processedSpeed;
        double turnFactor = static_cast<double>(m_processedTurn) / 50.0;
        int speedAdjustment = static_cast<int>(qAbs(baseSpeed) * 0.3 * qAbs(turnFactor));

        if (turnFactor > 0) {
            // Turning right
            leftSpeed = baseSpeed + speedAdjustment;
            rightSpeed = baseSpeed - speedAdjustment;
        } else if (turnFactor < 0) {
            // Turning left
            leftSpeed = baseSpeed - speedAdjustment;
            rightSpeed = baseSpeed + speedAdjustment;
        } else {
            // Going straight
            leftSpeed = baseSpeed;
            rightSpeed = baseSpeed;
        }
    }

    // Clamp values to valid range (-255 to 255)
    leftSpeed = qBound(-255, leftSpeed, 255);
    rightSpeed = qBound(-255, rightSpeed, 255);
}

void CarController::sendControlCommand()
{
    int leftSpeed, rightSpeed;
    calculateMotorSpeeds(leftSpeed, rightSpeed);

    // Create the command string: "leftSpeed rightSpeed"
    QString command = QString("%1 %2").arg(leftSpeed).arg(rightSpeed);

    // Only send if there's a change from the last command
    if (command != m_lastCommand) {
        m_lastCommand = command;

        QNetworkRequest request{QUrl(m_serverUrl)};
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

        QByteArray data = command.toUtf8();
        m_networkManager->post(request, data);

        // Start connection timeout timer
        m_connectionTimeoutTimer->start();

        emit commandSent(command);
        qDebug() << "Sending command:" << command;
    }
}

void CarController::centerSteering()
{
    setTurnValue(0);
}

void CarController::onSteeringCenterTimer()
{
    // Auto-center the steering when timer expires, but only if not currently pressed
    if (!m_steeringPressed) {
        setTurnValue(0);
    }
}

void CarController::onConnectionTimeout()
{
    // Set connection status to false if no response within timeout
    if (m_isConnected) {
        m_isConnected = false;
        emit connectionStatusChanged();
    }
}

void CarController::onNetworkReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    // Stop connection timeout timer since we got a response
    m_connectionTimeoutTimer->stop();

    if (reply->error() != QNetworkReply::NoError) {
        QString errorString = QString("Network error: %1").arg(reply->errorString());
        emit networkError(errorString);
        qWarning() << errorString;

        // Set connection status to false on error
        if (m_isConnected) {
            m_isConnected = false;
            emit connectionStatusChanged();
        }
    } else {
        // Successful response - set connection status to true
        if (!m_isConnected) {
            m_isConnected = true;
            emit connectionStatusChanged();
        }
    }

    reply->deleteLater();
}
