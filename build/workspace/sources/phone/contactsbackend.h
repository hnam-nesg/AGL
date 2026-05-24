#ifndef CONTACTSBACKEND_H
#define CONTACTSBACKEND_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QDBusObjectPath>

class Telephony;

struct ContactEntry
{
    QString name;
    QString number;
};

class ContactsBackend : public QObject
{
    Q_OBJECT

public:
    explicit ContactsBackend(Telephony *telephony, QObject *parent = nullptr);

    /*
     * Load từ file có sẵn.
     */
    Q_INVOKABLE bool loadFromVCard(QString path);

    /*
     * Tên cũ giữ lại để D-Bus ReloadContacts gọi được.
     * Bây giờ reloadDefaultPhonebook() sẽ tự sync PBAP từ điện thoại đang kết nối.
     */
    Q_INVOKABLE bool reloadDefaultPhonebook();

    /*
     * Method mới: tự tìm điện thoại đang connect, pull PBAP, rồi load danh bạ.
     */
    Q_INVOKABLE bool syncFromConnectedPhone();

    Q_INVOKABLE bool hasContact(QString name) const;
    Q_INVOKABLE QString findNumberByName(QString name) const;
    Q_INVOKABLE bool callByName(QString name);

    Q_INVOKABLE int count() const;
    Q_INVOKABLE QStringList allContactNames() const;
    Q_INVOKABLE QString currentPhonebookPath() const { return m_currentPhonebookPath; }
    Q_INVOKABLE QString currentPhoneAddress() const { return m_currentPhoneAddress; }

signals:
    void contactsLoaded(int count);
    void phonebookSynced(QString address, QString path);
    void callStarted(QString name, QString number);
    void contactNotFound(QString name);
    void ambiguousContact(QString name, QStringList matches);
    void errorOccurred(QString message);

private:
    QString normalizeName(QString text) const;
    QString normalizePhone(QString text) const;
    QString decodeVCardText(QString text) const;

    QVector<ContactEntry> findMatches(QString name) const;

    /*
     * Tự tìm phone HFP đang kết nối qua oFono.
     */
    QString currentConnectedPhoneAddressFromOfono(QString *errorMessage = nullptr) const;

    /*
     * Pull danh bạ qua BlueZ OBEX PBAP.
     */
    bool pullPhonebookPbap(const QString &address,
                           QString *outputPath,
                           QString *errorMessage) const;

    bool waitForObexTransfer(const QDBusObjectPath &transferPath,
                             int timeoutMs,
                             QString *errorMessage) const;

    QString phonebookPathForAddress(const QString &address) const;

private:
    Telephony *m_telephony = nullptr;
    QVector<ContactEntry> m_contacts;

    /*
     * File fallback cũ.
     */
    QString m_defaultPath = "/tmp/phonebook.vcf";

    /*
     * File danh bạ đang dùng hiện tại.
     */
    QString m_currentPhonebookPath;
    QString m_currentPhoneAddress;
};

#endif // CONTACTSBACKEND_H