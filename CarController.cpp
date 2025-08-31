#include "CarController.h"
#include <QDebug>
#include <QtMath>

CarController::CarController(QObject *parent)
    : QObject(parent)
    , m_networkManager(NetworkManager::instance())
    , m_steeringCenterTimer(new QTimer(this))
    , m_sendDebounceTimer(new QTimer(this))
    , m_serialPort(new QSerialPort(this))
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
    , m_ignoreHardwareInput(false)  // Initialize hardware ignore flag
    , m_emergencyStopActive(false)  // Initialize emergency stop flag
    , m_hardwareControlActive(false)  // Initialize hardware control flag
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

    initSerialPort();
}

// bool CarController::isConnected() const
// {
//     return m_networkManager->isConnected();
// }

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

        // Only start auto-center timer if not currently pressed AND no hardware input and value is not 0
        if (!m_steeringPressed && !m_hardwareControlActive && turn != 0) {
            m_steeringCenterTimer->stop();
            m_steeringCenterTimer->start();
        } else if (turn == 0 || m_hardwareControlActive) {
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

    QString command = QString("%1 %2").arg(leftSpeed).arg(rightSpeed);
    QString formData = QString("plain=%1").arg(command);

    // Only send if there's a change from the last command
    if (command != m_lastCommand) {
        m_lastCommand = command;

        m_networkManager->sendPostRequest(m_serverUrl, formData.toUtf8(),
                                          "application/x-www-form-urlencoded", this);

        emit commandSent(command);
        qDebug() << "CarController: Sending command:" << command << "as form data:" << formData;
    }
}

void CarController::stopCar()
{
    int leftSpeed = 0;
    int rightSpeed = 0;
    int speedValue = 0;

    // Set emergency stop active flag
    m_emergencyStopActive = true;
    m_ignoreHardwareInput = true;

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
            qDebug() << "CarController: Emergency stop activated - ignoring hardware input until controls return to dead zone";
        }
    }
}

void CarController::centerSteering()
{
    setTurnValue(0);
}

void CarController::onSteeringCenterTimer()
{
    // Auto-center the steering when timer expires, but only if not currently pressed and no hardware control
    if (!m_steeringPressed && !m_hardwareControlActive) {
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

void CarController::initSerialPort()
{
    // Find the correct serial port for the Arduino
    const auto ports = QSerialPortInfo::availablePorts();
    QString arduinoPortName;

    for (const QSerialPortInfo &port : ports) {
        qDebug() << "Checking port:" << port.portName()
        << "Manufacturer:" << port.manufacturer()
        << "VID:" << QString::number(port.vendorIdentifier(), 16)
        << "PID:" << QString::number(port.productIdentifier(), 16);

        // Genuine Arduino Uno (ATmega16U2 USB chip)
        bool isGenuineArduino = (port.vendorIdentifier() == 0x2341 && port.productIdentifier() == 0x0043);

        // CH340/CH341 clone (wch.cn)
        bool isCH340Clone = (port.vendorIdentifier() == 0x1A86 && port.productIdentifier() == 0x7523);

        if (isGenuineArduino || isCH340Clone) {
            arduinoPortName = port.portName();
            break;
        }
    }

    if (arduinoPortName.isEmpty()) {
        qDebug() << "CarController: Could not find Arduino port. Hardware controls will not work.";
        return;
    }

    m_serialPort->setPortName(arduinoPortName);
    m_serialPort->setBaudRate(QSerialPort::Baud115200); // Must match the Arduino's baud rate!
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadOnly)) {
        qDebug() << "CarController: Successfully connected to Arduino on port" << arduinoPortName;
        // Connect the 'readyRead' signal to our handler slot
        connect(m_serialPort, &QSerialPort::readyRead, this, &CarController::readSerialData);
    } else {
        qDebug() << "CarController: Failed to open serial port" << arduinoPortName
                 << "Error:" << m_serialPort->errorString();
    }
}

void CarController::readSerialData()
{
    // Check if there is data to be read
    while (m_serialPort->canReadLine()) {
        QByteArray data = m_serialPort->readLine();
        QString dataString = QString::fromUtf8(data).trimmed();

        qDebug() << "CarController: Received raw data:" << dataString;

        // Parse the comma-separated values: "speed,turn"
        QStringList values = dataString.split(',');

        if (values.size() == 2) {
            bool speedOk, turnOk;
            int speed = values[0].toInt(&speedOk);
            int turn = values[1].toInt(&turnOk);

            if (speedOk && turnOk &&
                speed >= -255 && speed <= 255 &&
                turn >= -50 && turn <= 50) {

                qDebug() << "CarController: Received valid values - Speed:" << speed << "Turn:" << turn;

                // Check if we should ignore hardware input
                if (m_ignoreHardwareInput) {
                    // Check if both controls have returned to dead zone
                    bool speedInDeadZone = qAbs(speed) <= m_speedDeadZone;
                    bool turnInDeadZone = qAbs(turn) <= m_turnDeadZone;

                    if (speedInDeadZone && turnInDeadZone) {
                        qDebug() << "CarController: Both controls returned to dead zone, re-enabling hardware input";
                        m_ignoreHardwareInput = false;
                        m_emergencyStopActive = false;

                        // Process these values since they're in the dead zone
                        if (m_speedValue != speed) {
                            setSpeedValue(speed);
                        }
                        if (m_turnValue != turn) {
                            setTurnValue(turn);
                        }
                    } else {
                        qDebug() << "CarController: Ignoring hardware input - waiting for both controls to return to dead zone"
                                 << "Speed in dead zone:" << speedInDeadZone
                                 << "Turn in dead zone:" << turnInDeadZone;
                    }
                } else {
                    // Normal operation - process hardware input
                    // Only update if the user is not currently interacting with the UI controls
                    bool hardwareInputReceived = false;

                    if (!m_speedPressed && m_speedValue != speed) {
                        setSpeedValue(speed);
                        hardwareInputReceived = true;
                    }
                    if (!m_steeringPressed && m_turnValue != turn) {
                        setTurnValue(turn);
                        hardwareInputReceived = true;
                    }

                    // Update hardware control status - active if any non-zero values from hardware
                    bool newHardwareControlActive = (qAbs(speed) > m_speedDeadZone || qAbs(turn) > m_turnDeadZone);
                    if (m_hardwareControlActive != newHardwareControlActive) {
                        m_hardwareControlActive = newHardwareControlActive;
                        qDebug() << "CarController: Hardware control active:" << m_hardwareControlActive;

                        // If hardware control becomes inactive, restart auto-center timer if needed
                        if (!m_hardwareControlActive && !m_steeringPressed && m_turnValue != 0) {
                            m_steeringCenterTimer->start();
                        }
                    }
                }
            } else {
                qDebug() << "CarController: Invalid range - Speed:" << speed << "(valid: -255 to 255)"
                         << "Turn:" << turn << "(valid: -50 to 50)";
            }
        } else {
            qDebug() << "CarController: Invalid data format. Expected 'speed,turn', got:" << dataString;
        }
    }
}
