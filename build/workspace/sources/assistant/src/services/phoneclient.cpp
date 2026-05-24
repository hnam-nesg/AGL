#include "phoneclient.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

PhoneClient::PhoneClient(QObject *parent)
    : QObject(parent)
{
}

bool PhoneClient::callBoolMethod(const QString &method)
{
    QDBusInterface iface(
        m_service,
        m_path,
        m_interface,
        QDBusConnection::sessionBus());

    if (!iface.isValid()) {
        qWarning() << "[PhoneClient] invalid D-Bus interface:"
                   << iface.lastError().message();
        return false;
    }

    QDBusReply<bool> reply = iface.call(method);

    if (!reply.isValid()) {
        qWarning() << "[PhoneClient]" << method
                   << "failed:" << reply.error().message();
        return false;
    }

    qDebug() << "[PhoneClient]" << method << "result =" << reply.value();
    return reply.value();
}

bool PhoneClient::callBoolMethodString(const QString &method, const QString &arg)
{
    QDBusInterface iface(
        m_service,
        m_path,
        m_interface,
        QDBusConnection::sessionBus());

    if (!iface.isValid()) {
        qWarning() << "[PhoneClient] invalid D-Bus interface:"
                   << iface.lastError().message();
        return false;
    }

    QDBusReply<bool> reply = iface.call(method, arg);

    if (!reply.isValid()) {
        qWarning() << "[PhoneClient]" << method
                   << "arg =" << arg
                   << "failed:" << reply.error().message();
        return false;
    }

    qDebug() << "[PhoneClient]" << method
             << "arg =" << arg
             << "result =" << reply.value();

    return reply.value();
}

bool PhoneClient::syncContacts()
{
    return callBoolMethod(QStringLiteral("SyncContacts"));
}

bool PhoneClient::hasContact(const QString &name)
{
    return callBoolMethodString(QStringLiteral("HasContact"), name.trimmed());
}

bool PhoneClient::callContact(const QString &name)
{
    return callBoolMethodString(QStringLiteral("CallContact"), name.trimmed());
}

bool PhoneClient::callNumber(const QString &number)
{
    return callBoolMethodString(QStringLiteral("CallNumber"), number.trimmed());
}

bool PhoneClient::hangup()
{
    return callBoolMethod(QStringLiteral("Hangup"));
}

bool PhoneClient::answer()
{
    return callBoolMethod(QStringLiteral("Answer"));
}