#ifndef CLUSTER_H
#define CLUSTER_H

#include <QByteArray>
#include <QObject>
#include <QString>

class QDBusConnection;

class Cluster : public QObject
{
    Q_OBJECT

public:
    explicit Cluster(QObject *parent = nullptr);
    ~Cluster() override;

    Q_PROPERTY(bool light_right READ getLightRight WRITE setLightRight NOTIFY signal_light_right)
    Q_PROPERTY(bool light_left READ getLightLeft WRITE setLightLeft NOTIFY signal_light_left)
    Q_PROPERTY(bool light_on READ getLightOn WRITE setLightOn NOTIFY signal_light_high)
    Q_PROPERTY(bool light_dim READ getLightOnDim WRITE setLightOnDim NOTIFY signal_light_dim)
    Q_PROPERTY(bool light_hazard READ getHazard WRITE setHazard NOTIFY signal_hazard)
    Q_PROPERTY(bool light_fog READ getFog WRITE setFog NOTIFY signal_fog)
    Q_PROPERTY(int speed_value READ getSpeed WRITE setSpeed NOTIFY speed)
    Q_PROPERTY(QString gear_value READ getGear WRITE setGear NOTIFY gear)

    bool getLightRight() const;
    bool getLightLeft() const;
    bool getLightOn() const;
    bool getLightOnDim() const;
    bool getHazard() const;
    bool getFog() const;
    int getSpeed() const;
    QString getGear() const;

    void sendData(const QByteArray &data);
    void setLightRight(bool mode);
    void setLightLeft(bool mode);
    void setLightOn(bool mode);
    void setLightOnDim(bool mode);
    void setHazard(bool mode);
    void setFog(bool mode);
    void setSpeed(int value);
    void setGear(QString gear);

signals:
    void signal_light_high(bool onLight);
    void signal_light_dim(bool onLight);
    void signal_light_right(bool signalRight);
    void signal_light_left(bool signalLeft);
    void signal_hazard(bool hazard);
    void signal_fog(bool fog);
    void speed(int value);
    void coolant(int value);
    void fuel(int value);
    void gear(QString gear);

private slots:
    void onReceiverSpeedChanged(int value);
    void onReceiverCoolantChanged(int value);
    void onReceiverFuelChanged(int value);
    void onReceiverGearChanged(const QString &gear);
    void onReceiverLightsChanged(bool right, bool left, bool high, bool low, bool fog, bool hazard);
    void onReceiverSerialStateChanged(bool connected, const QString &detail);
    void active_gear(QString gear);

private:
    void connectClusterReceiver();
    bool connectReceiverSignal(QDBusConnection &bus,
                               const QString &service,
                               const QString &signal,
                               const char *slot);

    bool mLight_right = false;
    bool mLight_left = false;
    bool mLight_on = false;
    bool mon_light_dim = false;
    bool m_hazard = false;
    bool m_fog = false;
    int m_speed = 0;
    QString m_gear;
};

#endif // CLUSTER_H
