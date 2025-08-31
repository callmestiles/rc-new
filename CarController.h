#ifndef CARCONTROLLER_H
#define CARCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "NetworkManager.h"

class CarController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int speedValue READ speedValue WRITE setSpeedValue NOTIFY speedValueChanged)
    Q_PROPERTY(int turnValue READ turnValue WRITE setTurnValue NOTIFY turnValueChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(int speedDeadZone READ speedDeadZone WRITE setSpeedDeadZone NOTIFY speedDeadZoneChanged)
    Q_PROPERTY(int turnDeadZone READ turnDeadZone WRITE setTurnDeadZone NOTIFY turnDeadZoneChanged)
    // Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(int leftMotorSpeed READ leftMotorSpeed NOTIFY motorSpeedsChanged)
    Q_PROPERTY(int rightMotorSpeed READ rightMotorSpeed NOTIFY motorSpeedsChanged)

public:
    explicit CarController(QObject *parent = nullptr);

    // Property getters
    int speedValue() const { return m_speedValue; }
    int turnValue() const { return m_turnValue; }
    QString serverUrl() const { return m_serverUrl; }
    int speedDeadZone() const { return m_speedDeadZone; }
    int turnDeadZone() const { return m_turnDeadZone; }
    // bool isConnected() const;
    int leftMotorSpeed() const { return m_leftMotorSpeed; }
    int rightMotorSpeed() const { return m_rightMotorSpeed; }

    // Property setters
    void setSpeedValue(int speed);
    void setTurnValue(int turn);
    void setServerUrl(const QString &url);
    void setSpeedDeadZone(int deadZone);
    void setTurnDeadZone(int deadZone);

public slots:
    void sendControlCommand();
    void stopCar();
    void centerSteering();
    void setSteeringPressed(bool pressed);
    void setSpeedPressed(bool pressed);
    void initSerialPort();

signals:
    void speedValueChanged();
    void turnValueChanged();
    void serverUrlChanged();
    void speedDeadZoneChanged();
    void turnDeadZoneChanged();
    void connectionStatusChanged();
    void motorSpeedsChanged();
    void commandSent(const QString &command);
    void networkError(const QString &error);

private slots:
    void onSteeringCenterTimer();
    void onNetworkRequestFinished(QObject *requester, bool success, const QString &errorString);
    void onNetworkConnectionChanged();
    void readSerialData();

private:
    void calculateMotorSpeeds(int &leftSpeed, int &rightSpeed);
    void applyDeadZones();
    void updateMotorSpeeds();

    NetworkManager *m_networkManager;
    QTimer *m_steeringCenterTimer;
    QTimer *m_sendDebounceTimer;

    int m_speedValue;
    int m_turnValue;
    int m_processedSpeed;
    int m_processedTurn;
    QString m_serverUrl;
    int m_speedDeadZone;
    int m_turnDeadZone;

    bool m_steeringPressed;
    bool m_speedPressed;
    int m_leftMotorSpeed;
    int m_rightMotorSpeed;
    QString m_lastCommand;

    QSerialPort *m_serialPort;

    bool m_ignoreHardwareInput;
    bool m_emergencyStopActive;
    bool m_hardwareControlActive;

    static const int STEERING_CENTER_TIMEOUT = 100; // ms
};

#endif // CARCONTROLLER_H
