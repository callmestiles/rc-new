#ifndef ARMCONTROLLER_H
#define ARMCONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QtMath>

class ArmController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(double targetX READ targetX WRITE setTargetX NOTIFY targetXChanged)
    Q_PROPERTY(double targetY READ targetY WRITE setTargetY NOTIFY targetYChanged)
    Q_PROPERTY(double targetZ READ targetZ WRITE setTargetZ NOTIFY targetZChanged)
    Q_PROPERTY(int baseAngle READ baseAngle NOTIFY baseAngleChanged)
    Q_PROPERTY(int shoulderAngle READ shoulderAngle NOTIFY shoulderAngleChanged)
    Q_PROPERTY(int elbowAngle READ elbowAngle NOTIFY elbowAngleChanged)
    Q_PROPERTY(int wristAngle READ wristAngle NOTIFY wristAngleChanged)

public:
    explicit ArmController(QObject *parent = nullptr);

    QString serverUrl() const { return m_serverUrl; }
    void setServerUrl(const QString &url);

    bool isConnected() const { return m_isConnected; }

    double targetX() const { return m_targetX; }
    void setTargetX(double x);

    double targetY() const { return m_targetY; }
    void setTargetY(double y);

    double targetZ() const { return m_targetZ; }
    void setTargetZ(double z);

    int baseAngle() const { return m_baseAngle; }
    int shoulderAngle() const { return m_shoulderAngle; }
    int elbowAngle() const { return m_elbowAngle; }
    int wristAngle() const { return m_wristAngle; }

    Q_INVOKABLE void moveToPosition(double x, double y, double z);
    Q_INVOKABLE void setIndividualServo(const QString &servo, int angle);
    Q_INVOKABLE void resetToHome();

signals:
    void serverUrlChanged();
    void isConnectedChanged();
    void targetXChanged();
    void targetYChanged();
    void targetZChanged();
    void baseAngleChanged();
    void shoulderAngleChanged();
    void elbowAngleChanged();
    void wristAngleChanged();
    void commandSent(const QString &command);
    void networkError(const QString &error);

private slots:
    void onNetworkReply();
    void checkConnection();

private:
    void calculateInverseKinematics(double x, double y, double z);
    void sendServoCommand(const QString &data);
    void setAngles(int base, int shoulder, int elbow, int wrist);

    QNetworkAccessManager *m_networkManager;
    QTimer *m_connectionTimer;
    QString m_serverUrl;
    bool m_isConnected;

    // Target position
    double m_targetX;
    double m_targetY;
    double m_targetZ;

    // Current servo angles
    int m_baseAngle;
    int m_shoulderAngle;
    int m_elbowAngle;
    int m_wristAngle;

    // Arm segment lengths (in cm, adjust based on your actual arm dimensions)
    const double SHOULDER_LENGTH = 14.5;
    const double ELBOW_LENGTH = 17.5;
    const double WRIST_LENGTH = 4.0;

    // Servo limits (adjust based on your servos)
    const int BASE_MIN = 0;
    const int BASE_MAX = 360;
    const int SHOULDER_MIN = 0;
    const int SHOULDER_MAX = 180;
    const int ELBOW_MIN = 0;
    const int ELBOW_MAX = 180;
    const int WRIST_MIN = 0;
    const int WRIST_MAX = 180;
};

#endif // ARMCONTROLLER_H
