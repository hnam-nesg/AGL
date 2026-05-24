#include "telephony.h"

#include <QDebug>
#include <QRegularExpression>
#include <QThread>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusVariant>

#include <QVariant>
#include <QVariantMap>

static bool dbusObjectHasInterface(const QString &service,
                                   const QString &path,
                                   const QString &interfaceName)
{
    QDBusInterface introspectable(
        service,
        path,
        "org.freedesktop.DBus.Introspectable",
        QDBusConnection::systemBus());

    if (!introspectable.isValid()) {
        qWarning() << "[DBusHelper] Introspectable invalid:"
                   << service
                   << path
                   << introspectable.lastError().message();
        return false;
    }

    QDBusReply<QString> reply = introspectable.call("Introspect");

    if (!reply.isValid()) {
        qWarning() << "[DBusHelper] Introspect failed:"
                   << service
                   << path
                   << reply.error().message();
        return false;
    }

    const QString xml = reply.value();

    const bool found = xml.contains(interfaceName);

    qDebug() << "[DBusHelper] objectHasInterface:"
             << "service =" << service
             << "path =" << path
             << "interface =" << interfaceName
             << "found =" << found;

    return found;
}

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

static bool bluezDeviceConnected(const QString &address, bool *known = nullptr)
{
    if (known)
        *known = false;

    const QString devicePath = bluezDevicePathFromAddress(address);

    if (devicePath.isEmpty())
        return false;

    QDBusInterface properties(
        "org.bluez",
        devicePath,
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus());

    if (!properties.isValid()) {
        qWarning() << "[DBusHelper] BlueZ properties invalid:"
                   << devicePath
                   << properties.lastError().message();
        return false;
    }

    QDBusReply<QVariant> reply =
        properties.call("Get", "org.bluez.Device1", "Connected");

    if (!reply.isValid()) {
        qWarning() << "[DBusHelper] BlueZ Connected Get failed:"
                   << devicePath
                   << reply.error().message();
        return false;
    }

    if (known)
        *known = true;

    const bool connected = reply.value().toBool();

    qDebug() << "[DBusHelper] BlueZ device connected:"
             << "address =" << address
             << "path =" << devicePath
             << "connected =" << connected;

    return connected;
}

static QVariantMap getOfonoModemProperties(const QString &path)
{
    QDBusInterface modem(
        "org.ofono",
        path,
        "org.ofono.Modem",
        QDBusConnection::systemBus());

    if (!modem.isValid()) {
        qWarning() << "[DBusHelper] GetProperties modem invalid:"
                   << path
                   << modem.lastError().message();
        return {};
    }

    QDBusReply<QVariantMap> reply = modem.call("GetProperties");

    if (!reply.isValid()) {
        qWarning() << "[DBusHelper] GetProperties failed:"
                   << path
                   << reply.error().message();
        return {};
    }

    return reply.value();
}

static bool setOfonoModemBoolProperty(const QString &path,
                                      const QString &name,
                                      bool value)
{
    QDBusInterface modem(
        "org.ofono",
        path,
        "org.ofono.Modem",
        QDBusConnection::systemBus());

    if (!modem.isValid()) {
        qWarning() << "[DBusHelper] Cannot SetProperty, modem invalid:"
                   << path
                   << modem.lastError().message();
        return false;
    }

    QDBusReply<void> reply = modem.call(
        "SetProperty",
        name,
        QVariant::fromValue(QDBusVariant(QVariant(value))));

    if (!reply.isValid()) {
        qWarning() << "[DBusHelper] SetProperty failed:"
                   << path
                   << name
                   << value
                   << reply.error().message();
        return false;
    }

    qDebug() << "[DBusHelper] SetProperty ok:"
             << path
             << name
             << value;

    return true;
}

Telephony::Telephony(QObject *parent)
    : QObject(parent)
{
    /*
     * Không hard-code thiết bị ở constructor.
     * Phone service sẽ tự autoSelect bằng oFono khi cần.
     */
    setConnected(false);
    setOnlineState(false);
    setCallState("disconnected");

    m_deviceRefreshTimer.setSingleShot(true);
    m_deviceRefreshTimer.setInterval(800);
    connect(&m_deviceRefreshTimer,
            &QTimer::timeout,
            this,
            &Telephony::refreshBluetoothDeviceSelection);

    connectDeviceChangeSignals();
}

Telephony::~Telephony()
{
    disconnectVoiceCallManagerSignals();
}

bool Telephony::autoSelectBluetoothDevice()
{
    return autoSelectBluetoothDeviceInternal(true);
}

bool Telephony::autoSelectBluetoothDeviceInternal(bool emitErrors)
{
    qDebug() << "[Telephony] autoSelectBluetoothDevice() using org.ofono.Manager.GetModems";

    QDBusInterface manager(
        "org.ofono",
        "/",
        "org.ofono.Manager",
        QDBusConnection::systemBus());

    if (!manager.isValid()) {
        qWarning() << "[Telephony] org.ofono.Manager invalid:"
                   << manager.lastError().message();

        if (emitErrors)
            emit errorOccurred("oFono Manager invalid: " + manager.lastError().message());

        return false;
    }

    QDBusMessage reply = manager.call("GetModems");

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "[Telephony] GetModems failed:"
                   << reply.errorMessage();

        if (emitErrors)
            emit errorOccurred("oFono GetModems failed: " + reply.errorMessage());

        return false;
    }

    const QList<QVariant> args = reply.arguments();

    if (args.isEmpty()) {
        qWarning() << "[Telephony] GetModems returned empty";

        if (emitErrors)
            emit errorOccurred("oFono GetModems returned empty");

        return false;
    }

    const QDBusArgument dbusArg = qvariant_cast<QDBusArgument>(args.at(0));
    const QDBusArgument &arrayArg = dbusArg;

    QString selectedDeviceId;
    QString selectedPath;

    arrayArg.beginArray();

    while (!arrayArg.atEnd()) {
        QDBusObjectPath modemPath;
        QVariantMap properties;

        arrayArg.beginStructure();
        arrayArg >> modemPath >> properties;
        arrayArg.endStructure();

        const QString path = modemPath.path();

        qDebug() << "[Telephony] oFono modem:"
                 << "path =" << path
                 << "properties =" << properties;

        if (!path.startsWith("/hfp/org/bluez/hci0/dev_")) {
            qDebug() << "[Telephony] skip non-HFP modem:" << path;
            continue;
        }

        const QString address = bluetoothAddressFromOfonoHfpPath(path);

        if (address.isEmpty()) {
            qWarning() << "[Telephony] invalid HFP path, cannot extract address:"
                       << path;
            continue;
        }

        bool connectedKnown = false;
        const bool bluezConnected = bluezDeviceConnected(address, &connectedKnown);

        if (!connectedKnown || !bluezConnected) {
            qDebug() << "[Telephony] skip disconnected/stale HFP modem:"
                     << "path =" << path
                     << "address =" << address
                     << "connectedKnown =" << connectedKnown
                     << "bluezConnected =" << bluezConnected;
            continue;
        }

        qDebug() << "[Telephony] prepare connected HFP modem:" << path;

        const bool poweredOk = setOfonoModemBoolProperty(path, "Powered", true);
        const bool onlineOk = setOfonoModemBoolProperty(path, "Online", true);

        qDebug() << "[Telephony] modem power result:"
                 << "path =" << path
                 << "PoweredOk =" << poweredOk
                 << "OnlineOk =" << onlineOk;

        QThread::msleep(500);

        QVariantMap refreshed = getOfonoModemProperties(path);

        qDebug() << "[Telephony] refreshed modem properties:"
                 << path
                 << refreshed;

        QStringList interfaces =
            refreshed.value("Interfaces").toStringList();

        bool hasVoiceCallManager =
            interfaces.contains("org.ofono.VoiceCallManager");

        if (!hasVoiceCallManager) {
            hasVoiceCallManager = dbusObjectHasInterface(
                "org.ofono",
                path,
                "org.ofono.VoiceCallManager");
        }

        if (!hasVoiceCallManager) {
            QThread::msleep(700);

            refreshed = getOfonoModemProperties(path);
            interfaces = refreshed.value("Interfaces").toStringList();

            qDebug() << "[Telephony] refreshed modem properties retry:"
                     << path
                     << refreshed;

            hasVoiceCallManager =
                interfaces.contains("org.ofono.VoiceCallManager");

            if (!hasVoiceCallManager) {
                hasVoiceCallManager = dbusObjectHasInterface(
                    "org.ofono",
                    path,
                    "org.ofono.VoiceCallManager");
            }
        }

        if (!hasVoiceCallManager) {
            qWarning() << "[Telephony] skip HFP modem without real VoiceCallManager:"
                       << path
                       << "Interfaces =" << interfaces;
            continue;
        }

        selectedPath = path;
        selectedDeviceId = address;

        qDebug() << "[Telephony] selected HFP modem with VoiceCallManager:"
                 << "path =" << selectedPath
                 << "address =" << selectedDeviceId;

        break;
    }

    arrayArg.endArray();

    if (selectedDeviceId.isEmpty()) {
        qWarning() << "[Telephony] no HFP modem with VoiceCallManager found";

        if (emitErrors)
            emit errorOccurred("No connected HFP phone with VoiceCallManager found in oFono");

        clearSelectedBluetoothDevice("no connected HFP modem with VoiceCallManager");
        return false;
    }

    setBluetoothDevice(selectedDeviceId);

    if (!m_connected || m_voiceCallManagerPath.isEmpty()) {
        qWarning() << "[Telephony] selected device but not connected:"
                   << "connected =" << m_connected
                   << "path =" << m_voiceCallManagerPath;

        if (emitErrors)
            emit errorOccurred("Selected HFP phone but VoiceCallManager is not connected");

        return false;
    }

    return true;
}

QString Telephony::normalizeBluetoothAddress(QString deviceIdOrAddress) const
{
    QString s = deviceIdOrAddress.trimmed();

    /*
     * Nhận các dạng:
     * 1. 88:A4:79:65:91:34
     * 2. dev_88_A4_79_65_91_34
     * 3. /org/bluez/hci0/dev_88_A4_79_65_91_34
     */

    static const QRegularExpression macRe(
        R"(([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2})"
    );

    QRegularExpressionMatch macMatch = macRe.match(s);
    if (macMatch.hasMatch())
        return macMatch.captured(0).toUpper();

    int devIndex = s.indexOf("dev_");
    if (devIndex >= 0) {
        QString dev = s.mid(devIndex + 4);
        dev = dev.replace("_", ":");

        QRegularExpressionMatch devMacMatch = macRe.match(dev);
        if (devMacMatch.hasMatch())
            return devMacMatch.captured(0).toUpper();
    }

    return "";
}

QString Telephony::bluetoothAddressToBluezDeviceName(QString address) const
{
    address = normalizeBluetoothAddress(address);

    if (address.isEmpty())
        return "";

    return "dev_" + address.replace(":", "_");
}

QString Telephony::bluetoothAddressToOfonoHfpPath(QString address) const
{
    const QString devName = bluetoothAddressToBluezDeviceName(address);

    if (devName.isEmpty())
        return "";

    return "/hfp/org/bluez/hci0/" + devName;
}

void Telephony::setBluetoothDevice(QString deviceIdOrAddress)
{
    qDebug() << "[Telephony] setBluetoothDevice:" << deviceIdOrAddress;

    const QString address = normalizeBluetoothAddress(deviceIdOrAddress);

    if (address.isEmpty()) {
        qWarning() << "[Telephony] invalid bluetooth device:" << deviceIdOrAddress;
        emit errorOccurred("Invalid Bluetooth device for telephony: " + deviceIdOrAddress);
        return;
    }

    const QString newPath = bluetoothAddressToOfonoHfpPath(address);

    if (newPath.isEmpty()) {
        qWarning() << "[Telephony] failed to build oFono HFP path from:" << address;
        emit errorOccurred("Failed to build oFono path for: " + address);
        return;
    }

    bool connectedKnown = false;
    const bool bluezConnected = bluezDeviceConnected(address, &connectedKnown);

    if (!connectedKnown || !bluezConnected) {
        qWarning() << "[Telephony] cannot select disconnected BlueZ device:"
                   << "address =" << address
                   << "connectedKnown =" << connectedKnown
                   << "bluezConnected =" << bluezConnected;

        clearSelectedBluetoothDevice("selected BlueZ device is not connected");
        emit errorOccurred("Bluetooth phone is not connected: " + address);
        return;
    }

    if (m_bluetoothAddress == address && m_voiceCallManagerPath == newPath) {
        if (validateVoiceCallManagerPath()) {
            qDebug() << "[Telephony] same connected bluetooth device, keep selection";
            setConnected(true);
            setOnlineState(true);
            return;
        }

        qWarning() << "[Telephony] same bluetooth device but old oFono path is stale:"
                   << newPath;
    }

    disconnectVoiceCallManagerSignals();

    m_bluetoothAddress = address;
    emit bluetoothAddressChanged(m_bluetoothAddress);

    m_voiceCallManagerPath = newPath;
    m_currentCallPath.clear();

    setCallState("disconnected");
    setCallClip("");
    setCallColp("");

    qDebug() << "[Telephony] selected address =" << m_bluetoothAddress;
    qDebug() << "[Telephony] selected oFono path =" << m_voiceCallManagerPath;

    if (!validateVoiceCallManagerPath()) {
        setConnected(false);
        setOnlineState(false);

        emit errorOccurred("oFono VoiceCallManager not available at: " + m_voiceCallManagerPath);
        return;
    }

    connectVoiceCallManagerSignals();

    setConnected(true);
    setOnlineState(true);
}

bool Telephony::selectedBluetoothDeviceStillConnected() const
{
    if (m_bluetoothAddress.isEmpty() || m_voiceCallManagerPath.isEmpty())
        return false;

    bool connectedKnown = false;
    const bool bluezConnected = bluezDeviceConnected(m_bluetoothAddress,
                                                    &connectedKnown);

    if (!connectedKnown || !bluezConnected) {
        qDebug() << "[Telephony] selected BlueZ device is no longer connected:"
                 << "address =" << m_bluetoothAddress
                 << "connectedKnown =" << connectedKnown
                 << "bluezConnected =" << bluezConnected;
        return false;
    }

    if (!dbusObjectHasInterface("org.ofono",
                                m_voiceCallManagerPath,
                                "org.ofono.VoiceCallManager")) {
        qDebug() << "[Telephony] selected oFono VoiceCallManager is stale:"
                 << m_voiceCallManagerPath;
        return false;
    }

    return true;
}

void Telephony::clearSelectedBluetoothDevice(const QString &reason)
{
    qDebug() << "[Telephony] clearSelectedBluetoothDevice:" << reason
             << "address =" << m_bluetoothAddress
             << "path =" << m_voiceCallManagerPath;

    disconnectVoiceCallManagerSignals();

    const bool hadAddress = !m_bluetoothAddress.isEmpty();

    m_bluetoothAddress.clear();
    m_voiceCallManagerPath.clear();
    disconnectCurrentVoiceCallSignals();
    m_currentCallPath.clear();

    setConnected(false);
    setOnlineState(false);
    setCallState("disconnected");
    setCallClip("");
    setCallColp("");

    if (hadAddress)
        emit bluetoothAddressChanged(m_bluetoothAddress);
}

void Telephony::connectDeviceChangeSignals()
{
    bool ok1 = QDBusConnection::systemBus().connect(
        "org.ofono",
        "/",
        "org.ofono.Manager",
        "ModemAdded",
        this,
        SLOT(onOfonoModemAdded(QDBusObjectPath,QVariantMap)));

    bool ok2 = QDBusConnection::systemBus().connect(
        "org.ofono",
        "/",
        "org.ofono.Manager",
        "ModemRemoved",
        this,
        SLOT(onOfonoModemRemoved(QDBusObjectPath)));

    bool ok3 = QDBusConnection::systemBus().connect(
        "org.bluez",
        QString(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this,
        SLOT(onBluezPropertiesChanged(QString,QVariantMap,QStringList)));

    bool ok4 = QDBusConnection::systemBus().connect(
        "org.bluez",
        "/",
        "org.freedesktop.DBus.ObjectManager",
        "InterfacesAdded",
        this,
        SLOT(onBluezInterfacesAdded(QDBusObjectPath,QVariantMap)));

    bool ok5 = QDBusConnection::systemBus().connect(
        "org.bluez",
        "/",
        "org.freedesktop.DBus.ObjectManager",
        "InterfacesRemoved",
        this,
        SLOT(onBluezInterfacesRemoved(QDBusObjectPath,QStringList)));

    qDebug() << "[Telephony] DBus connect device change signals:"
             << "ofonoAdded =" << ok1
             << "ofonoRemoved =" << ok2
             << "bluezProperties =" << ok3
             << "bluezInterfacesAdded =" << ok4
             << "bluezInterfacesRemoved =" << ok5;
}

void Telephony::onOfonoModemAdded(const QDBusObjectPath &path,
                                  const QVariantMap &properties)
{
    qDebug() << "[Telephony] oFono ModemAdded:"
             << path.path()
             << properties;

    if (!path.path().startsWith("/hfp/org/bluez/hci0/dev_"))
        return;

    m_deviceRefreshTimer.start();
}

void Telephony::onOfonoModemRemoved(const QDBusObjectPath &path)
{
    qDebug() << "[Telephony] oFono ModemRemoved:" << path.path();

    if (!path.path().startsWith("/hfp/org/bluez/hci0/dev_"))
        return;

    if (path.path() == m_voiceCallManagerPath)
        clearSelectedBluetoothDevice("selected oFono modem was removed");

    m_deviceRefreshTimer.start();
}

void Telephony::onBluezPropertiesChanged(const QString &interfaceName,
                                         const QVariantMap &changedProperties,
                                         const QStringList &invalidatedProperties)
{
    if (interfaceName != "org.bluez.Device1")
        return;

    if (!changedProperties.contains("Connected")
            && !invalidatedProperties.contains("Connected"))
        return;

    qDebug() << "[Telephony] BlueZ Device1 Connected changed:"
             << changedProperties
             << "invalidated =" << invalidatedProperties;

    if (!selectedBluetoothDeviceStillConnected())
        clearSelectedBluetoothDevice("selected BlueZ device disconnected or became stale");

    m_deviceRefreshTimer.start();
}

void Telephony::onBluezInterfacesAdded(const QDBusObjectPath &path,
                                       const QVariantMap &interfaces)
{
    if (!path.path().startsWith("/org/bluez/hci0/dev_"))
        return;

    if (!interfaces.contains("org.bluez.Device1"))
        return;

    qDebug() << "[Telephony] BlueZ InterfacesAdded:" << path.path();
    m_deviceRefreshTimer.start();
}

void Telephony::onBluezInterfacesRemoved(const QDBusObjectPath &path,
                                         const QStringList &interfaces)
{
    if (!path.path().startsWith("/org/bluez/hci0/dev_"))
        return;

    if (!interfaces.contains("org.bluez.Device1"))
        return;

    qDebug() << "[Telephony] BlueZ InterfacesRemoved:" << path.path();

    const QString currentBluezPath = bluezDevicePathFromAddress(m_bluetoothAddress);

    if (path.path() == currentBluezPath)
        clearSelectedBluetoothDevice("selected BlueZ device was removed");

    m_deviceRefreshTimer.start();
}

void Telephony::refreshBluetoothDeviceSelection()
{
    qDebug() << "[Telephony] refreshBluetoothDeviceSelection";

    if (selectedBluetoothDeviceStillConnected()) {
        setConnected(true);
        setOnlineState(true);
        return;
    }

    autoSelectBluetoothDeviceInternal(false);
}

bool Telephony::validateVoiceCallManagerPath()
{
    if (m_voiceCallManagerPath.isEmpty())
        return false;

    const bool hasVoiceCallManager = dbusObjectHasInterface(
        "org.ofono",
        m_voiceCallManagerPath,
        "org.ofono.VoiceCallManager");

    if (!hasVoiceCallManager) {
        qWarning() << "[Telephony] VoiceCallManager NOT present at"
                   << m_voiceCallManagerPath;
        return false;
    }

    qDebug() << "[Telephony] VoiceCallManager present at"
             << m_voiceCallManagerPath;

    return true;
}

void Telephony::connectVoiceCallManagerSignals()
{
    if (m_voiceCallManagerPath.isEmpty()) {
        qWarning() << "[Telephony] empty VoiceCallManager path";
        return;
    }

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

    qDebug() << "[Telephony] DBus connect CallAdded =" << ok1
             << "path =" << m_voiceCallManagerPath;

    qDebug() << "[Telephony] DBus connect CallRemoved =" << ok2
             << "path =" << m_voiceCallManagerPath;

    if (!ok1 || !ok2) {
        emit errorOccurred("Failed to connect oFono call signals for: " + m_voiceCallManagerPath);
    }
}

void Telephony::disconnectVoiceCallManagerSignals()
{
    if (m_voiceCallManagerPath.isEmpty())
        return;

    bool ok1 = QDBusConnection::systemBus().disconnect(
        "org.ofono",
        m_voiceCallManagerPath,
        "org.ofono.VoiceCallManager",
        "CallAdded",
        this,
        SLOT(onCallAdded(QDBusObjectPath,QVariantMap)));

    bool ok2 = QDBusConnection::systemBus().disconnect(
        "org.ofono",
        m_voiceCallManagerPath,
        "org.ofono.VoiceCallManager",
        "CallRemoved",
        this,
        SLOT(onCallRemoved(QDBusObjectPath)));

    qDebug() << "[Telephony] DBus disconnect CallAdded =" << ok1
             << "CallRemoved =" << ok2
             << "path =" << m_voiceCallManagerPath;
}

void Telephony::connectCurrentVoiceCallSignals()
{
    if (m_currentCallPath.isEmpty())
        return;

    bool ok = QDBusConnection::systemBus().connect(
        "org.ofono",
        m_currentCallPath,
        "org.ofono.VoiceCall",
        "PropertyChanged",
        this,
        SLOT(onVoiceCallPropertyChanged(QString,QDBusVariant)));

    qDebug() << "[Telephony] DBus connect VoiceCall PropertyChanged ="
             << ok
             << "path =" << m_currentCallPath;

    if (!ok)
        emit errorOccurred("Failed to connect oFono voice call signal for: " + m_currentCallPath);
}

void Telephony::disconnectCurrentVoiceCallSignals()
{
    if (m_currentCallPath.isEmpty())
        return;

    bool ok = QDBusConnection::systemBus().disconnect(
        "org.ofono",
        m_currentCallPath,
        "org.ofono.VoiceCall",
        "PropertyChanged",
        this,
        SLOT(onVoiceCallPropertyChanged(QString,QDBusVariant)));

    qDebug() << "[Telephony] DBus disconnect VoiceCall PropertyChanged ="
             << ok
             << "path =" << m_currentCallPath;
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
    qDebug() << "[Telephony] onCallAdded:" << path.path() << properties;

    if (m_currentCallPath != path.path())
        disconnectCurrentVoiceCallSignals();

    m_currentCallPath = path.path();
    connectCurrentVoiceCallSignals();

    const QString state = properties.value("State").toString();
    const QString lineId = properties.value("LineIdentification").toString();
    const QString name = properties.value("Name").toString();

    applyVoiceCallState(state, lineId, name);
}

void Telephony::onCallRemoved(const QDBusObjectPath &path)
{
    qDebug() << "[Telephony] onCallRemoved:" << path.path();

    if (path.path() == m_currentCallPath || m_currentCallPath.isEmpty()) {
        disconnectCurrentVoiceCallSignals();
        setCallState("disconnected");
        setCallClip("");
        setCallColp("");
        m_currentCallPath.clear();
    }
}

void Telephony::onVoiceCallPropertyChanged(const QString &name,
                                           const QDBusVariant &value)
{
    const QVariant v = value.variant();

    qDebug() << "[Telephony] VoiceCall PropertyChanged:"
             << name
             << v;

    if (name == "State") {
        applyVoiceCallState(v.toString());
    } else if (name == "LineIdentification") {
        const QString lineId = v.toString();

        if (!lineId.isEmpty())
            setCallClip(lineId);
    } else if (name == "Name") {
        const QString callerName = v.toString();

        if (m_clip.isEmpty() && !callerName.isEmpty())
            setCallClip(callerName);
    }
}

void Telephony::applyVoiceCallState(const QString &state,
                                    const QString &lineId,
                                    const QString &name)
{
    if (!lineId.isEmpty())
        setCallClip(lineId);
    else if (!name.isEmpty())
        setCallClip(name);

    if (state == "dialing" || state == "alerting" || state == "active")
        setCallColp(lineId);

    if (state == "incoming") {
        setCallState("incoming");
    } else if (state == "dialing") {
        setCallState("dialing");
    } else if (state == "alerting") {
        setCallState("alerting");
    } else if (state == "active") {
        setCallState("active");
    } else if (state == "held") {
        setCallState("held");
    } else if (state == "waiting") {
        setCallState("waiting");
    } else if (state == "disconnected") {
        setCallState("disconnected");
    } else if (!state.isEmpty()) {
        setCallState(state);
    }
}

bool Telephony::dial(QString number)
{
    number = number.trimmed();

    qDebug() << "[Telephony] dial:" << number
             << "managerPath =" << m_voiceCallManagerPath
             << "connected =" << m_connected;

    if (number.isEmpty()) {
        qWarning() << "[Telephony] cannot dial empty number";
        emit errorOccurred("Cannot dial: empty number");
        return false;
    }

    if (!selectedBluetoothDeviceStillConnected()) {
        qWarning() << "[Telephony] no usable VoiceCallManager, auto selecting...";

        clearSelectedBluetoothDevice("dial requested while current selection is stale");

        if (!autoSelectBluetoothDevice()) {
            qWarning() << "[Telephony] autoSelectBluetoothDevice failed";
            emit errorOccurred("No usable Bluetooth phone for dialing");
            return false;
        }
    }

    if (!dbusObjectHasInterface("org.ofono",
                                m_voiceCallManagerPath,
                                "org.ofono.VoiceCallManager")) {
        qWarning() << "[Telephony] selected path still has no VoiceCallManager:"
                   << m_voiceCallManagerPath;

        emit errorOccurred("Selected oFono path has no VoiceCallManager");
        return false;
    }

    QDBusInterface iface(
        "org.ofono",
        m_voiceCallManagerPath,
        "org.ofono.VoiceCallManager",
        QDBusConnection::systemBus());

    if (!iface.isValid()) {
        qWarning() << "[Telephony] VoiceCallManager interface invalid:"
                   << m_voiceCallManagerPath
                   << iface.lastError().message();

        emit errorOccurred("VoiceCallManager interface invalid: " + m_voiceCallManagerPath);
        return false;
    }

    /*
     * oFono thường dùng Dial(string number, string hide_callerid).
     * Một vài build khác có thể dùng Dial(string number), nên fallback thêm.
     */
    QDBusReply<QDBusObjectPath> dialReply =
        iface.call("Dial", number, "default");

    if (!dialReply.isValid()) {
        qWarning() << "[Telephony] Dial(number, default) failed:"
                   << dialReply.error().message();

        dialReply = iface.call("Dial", number);
    }

    if (!dialReply.isValid()) {
        qWarning() << "[Telephony] Dial failed:"
                   << dialReply.error().message();

        setCallState("disconnected");
        setCallColp("");

        emit errorOccurred("Dial failed: " + dialReply.error().message());
        return false;
    }

    m_currentCallPath = dialReply.value().path();
    connectCurrentVoiceCallSignals();

    qDebug() << "[Telephony] Dial ok callPath =" << m_currentCallPath;

    setCallColp(number);
    setCallState("dialing");

    return true;
}

void Telephony::answer()
{
    qDebug() << "[Telephony] answer currentCallPath =" << m_currentCallPath;

    if (m_currentCallPath.isEmpty()) {
        qWarning() << "[Telephony] no current call path to answer";
        emit errorOccurred("No current call path to answer");
        return;
    }

    QDBusInterface iface(
        "org.ofono",
        m_currentCallPath,
        "org.ofono.VoiceCall",
        QDBusConnection::systemBus());

    if (!iface.isValid()) {
        qWarning() << "[Telephony] VoiceCall interface invalid for answer";
        emit errorOccurred("VoiceCall interface invalid for answer: " + m_currentCallPath);
        return;
    }

    QDBusReply<void> reply = iface.call("Answer");

    if (!reply.isValid()) {
        qWarning() << "[Telephony] Answer failed:" << reply.error().message();
        emit errorOccurred("Answer failed: " + reply.error().message());
    }
}

void Telephony::hangup()
{
    qDebug() << "[Telephony] hangup currentCallPath =" << m_currentCallPath;

    if (m_currentCallPath.isEmpty()) {
        qWarning() << "[Telephony] no current call path to hangup";
        emit errorOccurred("No current call path to hangup");
        return;
    }

    QDBusInterface iface(
        "org.ofono",
        m_currentCallPath,
        "org.ofono.VoiceCall",
        QDBusConnection::systemBus());

    if (!iface.isValid()) {
        qWarning() << "[Telephony] VoiceCall interface invalid for hangup";
        emit errorOccurred("VoiceCall interface invalid for hangup: " + m_currentCallPath);
        return;
    }

    QDBusReply<void> reply = iface.call("Hangup");

    if (!reply.isValid()) {
        qWarning() << "[Telephony] Hangup failed:" << reply.error().message();
        emit errorOccurred("Hangup failed: " + reply.error().message());
    }
}
