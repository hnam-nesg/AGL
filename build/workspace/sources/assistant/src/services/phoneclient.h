#ifndef PHONECLIENT_H
#define PHONECLIENT_H

#include <QObject>
#include <QString>

class PhoneClient : public QObject
{
    Q_OBJECT

public:
    explicit PhoneClient(QObject *parent = nullptr);

    bool syncContacts();
    bool hasContact(const QString &name);
    bool callContact(const QString &name);
    bool callNumber(const QString &number);
    bool hangup();
    bool answer();

private:
    bool callBoolMethod(const QString &method);
    bool callBoolMethodString(const QString &method, const QString &arg);

private:
    const QString m_service = QStringLiteral("org.agl.Phone");
    const QString m_path = QStringLiteral("/org/agl/Phone");
    const QString m_interface = QStringLiteral("org.agl.Phone");
};

#endif // PHONECLIENT_H