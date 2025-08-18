#include "CarController.h"
#include <QDebug>
#include <QtMath>

CarController::CarController(QObject *parent)
    : QObject(parent)
    , m_networkManager(NetworkManager::instance())
    , m_steeringCenterTimer(new QTimer(this))
    , m_sendDebounceTimer(new QTimer(this))
    , m_speedValue(0)
    , m_turnValue(0)
    , m_processedSpeed(0)
    , m_processedTurn(0)
    , m_serverUrl("http://192.168.4.1/setSpeed")
    , m_speedDeadZone(10)  // Dead zone for speed slider
    , m_turnDeadZone(5)    // Dead zone for turn slider
    , m_steeringPressed(false)
    , m_speedPressed(false)
    , m_leftMotorSpeed(0)
    , m_rightMotorSpeed(0)
{
    // Connect to network manager signals
    connect(m_networkManager, &NetworkManager::requestFinished,
            this, &CarController::onNetworkRequestFinished);
    connect(m_networkManager, &NetworkManager::connectionStatusChanged,
            this, &CarController::onNetworkConnectionChanged);

    // Setup steering auto-center timer
    m_steeringCenterTimer->setSingleShot(true);
    m_steeringCenterTimer->setInterval(STEERING_CENTER_TIMEOUT);
    connect(m_steeringCenterTimer, &QTimer::timeout, this, &CarController::onSteeringCenterTimer);

    // Setup debounce timer for sending requests
    m_sendDebounceTimer->setSingleShot(true);
    m_sendDebounceTimer->setInterval(80); // ~80ms debounce
    connect(m_sendDebounceTimer, &QTimer::timeout, this, &CarController::sendControlCommand);
}

bool CarController::isConnected() const
{
    return m_networkManager->isConnected();
}

void CarController::setSpeedValue(int speed)
{
    if (m_speedValue != speed) {
        m_speedValue = speed;
        emit speedValueChanged();

        applyDeadZones();
        updateMotorSpeeds();

        m_sendDebounceTimer->start();
    }
}

void CarController::setTurnValue(int turn)
{
    if (m_turnValue != turn) {
        m_turnValue = turn;
        emit turnValueChanged();

        applyDeadZones();
        updateMotorSpeeds();

        m_sendDebounceTimer->start();

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
        int speedAdjustment = static_cast<int>(qAbs(baseSpeed) * 0.8 * qAbs(turnFactor));

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

        m_networkManager->sendPostRequest(m_serverUrl, command.toUtf8(), "text/plain", this);

        emit commandSent(command);
        qDebug() << "CarController: Sending command:" << command;
    }
}

void CarController::stopCar()
{
    int leftSpeed = 0;
    int rightSpeed = 0;
    int speedValue = 0;

    if(m_speedValue != speedValue){
        m_speedValue = 0;
        emit speedValueChanged();

        if (m_leftMotorSpeed != leftSpeed || m_rightMotorSpeed != rightSpeed) {
            m_leftMotorSpeed = leftSpeed;
            m_rightMotorSpeed = rightSpeed;
            emit motorSpeedsChanged();
        }

        QString command = QString("%1 %2").arg(0).arg(0);

        if (command != m_lastCommand) {
            m_lastCommand = command;

            m_networkManager->sendPostRequest(m_serverUrl, command.toUtf8(), "text/plain", this);

            emit commandSent(command);
            qDebug() << "CarController: Sending command:" << command;
        }
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

void CarController::onNetworkRequestFinished(QObject *requester, bool success, const QString &errorString)
{
    // Only handle our own requests
    if (requester != this) {
        return;
    }

    if (!success) {
        emit networkError(errorString);
        qDebug() << "CarController: Network error:" << errorString;
    }
}

void CarController::onNetworkConnectionChanged()
{
    emit connectionStatusChanged();
}
