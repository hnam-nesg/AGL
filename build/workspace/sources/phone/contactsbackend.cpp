#include "contactsbackend.h"
#include "telephony.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QTextStream>
#include <QThread>
#include <QElapsedTimer>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusVariant>
#include <QVariantMap>
#include <QDir>
#include <QFileInfo>
#include <QElapsedTimer>

static QString bluetoothAddressFromOfonoHfpPath(const QString &path)
{
    const int devIndex = path.lastIndexOf("/dev_");

    if (devIndex < 0)
        return "";

    QString dev = path.mid(devIndex + 5);
    dev.replace("_", ":");

    static const QRegularExpression macRe(
        R"(([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2})"
    );

    const QRegularExpressionMatch match = macRe.match(dev);

    if (!match.hasMatch())
        return "";

    return match.captured(0).toUpper();
}

static QString bluezDevicePathFromAddress(QString address)
{
    address = address.trimmed().toUpper();

    static const QRegularExpression macRe(
        R"(^([0-9A-F]{2}:){5}[0-9A-F]{2}$)"
    );

    if (!macRe.match(address).hasMatch())
        return "";

    address.replace(":", "_");
    return "/org/bluez/hci0/dev_" + address;
}

static bool bluezDeviceConnected(const QString &address)
{
    const QString devicePath = bluezDevicePathFromAddress(address);

    if (devicePath.isEmpty())
        return false;

    QDBusInterface properties(
        "org.bluez",
        devicePath,
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus());

    if (!properties.isValid()) {
        qWarning() << "[ContactsBackend] BlueZ properties invalid:"
                   << devicePath
                   << properties.lastError().message();
        return false;
    }

    QDBusReply<QVariant> reply =
        properties.call("Get", "org.bluez.Device1", "Connected");

    if (!reply.isValid()) {
        qWarning() << "[ContactsBackend] BlueZ Connected Get failed:"
                   << devicePath
                   << reply.error().message();
        return false;
    }

    const bool connected = reply.value().toBool();

    qDebug() << "[ContactsBackend] BlueZ device connected:"
             << "address =" << address
             << "path =" << devicePath
             << "connected =" << connected;

    return connected;
}

ContactsBackend::ContactsBackend(Telephony *telephony, QObject *parent)
    : QObject(parent)
    , m_telephony(telephony)
{
}

QString ContactsBackend::normalizeName(QString text) const
{
    text = text.trimmed().toLower();
    text.replace(QRegularExpression("\\s+"), " ");
    return text;
}

QString ContactsBackend::normalizePhone(QString text) const
{
    text = text.trimmed();
    text.remove(QRegularExpression("[^0-9+]"));
    return text;
}

QString ContactsBackend::decodeVCardText(QString text) const
{
    /*
     * vCard từ iPhone/Android thường là UTF-8.
     * Nếu sau này gặp QUOTED-PRINTABLE thì mở rộng ở đây.
     */
    return text.trimmed();
}

QString ContactsBackend::phonebookPathForAddress(const QString &address) const
{
    QString safe = address.trimmed().toUpper();
    safe.replace(":", "_");

    QDir dir("/tmp/phonebooks");
    if (!dir.exists())
        dir.mkpath(".");

    return dir.absoluteFilePath("phonebook_" + safe + ".vcf");
}

bool ContactsBackend::reloadDefaultPhonebook()
{
    /*
     * Trước đây hàm này load /tmp/phonebook.vcf.
     * Bây giờ đổi sang sync từ điện thoại đang kết nối.
     */
    return syncFromConnectedPhone();
}

bool ContactsBackend::syncFromConnectedPhone()
{
    qDebug() << "[ContactsBackend] syncFromConnectedPhone";

    QString error;
    const QString address = currentConnectedPhoneAddressFromOfono(&error);

    if (address.isEmpty()) {
        const QString msg = "Cannot sync contacts: no connected phone found. " + error;
        qWarning() << "[ContactsBackend]" << msg;
        emit errorOccurred(msg);
        return false;
    }

    qDebug() << "[ContactsBackend] current connected phone address:" << address;

    QString phonebookPath;

    if (!pullPhonebookPbap(address, &phonebookPath, &error)) {
        const QString msg = "PBAP pull failed for " + address + ": " + error;
        qWarning() << "[ContactsBackend]" << msg;
        emit errorOccurred(msg);
        return false;
    }

    if (phonebookPath.isEmpty()) {
        const QString msg = "PBAP pull succeeded but output path is empty";
        qWarning() << "[ContactsBackend]" << msg;
        emit errorOccurred(msg);
        return false;
    }

    m_currentPhoneAddress = address;
    m_currentPhonebookPath = phonebookPath;

    /*
     * Optional: copy ra /tmp/phonebook.vcf để giữ tương thích debug/lệnh cũ.
     */
    QFile::remove(m_defaultPath);
    QFile::copy(m_currentPhonebookPath, m_defaultPath);

    const bool ok = loadFromVCard(m_currentPhonebookPath);

    if (!ok) {
        const QString msg = "PBAP pulled but loadFromVCard failed: " + m_currentPhonebookPath;
        qWarning() << "[ContactsBackend]" << msg;
        emit errorOccurred(msg);
        return false;
    }

    emit phonebookSynced(address, m_currentPhonebookPath);

    qDebug() << "[ContactsBackend] sync contacts ok:"
             << "address =" << address
             << "path =" << m_currentPhonebookPath
             << "count =" << m_contacts.size();

    return true;
}

QString ContactsBackend::currentConnectedPhoneAddressFromOfono(QString *errorMessage) const
{
    QDBusInterface manager(
        "org.ofono",
        "/",
        "org.ofono.Manager",
        QDBusConnection::systemBus());

    if (!manager.isValid()) {
        if (errorMessage)
            *errorMessage = "org.ofono.Manager invalid: " + manager.lastError().message();
        return "";
    }

    QDBusMessage reply = manager.call("GetModems");

    if (reply.type() == QDBusMessage::ErrorMessage) {
        if (errorMessage)
            *errorMessage = "GetModems failed: " + reply.errorMessage();
        return "";
    }

    const QList<QVariant> args = reply.arguments();

    if (args.isEmpty()) {
        if (errorMessage)
            *errorMessage = "GetModems returned empty";
        return "";
    }

    /*
     * GetModems returns a(oa{sv}).
     * Phải đọc QDBusArgument ở chế độ read-only.
     */
    const QDBusArgument dbusArg = qvariant_cast<QDBusArgument>(args.at(0));
    const QDBusArgument &arrayArg = dbusArg;

    QString selectedAddress;

    arrayArg.beginArray();

    while (!arrayArg.atEnd()) {
        QDBusObjectPath modemPath;
        QVariantMap properties;

        arrayArg.beginStructure();
        arrayArg >> modemPath >> properties;
        arrayArg.endStructure();

        const QString path = modemPath.path();

        qDebug() << "[ContactsBackend] oFono modem:"
                 << "path =" << path
                 << "properties =" << properties;

        if (!path.startsWith("/hfp/org/bluez/hci0/dev_"))
            continue;

        const QString address = bluetoothAddressFromOfonoHfpPath(path);

        if (address.isEmpty())
            continue;

        if (!bluezDeviceConnected(address)) {
            qDebug() << "[ContactsBackend] skip disconnected/stale phone:"
                     << "address =" << address
                     << "path =" << path;
            continue;
        }

        selectedAddress = address;
        qDebug() << "[ContactsBackend] selected connected phone from oFono:"
                 << selectedAddress
                 << "path =" << path;
        break;
    }

    arrayArg.endArray();

    if (selectedAddress.isEmpty() && errorMessage)
        *errorMessage = "No connected /hfp/org/bluez/hci0/dev_* modem found in oFono";

    return selectedAddress;
}

bool ContactsBackend::pullPhonebookPbap(const QString &address,
                                        QString *outputPath,
                                        QString *errorMessage) const
{
    const QString addr = address.trimmed().toUpper();

    if (addr.isEmpty()) {
        if (errorMessage)
            *errorMessage = "empty bluetooth address";
        return false;
    }

    const QString outPath = phonebookPathForAddress(addr);

    qDebug() << "[ContactsBackend] pullPhonebookPbap:"
             << "address =" << addr
             << "outPath =" << outPath;

    QFile::remove(outPath);

    QDBusConnection bus = QDBusConnection::sessionBus();

    if (!bus.isConnected()) {
        if (errorMessage)
            *errorMessage = "session bus is not connected";
        return false;
    }

    QDBusInterface obexClient(
        "org.bluez.obex",
        "/org/bluez/obex",
        "org.bluez.obex.Client1",
        bus);

    if (!obexClient.isValid()) {
        if (errorMessage)
            *errorMessage = "org.bluez.obex.Client1 invalid: "
                            + obexClient.lastError().message();
        return false;
    }

    QVariantMap createArgs;
    createArgs.insert("Target", QVariant("pbap"));
    //createArgs.insert("Channel", QVariant::fromValue<uchar>(13)); duma lỗi vô duyến 

    QDBusReply<QDBusObjectPath> sessionReply =
        obexClient.call("CreateSession", addr, createArgs);

    if (!sessionReply.isValid()) {
        if (errorMessage)
            *errorMessage = "CreateSession PBAP failed: "
                            + sessionReply.error().message();
        return false;
    }

    const QDBusObjectPath sessionPath = sessionReply.value();

    qDebug() << "[ContactsBackend] PBAP session created:"
             << sessionPath.path();

    bool success = false;
    QString localError;

    do {
        QDBusInterface pbap(
            "org.bluez.obex",
            sessionPath.path(),
            "org.bluez.obex.PhonebookAccess1",
            bus);

        if (!pbap.isValid()) {
            localError = "PhonebookAccess1 invalid: "
                         + pbap.lastError().message();
            break;
        }

        QDBusReply<void> selectReply =
            pbap.call("Select", QString("int"), QString("pb"));

        if (!selectReply.isValid()) {
            localError = "PBAP Select(int,pb) failed: "
                         + selectReply.error().message();
            break;
        }

        QVariantMap pullArgs;
        pullArgs.insert("Format", QVariant("vcard30"));

        /*
         * PullAll returns:
         *   object path transfer, dict properties
         * Signature: sa{sv} -> oa{sv}
         *
         * Trên target của bạn transfer path không hỗ trợ
         * org.freedesktop.DBus.Properties.Get, nên không poll Status bằng Get.
         * Ta dùng reply properties + chờ file output được ghi xong.
         */
        QDBusMessage pullReply = pbap.call("PullAll", outPath, pullArgs);

        if (pullReply.type() == QDBusMessage::ErrorMessage) {
            localError = "PBAP PullAll failed: " + pullReply.errorMessage();
            break;
        }

        const QList<QVariant> replyArgs = pullReply.arguments();

        if (replyArgs.isEmpty()) {
            localError = "PBAP PullAll returned empty reply";
            break;
        }

        const QDBusObjectPath transferPath =
            qvariant_cast<QDBusObjectPath>(replyArgs.at(0));

        QVariantMap transferProps;

        if (replyArgs.size() >= 2) {
            const QDBusArgument arg =
                qvariant_cast<QDBusArgument>(replyArgs.at(1));

            /*
             * a{sv}
             */
            const QDBusArgument &dictArg = arg;
            dictArg >> transferProps;
        }

        qDebug() << "[ContactsBackend] PBAP PullAll transfer:"
                 << transferPath.path()
                 << "props =" << transferProps;

        if (!waitForObexTransfer(transferPath, 60000, &localError)) {
            break;
        }

        if (!QFile::exists(outPath)) {
            localError = "PBAP transfer completed but file does not exist: "
                         + outPath;
            break;
        }

        QFileInfo info(outPath);

        if (info.size() <= 0) {
            localError = "PBAP output file is empty: " + outPath;
            break;
        }

        qDebug() << "[ContactsBackend] PBAP output file:"
                 << outPath
                 << "size =" << info.size();

        success = true;
    } while (false);

    /*
     * Dọn session PBAP.
     */
    QDBusReply<void> removeReply =
        obexClient.call("RemoveSession", QVariant::fromValue(sessionPath));

    if (!removeReply.isValid()) {
        qWarning() << "[ContactsBackend] RemoveSession failed:"
                   << removeReply.error().message();
    }

    if (!success) {
        if (errorMessage)
            *errorMessage = localError;
        return false;
    }

    if (outputPath)
        *outputPath = outPath;

    qDebug() << "[ContactsBackend] PBAP pull ok:" << outPath;

    return true;
}

bool ContactsBackend::waitForObexTransfer(const QDBusObjectPath &transferPath,
                                          int timeoutMs,
                                          QString *errorMessage) const
{
    Q_UNUSED(transferPath)

    /*
     * Lý do không đọc org.freedesktop.DBus.Properties.Get:
     * Trên BlueZ/OBEX target hiện tại, transfer path báo:
     *   Method "Get" with signature "ss" ... doesn't exist
     *
     * Vì PullAll ghi trực tiếp ra file đích, ta theo dõi file trong /tmp/phonebooks.
     * Do hàm này không nhận outPath, ta suy ra file mới nhất trong /tmp/phonebooks.
     */

    QElapsedTimer timer;
    timer.start();

    QString candidatePath;
    qint64 lastSize = -1;
    int stableCount = 0;

    while (timer.elapsed() < timeoutMs) {
        QDir dir("/tmp/phonebooks");

        QFileInfo newest;

        if (dir.exists()) {
            const QFileInfoList files = dir.entryInfoList(
                QStringList() << "*.vcf",
                QDir::Files,
                QDir::Time);

            if (!files.isEmpty())
                newest = files.first();
        }

        if (newest.exists()) {
            candidatePath = newest.absoluteFilePath();

            const qint64 size = newest.size();

            qDebug() << "[ContactsBackend] waiting PBAP file:"
                     << candidatePath
                     << "size =" << size
                     << "lastSize =" << lastSize
                     << "stableCount =" << stableCount
                     << "elapsed =" << timer.elapsed();

            if (size > 0 && size == lastSize) {
                stableCount++;
            } else {
                stableCount = 0;
            }

            lastSize = size;

            /*
             * File size ổn định khoảng 1 giây là coi như xong.
             */
            if (size > 0 && stableCount >= 4) {
                qDebug() << "[ContactsBackend] PBAP file stable:"
                         << candidatePath
                         << "size =" << size;
                return true;
            }
        } else {
            qDebug() << "[ContactsBackend] waiting PBAP file, no .vcf yet"
                     << "elapsed =" << timer.elapsed();
        }

        QThread::msleep(250);
    }

    if (errorMessage) {
        *errorMessage =
            "PBAP file wait timeout, candidate="
            + candidatePath
            + ", lastSize="
            + QString::number(lastSize);
    }

    qWarning() << "[ContactsBackend] PBAP file wait timeout:"
               << "candidate =" << candidatePath
               << "lastSize =" << lastSize;

    return false;
}

bool ContactsBackend::loadFromVCard(QString path)
{
    path = path.trimmed();

    if (path.isEmpty())
        path = m_defaultPath;

    qDebug() << "[ContactsBackend] loadFromVCard:" << path;

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString msg = "Cannot open vCard file: " + path;
        qWarning() << "[ContactsBackend]" << msg;
        emit errorOccurred(msg);
        return false;
    }

    QVector<ContactEntry> loaded;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    bool inCard = false;
    QString name;
    QString number;
    QString pendingLine;

    auto processLine = [&](QString rawLine) {
        QString line = rawLine.trimmed();

        if (line == "BEGIN:VCARD") {
            inCard = true;
            name.clear();
            number.clear();
            return;
        }

        if (line == "END:VCARD") {
            if (!name.isEmpty() && !number.isEmpty()) {
                ContactEntry c;
                c.name = decodeVCardText(name);
                c.number = normalizePhone(number);

                if (!c.name.isEmpty() && !c.number.isEmpty()) {
                    loaded.push_back(c);

                    qDebug() << "[ContactsBackend] loaded contact:"
                             << c.name
                             << c.number;
                }
            }

            inCard = false;
            name.clear();
            number.clear();
            return;
        }

        if (!inCard)
            return;

        if (line.startsWith("FN:", Qt::CaseInsensitive)) {
            name = line.mid(3).trimmed();
            return;
        }

        if (line.startsWith("FN;", Qt::CaseInsensitive)) {
            const int colon = line.indexOf(':');
            if (colon >= 0)
                name = line.mid(colon + 1).trimmed();
            return;
        }

        if (line.startsWith("N:", Qt::CaseInsensitive) && name.isEmpty()) {
            QString n = line.mid(2).trimmed();
            n.replace(';', ' ');
            name = n.simplified();
            return;
        }

        if (line.startsWith("N;", Qt::CaseInsensitive) && name.isEmpty()) {
            const int colon = line.indexOf(':');

            if (colon >= 0) {
                QString n = line.mid(colon + 1).trimmed();
                n.replace(';', ' ');
                name = n.simplified();
            }

            return;
        }

        if (line.startsWith("TEL", Qt::CaseInsensitive)) {
            const int colon = line.indexOf(':');

            if (colon >= 0 && number.isEmpty()) {
                number = line.mid(colon + 1).trimmed();
            }

            return;
        }
    };

    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.startsWith(' ') || line.startsWith('\t')) {
            pendingLine += line.mid(1);
            continue;
        }

        if (!pendingLine.isEmpty()) {
            processLine(pendingLine);
            pendingLine.clear();
        }

        pendingLine = line;
    }

    if (!pendingLine.isEmpty())
        processLine(pendingLine);

    m_contacts = loaded;
    m_currentPhonebookPath = path;

    qDebug() << "[ContactsBackend] contacts loaded:" << m_contacts.size();

    emit contactsLoaded(m_contacts.size());
    return true;
}

QVector<ContactEntry> ContactsBackend::findMatches(QString name) const
{
    const QString q = normalizeName(name);

    QVector<ContactEntry> matches;

    if (q.isEmpty())
        return matches;

    for (const ContactEntry &c : m_contacts) {
        if (normalizeName(c.name) == q)
            matches.push_back(c);
    }

    if (!matches.isEmpty())
        return matches;

    for (const ContactEntry &c : m_contacts) {
        const QString cn = normalizeName(c.name);

        if (cn.contains(q) || q.contains(cn))
            matches.push_back(c);
    }

    return matches;
}

QString ContactsBackend::findNumberByName(QString name) const
{
    const QVector<ContactEntry> matches = findMatches(name);

    if (matches.size() == 1)
        return matches.first().number;

    return "";
}

bool ContactsBackend::hasContact(QString name) const
{
    const QVector<ContactEntry> matches = findMatches(name);

    const bool found = !matches.isEmpty();

    qDebug() << "[ContactsBackend] hasContact:"
             << name
             << "found =" << found
             << "matches =" << matches.size();

    return found;
}

bool ContactsBackend::callByName(QString name)
{
    qDebug() << "[ContactsBackend] callByName:" << name;

    if (!m_telephony) {
        emit errorOccurred("Telephony backend is null");
        return false;
    }

    const QVector<ContactEntry> matches = findMatches(name);

    if (matches.isEmpty()) {
        qWarning() << "[ContactsBackend] contact not found:" << name;
        emit contactNotFound(name);
        return false;
    }

    if (matches.size() > 1) {
        QStringList list;

        for (const ContactEntry &c : matches)
            list << c.name + " - " + c.number;

        qWarning() << "[ContactsBackend] ambiguous contact:"
                   << name
                   << list;

        emit ambiguousContact(name, list);
        return false;
    }

    const ContactEntry contact = matches.first();

    qDebug() << "[ContactsBackend] dialing contact:"
             << contact.name
             << contact.number;

    const bool ok = m_telephony->dial(contact.number);

    if (!ok) {
        qWarning() << "[ContactsBackend] dial failed for contact:"
                   << contact.name
                   << contact.number;

        emit errorOccurred("Dial failed for contact: " + contact.name);
        return false;
    }

    emit callStarted(contact.name, contact.number);
    return true;
}

int ContactsBackend::count() const
{
    return m_contacts.size();
}

QStringList ContactsBackend::allContactNames() const
{
    QStringList names;

    for (const ContactEntry &c : m_contacts)
        names << c.name;

    return names;
}
