#include "phonedbusservice.h"
#include "telephony.h"
#include "contactsbackend.h"

#include <QDebug>

PhoneDbusService::PhoneDbusService(Telephony *telephony,
                                   ContactsBackend *contacts,
                                   QObject *parent)
    : QObject(parent)
    , m_telephony(telephony)
    , m_contacts(contacts)
{
}

bool PhoneDbusService::CallNumber(const QString &number)
{
    const QString n = number.trimmed();

    qDebug() << "[PhoneDbusService] CallNumber:" << n;

    if (!m_telephony) {
        emit Error("Telephony backend is null");
        return false;
    }

    if (n.isEmpty()) {
        emit Error("Empty phone number");
        return false;
    }

    const bool ok = m_telephony->dial(n);

    if (!ok) {
        emit Error("Dial failed: " + n);
        return false;
    }

    emit CallStarted(n);
    return true;
}

bool PhoneDbusService::CallContact(const QString &name)
{
    const QString contactName = name.trimmed();

    qDebug() << "[PhoneDbusService] CallContact:" << contactName;

    if (!m_contacts) {
        emit Error("Contacts backend is null");
        return false;
    }

    if (contactName.isEmpty()) {
        emit Error("Empty contact name");
        return false;
    }

    const bool ok = m_contacts->callByName(contactName);

    if (!ok) {
        emit Error("Cannot call contact: " + contactName);
        return false;
    }

    emit CallStarted(contactName);
    return true;
}

bool PhoneDbusService::HasContact(const QString &name)
{
    const QString contactName = name.trimmed();

    qDebug() << "[PhoneDbusService] HasContact:" << contactName;

    if (!m_contacts) {
        emit Error("Contacts backend is null");
        return false;
    }

    if (contactName.isEmpty()) {
        emit Error("Empty contact name");
        return false;
    }

    const bool found = m_contacts->hasContact(contactName);

    qDebug() << "[PhoneDbusService] HasContact result:"
             << contactName
             << found;

    return found;
}

bool PhoneDbusService::ReloadContacts()
{
    qDebug() << "[PhoneDbusService] ReloadContacts";

    if (!m_contacts) {
        emit Error("Contacts backend is null");
        return false;
    }

    const bool ok = m_contacts->syncFromConnectedPhone();

    if (!ok) {
        emit Error("ReloadContacts failed");
        return false;
    }

    return true;
}

bool PhoneDbusService::SyncContacts()
{
    qDebug() << "[PhoneDbusService] SyncContacts";

    if (!m_contacts) {
        emit Error("Contacts backend is null");
        return false;
    }

    const bool ok = m_contacts->syncFromConnectedPhone();

    if (!ok) {
        emit Error("SyncContacts failed");
        return false;
    }

    return true;
}

bool PhoneDbusService::Hangup()
{
    qDebug() << "[PhoneDbusService] Hangup";

    if (!m_telephony) {
        emit Error("Telephony backend is null");
        return false;
    }

    m_telephony->hangup();
    emit CallEnded();

    return true;
}

bool PhoneDbusService::Answer()
{
    qDebug() << "[PhoneDbusService] Answer";

    if (!m_telephony) {
        emit Error("Telephony backend is null");
        return false;
    }

    m_telephony->answer();
    return true;
}