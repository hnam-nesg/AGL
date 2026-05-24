#include "header_file/cluster.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QtGlobal>

namespace {

const QString kClusterReceiverService = QStringLiteral("org.agl.ClusterReceiver");
const QString kClusterReceiverPath = QStringLiteral("/org/agl/ClusterReceiver");
const QString kClusterReceiverInterface = QStringLiteral("org.agl.ClusterReceiver.Serial");

} // namespace

Cluster::Cluster(QObject *parent)
    : QObject(parent)
{
    connectClusterReceiver();
}

Cluster::~Cluster() = default;

bool Cluster::getLightRight() const
{
    return mLight_right;
}

bool Cluster::getLightLeft() const
{
    return mLight_left;
}

bool Cluster::getLightOn() const
{
    return mLight_on;
}

bool Cluster::getLightOnDim() const
{
    return mon_light_dim;
}

bool Cluster::getHazard() const
{
    return m_hazard;
}

bool Cluster::getFog() const
{
    return m_fog;
}

int Cluster::getSpeed() const
{
    return m_speed;
}

QString Cluster::getGear() const
{
    return m_gear;
}

void Cluster::sendData(const QByteArray &data)
{
    Q_UNUSED(data)
    qWarning() << "Cluster::sendData ignored; serial transport is owned by cluster-receiver";
}

void Cluster::setLightRight(bool mode)
{
    if (mLight_right != mode)
        mLight_right = mode;
    emit signal_light_right(mode);
}

void Cluster::setLightLeft(bool mode)
{
    if (mLight_left != mode)
        mLight_left = mode;
    emit signal_light_left(mode);
}

void Cluster::setLightOn(bool mode)
{
    if (mLight_on != mode)
        mLight_on = mode;
    emit signal_light_high(mode);
}

void Cluster::setLightOnDim(bool mode)
{
    if (mon_light_dim != mode)
        mon_light_dim = mode;
    emit signal_light_dim(mode);
}

void Cluster::setHazard(bool mode)
{
    if (m_hazard != mode)
        m_hazard = mode;
    emit signal_hazard(mode);
}

void Cluster::setFog(bool mode)
{
    if (m_fog != mode)
        m_fog = mode;
    emit signal_fog(mode);
}

void Cluster::setSpeed(int value)
{
    const int clamped = qBound(0, value, 220);
    if (m_speed != clamped)
        m_speed = clamped;
    emit speed(clamped);
}

void Cluster::setGear(QString newGear)
{
    newGear = newGear.trimmed();
    if (m_gear != newGear)
        m_gear = newGear;
    emit gear(newGear);
}

void Cluster::connectClusterReceiver()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qWarning() << "Cluster receiver session D-Bus unavailable:"
                   << bus.lastError().message()
                   << "trying system bus";
        bus = QDBusConnection::systemBus();
    }

    if (!bus.isConnected()) {
        qWarning() << "Cluster receiver D-Bus unavailable:" << bus.lastError().message();
        return;
    }

    const QString service = kClusterReceiverService;
    bool ok = true;
    ok = connectReceiverSignal(bus, service, QStringLiteral("SpeedChanged"),
                               SLOT(onReceiverSpeedChanged(int))) && ok;
    ok = connectReceiverSignal(bus, service, QStringLiteral("CoolantChanged"),
                               SLOT(onReceiverCoolantChanged(int))) && ok;
    ok = connectReceiverSignal(bus, service, QStringLiteral("FuelChanged"),
                               SLOT(onReceiverFuelChanged(int))) && ok;
    ok = connectReceiverSignal(bus, service, QStringLiteral("GearChanged"),
                               SLOT(onReceiverGearChanged(QString))) && ok;
    ok = connectReceiverSignal(bus, service, QStringLiteral("LightsChanged"),
                               SLOT(onReceiverLightsChanged(bool,bool,bool,bool,bool,bool))) && ok;
    ok = connectReceiverSignal(bus, service, QStringLiteral("SerialStateChanged"),
                               SLOT(onReceiverSerialStateChanged(bool,QString))) && ok;

    if (ok) {
        qInfo() << "Cluster receiver D-Bus subscribed:"
                << kClusterReceiverInterface
                << kClusterReceiverPath;
    } else {
        qWarning() << "Cluster receiver D-Bus subscription is incomplete";
    }
}

bool Cluster::connectReceiverSignal(QDBusConnection &bus,
                                    const QString &service,
                                    const QString &signal,
                                    const char *slot)
{
    const bool ok = bus.connect(service,
                                kClusterReceiverPath,
                                kClusterReceiverInterface,
                                signal,
                                this,
                                slot);
    if (!ok) {
        qWarning() << "Cluster receiver D-Bus signal connect failed:"
                   << signal
                   << bus.lastError().message();
    }
    return ok;
}

void Cluster::onReceiverSpeedChanged(int value)
{
    qDebug() << "Cluster receiver speed=" << value;
    setSpeed(value);
}

void Cluster::onReceiverCoolantChanged(int value)
{
    const int clamped = qBound(0, value, 100);
    qDebug() << "Cluster receiver coolant=" << clamped;
    emit coolant(clamped);
}

void Cluster::onReceiverFuelChanged(int value)
{
    const int clamped = qBound(0, value, 100);
    qDebug() << "Cluster receiver fuel=" << clamped;
    emit fuel(clamped);
}

void Cluster::onReceiverGearChanged(const QString &gear)
{
    qDebug() << "Cluster receiver gear=" << gear;
    setGear(gear);
}

void Cluster::onReceiverLightsChanged(bool right, bool left, bool high, bool low, bool fog, bool hazard)
{
    qDebug() << "Cluster receiver lights:"
             << "right=" << right
             << "left=" << left
             << "high=" << high
             << "low=" << low
             << "fog=" << fog
             << "hazard=" << hazard;

    setLightRight(right);
    setLightLeft(left);
    setLightOn(high);
    setLightOnDim(low);
    setFog(fog);
    setHazard(hazard);
}

void Cluster::onReceiverSerialStateChanged(bool connected, const QString &detail)
{
    qInfo() << "Cluster receiver serial state:"
            << (connected ? "connected" : "disconnected")
            << detail;
}

void Cluster::active_gear(QString gear)
{
    setGear(gear);
}
