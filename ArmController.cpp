#include "ArmController.h"
#include <QDebug>

ArmController::ArmController(QObject *parent)
    : QObject(parent)
    , m_networkManager(NetworkManager::instance())
    , m_commandTimer(new QTimer(this))
    , m_baseAngle(90)        // Default to middle position
    , m_shoulderAngle(90)
    , m_elbowAngle(90)
    , m_wristAngle(90)
    , m_gripperAngle(90)
    , m_serverUrl("http://192.168.4.1/setServo")
{
    // Connect to network manager signals
    connect(m_networkManager, &NetworkManager::requestFinished,
            this, &ArmController::onNetworkRequestFinished);

    // Setup command batching timer
    m_commandTimer->setSingleShot(true);
    m_commandTimer->setInterval(COMMAND_BATCH_TIMEOUT);
    connect(m_commandTimer, &QTimer::timeout, this, &ArmController::sendPendingCommands);
}

void ArmController::setBaseAngle(int angle)
{
    if (isValidAngle(angle) && m_baseAngle != angle) {
        m_baseAngle = angle;
        emit baseAngleChanged();
        queueServoCommand("base", angle);
    }
}

void ArmController::setShoulderAngle(int angle)
{
    if (isValidAngle(angle) && m_shoulderAngle != angle) {
        m_shoulderAngle = angle;
        emit shoulderAngleChanged();
        queueServoCommand("shoulder", angle);
    }
}

void ArmController::setElbowAngle(int angle)
{
    if (isValidAngle(angle) && m_elbowAngle != angle) {
        m_elbowAngle = angle;
        emit elbowAngleChanged();
        queueServoCommand("elbow", angle);
    }
}

void ArmController::setWristAngle(int angle)
{
    if (isValidAngle(angle) && m_wristAngle != angle) {
        m_wristAngle = angle;
        emit wristAngleChanged();
        queueServoCommand("wrist", angle);
    }
}

void ArmController::setGripperAngle(int angle)
{
    if (isValidAngle(angle) && m_gripperAngle != angle) {
        m_gripperAngle = angle;
        emit gripperAngleChanged();
        queueServoCommand("gripper", angle);
    }
}

void ArmController::setServerUrl(const QString &url)
{
    if (m_serverUrl != url) {
        m_serverUrl = url;
        emit serverUrlChanged();
    }
}

void ArmController::moveBase(int angle)
{
    if (isValidAngle(angle)) {
        QString command = QString("base=%1").arg(angle);
        sendServoCommand(command);

        // Update internal state
        m_baseAngle = angle;
        emit baseAngleChanged();
    }
}

void ArmController::moveShoulder(int angle)
{
    if (isValidAngle(angle)) {
        QString command = QString("shoulder=%1").arg(angle);
        sendServoCommand(command);

        // Update internal state
        m_shoulderAngle = angle;
        emit shoulderAngleChanged();
    }
}

void ArmController::moveElbow(int angle)
{
    if (isValidAngle(angle)) {
        QString command = QString("elbow=%1").arg(angle);
        sendServoCommand(command);

        // Update internal state
        m_elbowAngle = angle;
        emit elbowAngleChanged();
    }
}

void ArmController::moveWrist(int angle)
{
    if (isValidAngle(angle)) {
        QString command = QString("wrist=%1").arg(angle);
        sendServoCommand(command);

        // Update internal state
        m_wristAngle = angle;
        emit wristAngleChanged();
    }
}

void ArmController::moveGripper(int angle)
{
    if (isValidAngle(angle)) {
        QString command = QString("gripper=%1").arg(angle);
        sendServoCommand(command);

        // Update internal state
        m_gripperAngle = angle;
        emit gripperAngleChanged();
    }
}

void ArmController::moveMultipleServos(const QVariantMap &servos)
{
    QStringList commandParts;

    for (auto it = servos.begin(); it != servos.end(); ++it) {
        QString servo = it.key().toLower();
        int angle = it.value().toInt();

        if (isValidAngle(angle)) {
            commandParts << QString("%1=%2").arg(servo).arg(angle);

            // Update internal state
            if (servo == "base") {
                m_baseAngle = angle;
                emit baseAngleChanged();
            } else if (servo == "shoulder") {
                m_shoulderAngle = angle;
                emit shoulderAngleChanged();
            } else if (servo == "elbow") {
                m_elbowAngle = angle;
                emit elbowAngleChanged();
            } else if (servo == "wrist") {
                m_wristAngle = angle;
                emit wristAngleChanged();
            } else if (servo == "gripper") {
                m_gripperAngle = angle;
                emit gripperAngleChanged();
            }
        }
    }

    if (!commandParts.isEmpty()) {
        QString command = commandParts.join("&");
        sendServoCommand(command);
    }
}

void ArmController::moveAllServos(int base, int shoulder, int elbow, int wrist, int gripper)
{
    QVariantMap servos;
    servos["base"] = base;
    servos["shoulder"] = shoulder;
    servos["elbow"] = elbow;
    servos["wrist"] = wrist;
    servos["gripper"] = gripper;

    moveMultipleServos(servos);
}

void ArmController::resetArmToDefault()
{
    moveAllServos(90, 90, 90, 90, 90);
}

void ArmController::openGripper()
{
    moveGripper(180); // Assuming 180 is open
}

void ArmController::closeGripper()
{
    moveGripper(0); // Assuming 0 is closed
}

void ArmController::onNetworkRequestFinished(QObject *requester, bool success, const QString &errorString)
{
    // Only handle our own requests
    if (requester != this) {
        return;
    }

    if (!success) {
        emit networkError(errorString);
        qDebug() << "ArmController: Network error:" << errorString;
    } else {
        qDebug() << "ArmController: Servo command sent successfully";
    }
}

void ArmController::sendPendingCommands()
{
    if (m_pendingCommands.isEmpty()) {
        return;
    }

    // Combine all pending commands into one request
    QString command = m_pendingCommands.join("&");
    m_pendingCommands.clear();

    sendServoCommand(command);
}

bool ArmController::isValidAngle(int angle) const
{
    return angle >= MIN_SERVO_ANGLE && angle <= MAX_SERVO_ANGLE;
}

void ArmController::queueServoCommand(const QString &servo, int angle)
{
    // Remove any existing command for this servo
    for (int i = m_pendingCommands.size() - 1; i >= 0; --i) {
        if (m_pendingCommands[i].startsWith(servo + "=")) {
            m_pendingCommands.removeAt(i);
        }
    }

    // Add new command
    m_pendingCommands << QString("%1=%2").arg(servo).arg(angle);

    // Start/restart timer
    m_commandTimer->start();
}

void ArmController::sendServoCommand(const QString &command)
{
    if (command.isEmpty() || command == m_lastCommand) {
        return;
    }

    m_lastCommand = command;

    qDebug() << "ArmController: Sending servo command:" << command;

    m_networkManager->sendPostRequest(m_serverUrl, command.toUtf8(),
                                      "application/x-www-form-urlencoded", this);

    emit commandSent(command);
}
