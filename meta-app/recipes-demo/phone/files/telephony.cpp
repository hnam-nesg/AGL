#include "telephony.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

Telephony::Telephony(QObject *parent)
    : QObject(parent)
{
    m_voiceCallManagerPath = "/hfp/org/bluez/hci0/dev_88_A4_79_65_91_34";

    bool ok1 = QDBusConnection::systemBus().connect(
        "org.ofono",
        m_voiceCallManagerPath,
        "org.ofono.VoiceCallManager",
        "CallAdded",
        this,
        SLOT(onCallAdded(QDBusObjectPath,QVariantMap)));

    bool ok2 = QDBusConnection::systemBus().connect(
        "org.ofono",
        m_voiceCallManagerPath,
        "org.ofono.VoiceCallManager",
        "CallRemoved",
        this,
        SLOT(onCallRemoved(QDBusObjectPath)));

    qDebug() << "Telephony DBus connect CallAdded =" << ok1;
    qDebug() << "Telephony DBus connect CallRemoved =" << ok2;

    setConnected(true);
    setOnlineState(true);
}

Telephony::~Telephony()
{
}

void Telephony::setConnected(bool state)
{
    if (m_connected == state)
        return;

    m_connected = state;
    emit connectedChanged(state);
}

void Telephony::setCallState(const QString &callState)
{
    if (m_call_state == callState)
        return;

    m_call_state = callState;
    emit callStateChanged(m_call_state);
}

void Telephony::setOnlineState(bool state)
{
    if (m_online == state)
        return;

    m_online = state;
    emit onlineChanged(state);
}

void Telephony::setCallClip(const QString &clip)
{
    if (m_clip == clip)
        return;

    m_clip = clip;
    emit callClipChanged(m_clip);
}

void Telephony::setCallColp(const QString &colp)
{
    if (m_colp == colp)
        return;

    m_colp = colp;
    emit callColpChanged(m_colp);
}

void Telephony::onCallAdded(const QDBusObjectPath &path, const QVariantMap &properties)
{
    qDebug() << "onCallAdded:" << path.path() << properties;

    m_currentCallPath = path.path();

    const QString state = properties.value("State").toString();
    const QString lineId = properties.value("LineIdentification").toString();
    const QString name = properties.value("Name").toString();

    if (!lineId.isEmpty())
        setCallClip(lineId);
    else
        setCallClip(name);

    if (state == "incoming") {
        setCallState("incoming");
    } else if (state == "dialing") {
        setCallColp(lineId);
        setCallState("dialing");
    } else if (state == "active") {
        setCallState("active");
    } else {
        setCallState(state);
    }
}

void Telephony::onCallRemoved(const QDBusObjectPath &path)
{
    qDebug() << "onCallRemoved:" << path.path();

    if (path.path() == m_currentCallPath || m_currentCallPath.isEmpty()) {
        setCallState("disconnected");
        setCallClip("");
        setCallColp("");
        m_currentCallPath.clear();
    }
}

void Telephony::dial(QString number)
{
    qDebug() << "dial:" << number;

    QDBusInterface iface(
        "org.ofono",
        m_voiceCallManagerPath,
        "org.ofono.VoiceCallManager",
        QDBusConnection::systemBus());

    if (!iface.isValid()) {
        qWarning() << "VoiceCallManager interface invalid";
        return;
    }

    QDBusReply<QDBusObjectPath> reply = iface.call("Dial", number, "default");
    if (!reply.isValid()) {
        qWarning() << "Dial failed:" << reply.error().message();
        setCallState("disconnected");
        setCallColp("");
        return;
    }

    setCallColp(number);
    setCallState("dialing");
}

void Telephony::answer()
{
    qDebug() << "answer currentCallPath =" << m_currentCallPath;

    if (m_currentCallPath.isEmpty()) {
        qWarning() << "No current call path to answer";
        return;
    }

    QDBusInterface iface(
        "org.ofono",
        m_currentCallPath,
        "org.ofono.VoiceCall",
        QDBusConnection::systemBus());

    if (!iface.isValid()) {
        qWarning() << "VoiceCall interface invalid for answer";
        return;
    }

    QDBusReply<void> reply = iface.call("Answer");
    if (!reply.isValid()) {
        qWarning() << "Answer failed:" << reply.error().message();
    }
}

void Telephony::hangup()
{
    qDebug() << "hangup currentCallPath =" << m_currentCallPath;

    if (m_currentCallPath.isEmpty()) {
        qWarning() << "No current call path to hangup";
        return;
    }

    QDBusInterface iface(
        "org.ofono",
        m_currentCallPath,
        "org.ofono.VoiceCall",
        QDBusConnection::systemBus());

    if (!iface.isValid()) {
        qWarning() << "VoiceCall interface invalid for hangup";
        return;
    }

    QDBusReply<void> reply = iface.call("Hangup");
    if (!reply.isValid()) {
        qWarning() << "Hangup failed:" << reply.error().message();
    }
}