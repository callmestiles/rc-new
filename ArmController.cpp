#include "ArmController.h"
#include <QDebug>

ArmController::ArmController(QObject *parent)
    : QObject(parent)
    , m_networkManager(NetworkManager::instance())
    , m_connectionTimer(new QTimer(this))
    , m_serverUrl("http://192.168.4.1/setServo")
    , m_targetX(20.0)
    , m_targetY(0.0)
    , m_targetZ(15.0)
    , m_baseAngle(90)
    , m_shoulderAngle(90)
    , m_elbowAngle(90)
    , m_wristAngle(90)
{
    // Connect to network manager signals
    connect(m_networkManager, &NetworkManager::requestFinished,
            this, &ArmController::onNetworkRequestFinished);
    connect(m_networkManager, &NetworkManager::connectionStatusChanged,
            this, &ArmController::onNetworkConnectionChanged);

    // Setup connection check timer
    // connect(m_connectionTimer, &QTimer::timeout, this, &ArmController::checkConnection);
    // m_connectionTimer->start(5000); // Check connection every 5 seconds

    // Initialize with home position
    resetToHome();
}

void ArmController::setServerUrl(const QString &url)
{
    if (m_serverUrl != url) {
        m_serverUrl = url;
        emit serverUrlChanged();
    }
}

bool ArmController::isConnected() const
{
    return m_networkManager->isConnected();
}

void ArmController::setTargetX(double x)
{
    if (qAbs(m_targetX - x) > 0.1) {
        m_targetX = x;
        emit targetXChanged();
        calculateInverseKinematics(m_targetX, m_targetY, m_targetZ);
    }
}

void ArmController::setTargetY(double y)
{
    if (qAbs(m_targetY - y) > 0.1) {
        m_targetY = y;
        emit targetYChanged();
        calculateInverseKinematics(m_targetX, m_targetY, m_targetZ);
    }
}

void ArmController::setTargetZ(double z)
{
    if (qAbs(m_targetZ - z) > 0.1) {
        m_targetZ = z;
        emit targetZChanged();
        calculateInverseKinematics(m_targetX, m_targetY, m_targetZ);
    }
}

void ArmController::moveToPosition(double x, double y, double z)
{
    m_targetX = x;
    m_targetY = y;
    m_targetZ = z;

    emit targetXChanged();
    emit targetYChanged();
    emit targetZChanged();

    calculateInverseKinematics(x, y, z);
}

void ArmController::setIndividualServo(const QString &servo, int angle)
{
    QString command = QString("%1=%2").arg(servo).arg(angle);
    sendServoCommand(command);

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
    }
}

void ArmController::resetToHome()
{
    setAngles(90, 90, 90, 90);
    m_targetX = SHOULDER_LENGTH + ELBOW_LENGTH * qCos(qDegreesToRadians(90.0));
    m_targetY = 0.0;
    m_targetZ = ELBOW_LENGTH * qSin(qDegreesToRadians(90.0));

    emit targetXChanged();
    emit targetYChanged();
    emit targetZChanged();
}

void ArmController::calculateInverseKinematics(double x, double y, double z)
{
    // Base angle (rotation around vertical axis)
    double baseAngle = qRadiansToDegrees(qAtan2(y, x));

    // Project to 2D problem in the arm plane
    double r = qSqrt(x * x + y * y);
    double reach = qSqrt(r * r + z * z);

    // Check if target is reachable
    double maxReach = SHOULDER_LENGTH + ELBOW_LENGTH;
    double minReach = qAbs(SHOULDER_LENGTH - ELBOW_LENGTH);

    if (reach > maxReach || reach < minReach) {
        qDebug() << "Target position unreachable:" << x << y << z;
        return;
    }

    // Calculate shoulder and elbow angles using law of cosines
    double cosElbow = (SHOULDER_LENGTH * SHOULDER_LENGTH + ELBOW_LENGTH * ELBOW_LENGTH - reach * reach) /
                      (2 * SHOULDER_LENGTH * ELBOW_LENGTH);

    // Clamp to valid range
    cosElbow = qBound(-1.0, cosElbow, 1.0);

    double elbowAngle = qRadiansToDegrees(qAcos(cosElbow));

    double alpha = qAtan2(z, r);
    double beta = qAcos((SHOULDER_LENGTH * SHOULDER_LENGTH + reach * reach - ELBOW_LENGTH * ELBOW_LENGTH) /
                        (2 * SHOULDER_LENGTH * reach));

    double shoulderAngle = qRadiansToDegrees(alpha + beta);

    // Calculate wrist angle to keep end effector level (adjust as needed)
    double wristAngle = 180 - shoulderAngle - elbowAngle;

    // Convert to servo angles and apply constraints
    int base = qBound(BASE_MIN, static_cast<int>(baseAngle + 90), BASE_MAX);
    int shoulder = qBound(SHOULDER_MIN, static_cast<int>(shoulderAngle), SHOULDER_MAX);
    int elbow = qBound(ELBOW_MIN, static_cast<int>(elbowAngle), ELBOW_MAX);
    int wrist = qBound(WRIST_MIN, static_cast<int>(wristAngle), WRIST_MAX);

    setAngles(base, shoulder, elbow, wrist);
}

void ArmController::setAngles(int base, int shoulder, int elbow, int wrist)
{
    if (base != m_baseAngle || shoulder != m_shoulderAngle ||
        elbow != m_elbowAngle || wrist != m_wristAngle) {

        m_baseAngle = base;
        m_shoulderAngle = shoulder;
        m_elbowAngle = elbow;
        m_wristAngle = wrist;

        emit baseAngleChanged();
        emit shoulderAngleChanged();
        emit elbowAngleChanged();
        emit wristAngleChanged();

        // Send command to servos
        QString command = QString("base=%1&shoulder=%2&elbow=%3&wrist=%4")
                              .arg(base).arg(shoulder).arg(elbow).arg(wrist);
        sendServoCommand(command);
    }
}

void ArmController::sendServoCommand(const QString &data)
{
    if (m_serverUrl.isEmpty()) {
        qDebug() << "ArmController: Server URL not set";
        return;
    }

    m_networkManager->sendPostRequest(m_serverUrl, data.toUtf8(),
                                      "application/x-www-form-urlencoded", this);
    emit commandSent(data);
}

// void ArmController::checkConnection()
// {
//     // Send a simple command to check connectivity
//     sendServoCommand("base=" + QString::number(m_baseAngle));
// }

void ArmController::onNetworkRequestFinished(QObject *requester, bool success, const QString &errorString)
{
    // Only handle our own requests
    if (requester != this) {
        return;
    }

    if (!success) {
        emit networkError(errorString);
        qDebug() << "ArmController: Network error:" << errorString;
    }
}

void ArmController::onNetworkConnectionChanged()
{
    emit isConnectedChanged();
}
