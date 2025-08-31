#ifndef ARMCONTROLLER_H
#define ARMCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include "NetworkManager.h"

class ArmController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int baseAngle READ baseAngle WRITE setBaseAngle NOTIFY baseAngleChanged)
    Q_PROPERTY(int shoulderAngle READ shoulderAngle WRITE setShoulderAngle NOTIFY shoulderAngleChanged)
    Q_PROPERTY(int elbowAngle READ elbowAngle WRITE setElbowAngle NOTIFY elbowAngleChanged)
    Q_PROPERTY(int wristAngle READ wristAngle WRITE setWristAngle NOTIFY wristAngleChanged)
    Q_PROPERTY(int gripperAngle READ gripperAngle WRITE setGripperAngle NOTIFY gripperAngleChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)

public:
    explicit ArmController(QObject *parent = nullptr);

    // Property getters
    int baseAngle() const { return m_baseAngle; }
    int shoulderAngle() const { return m_shoulderAngle; }
    int elbowAngle() const { return m_elbowAngle; }
    int wristAngle() const { return m_wristAngle; }
    int gripperAngle() const { return m_gripperAngle; }
    QString serverUrl() const { return m_serverUrl; }

    // Property setters
    void setBaseAngle(int angle);
    void setShoulderAngle(int angle);
    void setElbowAngle(int angle);
    void setWristAngle(int angle);
    void setGripperAngle(int angle);
    void setServerUrl(const QString &url);

public slots:
    // Individual servo control
    void moveBase(int angle);
    void moveShoulder(int angle);
    void moveElbow(int angle);
    void moveWrist(int angle);
    void moveGripper(int angle);

    // Multiple servo control
    void moveMultipleServos(const QVariantMap &servos);
    void moveAllServos(int base, int shoulder, int elbow, int wrist, int gripper);

    // Utility functions
    void resetArmToDefault();
    void openGripper();
    void closeGripper();

signals:
    void baseAngleChanged();
    void shoulderAngleChanged();
    void elbowAngleChanged();
    void wristAngleChanged();
    void gripperAngleChanged();
    void serverUrlChanged();
    void commandSent(const QString &command);
    void networkError(const QString &error);

private slots:
    void onNetworkRequestFinished(QObject *requester, bool success, const QString &errorString);
    void sendPendingCommands();

private:
    bool isValidAngle(int angle) const;
    void queueServoCommand(const QString &servo, int angle);
    void sendServoCommand(const QString &command);

    NetworkManager *m_networkManager;
    QTimer *m_commandTimer;

    int m_baseAngle;
    int m_shoulderAngle;
    int m_elbowAngle;
    int m_wristAngle;
    int m_gripperAngle;
    QString m_serverUrl;

    // For batching commands
    QStringList m_pendingCommands;
    QString m_lastCommand;

    static const int COMMAND_BATCH_TIMEOUT = 100; // ms
    static const int MIN_SERVO_ANGLE = 0;
    static const int MAX_SERVO_ANGLE = 180;
};

#endif // ARMCONTROLLER_H
