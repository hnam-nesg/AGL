#ifndef PHONEDBUSSERVICE_H
#define PHONEDBUSSERVICE_H

#include <QObject>
#include <QString>

class Telephony;
class ContactsBackend;

class PhoneDbusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.agl.Phone")

public:
    explicit PhoneDbusService(Telephony *telephony,
                              ContactsBackend *contacts,
                              QObject *parent = nullptr);

public slots:
    bool CallNumber(const QString &number);
    bool CallContact(const QString &name);
    bool HasContact(const QString &name);

    /*
     * ReloadContacts và SyncContacts đều sẽ pull PBAP từ điện thoại hiện tại.
     */
    bool ReloadContacts();
    bool SyncContacts();

    bool Hangup();
    bool Answer();

signals:
    void Error(const QString &message);
    void CallStarted(const QString &target);
    void CallEnded();

private:
    Telephony *m_telephony = nullptr;
    ContactsBackend *m_contacts = nullptr;
};

#endif // PHONEDBUSSERVICE_H