#include "bluetoothmediaplayer.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDebug>

namespace {
constexpr const char *BluezService = "org.bluez";
constexpr const char *ObjectManagerInterface = "org.freedesktop.DBus.ObjectManager";
constexpr const char *PropertiesInterface = "org.freedesktop.DBus.Properties";
constexpr const char *DeviceInterface = "org.bluez.Device1";
constexpr const char *MediaControlInterface = "org.bluez.MediaControl1";
constexpr const char *MediaPlayerInterface = "org.bluez.MediaPlayer1";
constexpr const char *SocialService = "org.agl.social";
constexpr const char *SocialObjectPath = "/org/agl/social";
constexpr const char *SocialInterface = "org.agl.social";

constexpr const char *A2dpSourceUuid = "0000110a-0000-1000-8000-00805f9b34fb";
constexpr const char *AvrcpRemoteControlUuid = "0000110e-0000-1000-8000-00805f9b34fb";

constexpr uchar AvrcpRewindKey = 0x48;
constexpr uchar AvrcpFastForwardKey = 0x49;
}

QDBusArgument &operator<<(QDBusArgument &argument, const BluezInterfaceMap &interfaces)
{
    argument.beginMap(QMetaType::fromType<QString>(), QMetaType::fromType<QVariantMap>());
    for (auto it = interfaces.cbegin(); it != interfaces.cend(); ++it) {
        argument.beginMapEntry();
        argument << it.key() << it.value();
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, BluezInterfaceMap &interfaces)
{
    interfaces.clear();
    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        QVariantMap value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        interfaces.insert(key, value);
    }
    argument.endMap();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const BluezManagedObjectMap &objects)
{
    argument.beginMap(QMetaType::fromType<QDBusObjectPath>(), QMetaType::fromType<BluezInterfaceMap>());
    for (auto it = objects.cbegin(); it != objects.cend(); ++it) {
        argument.beginMapEntry();
        argument << it.key() << it.value();
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, BluezManagedObjectMap &objects)
{
    objects.clear();
    argument.beginMap();
    while (!argument.atEnd()) {
        QDBusObjectPath key;
        BluezInterfaceMap value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        objects.insert(key, value);
    }
    argument.endMap();
    return argument;
}

BluetoothMediaPlayer::BluetoothMediaPlayer(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<BluezInterfaceMap>();
    qDBusRegisterMetaType<BluezManagedObjectMap>();

    m_refreshTimer.setInterval(500);
    m_refreshTimer.setSingleShot(false);
    QObject::connect(&m_refreshTimer, &QTimer::timeout, this, &BluetoothMediaPlayer::refresh);

    m_seekReleaseTimer.setSingleShot(true);
    QObject::connect(&m_seekReleaseTimer, &QTimer::timeout, this, &BluetoothMediaPlayer::finishSeek);

    m_seekPollTimer.setInterval(250);
    m_seekPollTimer.setSingleShot(false);
    QObject::connect(&m_seekPollTimer, &QTimer::timeout, this, &BluetoothMediaPlayer::updateSeekProgress);
}

bool BluetoothMediaPlayer::connected() const
{
    return m_connected;
}

QString BluetoothMediaPlayer::status() const
{
    return m_status;
}

QString BluetoothMediaPlayer::deviceAddress() const
{
    return m_deviceAddress;
}

void BluetoothMediaPlayer::start()
{
    if (m_started)
        return;

    m_started = true;
    refresh();
    m_refreshTimer.start(m_connected ? 500 : 2000);
}

void BluetoothMediaPlayer::connect()
{
    connectBluetooth();
}

void BluetoothMediaPlayer::disconnect()
{
    disconnectBluetooth();
}

void BluetoothMediaPlayer::connectBluetooth()
{
    if (m_devicePath.isEmpty())
        refreshObjects();

    if (m_devicePath.isEmpty()) {
        qWarning() << "No Bluetooth media device found to connect";
        return;
    }

    callDeviceProfile("ConnectProfile", A2dpSourceUuid);
    callDeviceProfile("ConnectProfile", AvrcpRemoteControlUuid);
    refresh();
}

void BluetoothMediaPlayer::disconnectBluetooth()
{
    if (m_devicePath.isEmpty())
        refreshObjects();

    if (m_devicePath.isEmpty()) {
        qWarning() << "No Bluetooth media device found to disconnect";
        return;
    }

    callDeviceProfile("DisconnectProfile", AvrcpRemoteControlUuid);
    callDeviceProfile("DisconnectProfile", A2dpSourceUuid);
    refresh();
}

void BluetoothMediaPlayer::play()
{
    requestExternalMediaPause();
    callPlayerMethod("Play");
}

void BluetoothMediaPlayer::pause()
{
    callPlayerMethod("Pause");
}

void BluetoothMediaPlayer::stop()
{
    callPlayerMethod("Stop");
}

void BluetoothMediaPlayer::previous()
{
    callPlayerMethod("Previous");
}

void BluetoothMediaPlayer::next()
{
    callPlayerMethod("Next");
}

void BluetoothMediaPlayer::seek(int milliseconds)
{
    if (m_playerPath.isEmpty())
        refreshObjects();

	if (m_playerPath.isEmpty()) {
	    qWarning() << "No Bluetooth media player found for seek";
	    return;
	}

	if (m_seekReleaseTimer.isActive() || m_seekPollTimer.isActive())
	    finishSeek();

	refreshPlayerProperties();

	const int maxMs = m_durationMs > 0 ? m_durationMs : milliseconds;
    const int targetMs = qBound(0, milliseconds, maxMs);
    const int offsetMs = targetMs - m_positionMs;
    if (offsetMs == 0)
        return;

    if (trySetPosition(targetMs)) {
        m_positionMs = targetMs;

        QVariantMap metadata;
        metadata.insert("position", m_positionMs);
        metadata.insert("connected", m_connected);
        emitMetadataIfChanged(metadata);

        refreshPlayerProperties();
        return;
    }

    const bool ok = startTimedSeek(offsetMs > 0,
                                   targetMs,
                                   qBound(1000, qAbs(offsetMs) / 2, 30000));

    if (!ok)
        qWarning() << "Bluetooth media seek is not supported by this phone/BlueZ player";
}

void BluetoothMediaPlayer::refreshPosition()
{
    refreshPlayerProperties();
}

void BluetoothMediaPlayer::fastforward(int milliseconds)
{
    Q_UNUSED(milliseconds)
    callPlayerMethod("FastForward");
}

void BluetoothMediaPlayer::rewind(int milliseconds)
{
    Q_UNUSED(milliseconds)
    callPlayerMethod("Rewind");
}

void BluetoothMediaPlayer::picktrack(int track)
{
    Q_UNUSED(track)
}

void BluetoothMediaPlayer::volume(int volume)
{
    Q_UNUSED(volume)
}

void BluetoothMediaPlayer::loop(const QString &state)
{
    if (m_playerPath.isEmpty())
        refreshObjects();

    if (m_playerPath.isEmpty()) {
        qWarning() << "No Bluetooth media player found for repeat mode";
        return;
    }

    const QString normalized = state.toLower();
    const QString repeat = normalized == QLatin1String("one")
        || normalized == QLatin1String("singletrack")
        ? QStringLiteral("singletrack")
        : (normalized == QLatin1String("all")
               || normalized == QLatin1String("alltracks")
               ? QStringLiteral("alltracks")
               : QStringLiteral("off"));

    if (!setProperty(m_playerPath, MediaPlayerInterface, "Repeat", repeat))
        qWarning() << "Bluetooth media repeat mode is not supported by this player";
}

void BluetoothMediaPlayer::shuffle(bool enabled)
{
    if (m_playerPath.isEmpty())
        refreshObjects();

    if (m_playerPath.isEmpty()) {
        qWarning() << "No Bluetooth media player found for shuffle mode";
        return;
    }

    if (!setProperty(m_playerPath,
                     MediaPlayerInterface,
                     "Shuffle",
                     enabled ? QStringLiteral("alltracks") : QStringLiteral("off")))
        qWarning() << "Bluetooth media shuffle mode is not supported by this player";
}

void BluetoothMediaPlayer::refresh()
{
    if (refreshObjects())
        refreshPlayerProperties();
}

void BluetoothMediaPlayer::finishSeek()
{
    m_seekPollTimer.stop();

    if (!m_seekStopMethod.isEmpty()) {
        if (m_seekStopMethod == QLatin1String("Release"))
            callMethod(m_playerPath, MediaPlayerInterface, m_seekStopMethod, {}, false);
        else
            callMethod(m_playerPath, MediaPlayerInterface, m_seekStopMethod, {}, false);
        m_seekStopMethod.clear();
    }

    m_pendingSeekTargetMs = -1;
    refreshPlayerProperties();
}

void BluetoothMediaPlayer::updateSeekProgress()
{
    if (m_pendingSeekTargetMs < 0) {
        m_seekPollTimer.stop();
        return;
    }

    refreshPlayerProperties();

    constexpr int SeekToleranceMs = 1000;
    const bool reached = m_pendingSeekForward
        ? m_positionMs >= m_pendingSeekTargetMs - SeekToleranceMs
        : m_positionMs <= m_pendingSeekTargetMs + SeekToleranceMs;

    if (reached)
        finishSeek();
}

void BluetoothMediaPlayer::onPropertiesChanged(const QString &interface,
                                               const QVariantMap &changed,
                                               const QStringList &invalidated)
{
    if (interface == QLatin1String(MediaPlayerInterface)) {
        if (!changed.isEmpty())
            updateMetadataFromProperties(changed);

        if (changed.isEmpty() || invalidated.contains(QLatin1String("Position"))
            || invalidated.contains(QLatin1String("Track"))
            || invalidated.contains(QLatin1String("Status"))) {
            refreshPlayerProperties();
        }
        return;
    }

    if (interface == QLatin1String(MediaControlInterface) || interface == QLatin1String(DeviceInterface))
        refresh();
}

bool BluetoothMediaPlayer::refreshObjects()
{
    QDBusInterface manager(BluezService,
                           "/",
                           ObjectManagerInterface,
                           QDBusConnection::systemBus());
    if (!manager.isValid()) {
        qWarning() << "BlueZ ObjectManager is unavailable" << manager.lastError().message();
        return false;
    }

    QDBusReply<BluezManagedObjectMap> reply = manager.call("GetManagedObjects");
    if (!reply.isValid()) {
        qWarning() << "Failed to query BlueZ managed objects" << reply.error().message();
        return false;
    }

    const BluezManagedObjectMap objects = reply.value();
    QString playerPath;
    QString mediaDevicePath;
    QString connectedDevicePath;
    bool mediaControlConnected = false;

    for (auto it = objects.cbegin(); it != objects.cend(); ++it) {
        const QString path = it.key().path();
        const BluezInterfaceMap interfaces = it.value();

        if (interfaces.contains(QLatin1String(MediaPlayerInterface)) && playerPath.isEmpty())
            playerPath = path;

        const QVariantMap mediaControlProperties = interfaces.value(QLatin1String(MediaControlInterface));
        if (!mediaControlProperties.isEmpty()) {
            const bool connected = unwrapVariant(mediaControlProperties.value("Connected")).toBool();
            if (connected) {
                mediaControlConnected = true;
                mediaDevicePath = path;
            }

            const QVariant playerVariant = unwrapVariant(mediaControlProperties.value("Player"));
            if (playerVariant.canConvert<QDBusObjectPath>() && playerPath.isEmpty())
                playerPath = playerVariant.value<QDBusObjectPath>().path();
            else if (playerVariant.canConvert<QString>() && playerPath.isEmpty())
                playerPath = playerVariant.toString();
        }

        const QVariantMap deviceProperties = interfaces.value(QLatin1String(DeviceInterface));
        if (!deviceProperties.isEmpty()) {
            const bool connected = unwrapVariant(deviceProperties.value("Connected")).toBool();
            if (connected && connectedDevicePath.isEmpty())
                connectedDevicePath = path;
            if (deviceHasMediaUuid(deviceProperties) && mediaDevicePath.isEmpty())
                mediaDevicePath = path;
        }
    }

    const QString newDevicePath = !playerPath.isEmpty()
        ? devicePathFromPlayerPath(playerPath)
        : (!mediaDevicePath.isEmpty() ? mediaDevicePath : connectedDevicePath);
    const bool newConnected = mediaControlConnected || !playerPath.isEmpty();

    if (m_devicePath != newDevicePath) {
        m_devicePath = newDevicePath;
        if (!m_devicePath.isEmpty())
            watchProperties(m_devicePath);
    }

    const QString newDeviceAddress = deviceAddressFromPath(m_devicePath);
    if (m_deviceAddress != newDeviceAddress) {
        m_deviceAddress = newDeviceAddress;
        emit deviceAddressChanged(m_deviceAddress);
    }

    if (m_playerPath != playerPath) {
        m_playerPath = playerPath;
        if (!m_playerPath.isEmpty())
            watchProperties(m_playerPath);
    }

    if (m_connected != newConnected) {
        m_connected = newConnected;
        m_refreshTimer.start(m_connected ? 500 : 2000);
        emit connectedChanged(m_connected);
    }

    if (!m_connected) {
        if (m_status != QLatin1String("stopped")) {
            m_status = "stopped";
            m_refreshTimer.start(2000);
            emit statusChanged(m_status);
        }
        m_positionMs = 0;
        m_durationMs = 0;
        emitMetadataIfChanged(emptyMetadata());
    }

    return m_connected;
}

void BluetoothMediaPlayer::refreshPlayerProperties()
{
    if (m_playerPath.isEmpty())
        return;

    const QVariantMap properties = getAllProperties(m_playerPath, MediaPlayerInterface);
    if (!properties.isEmpty())
        updateMetadataFromProperties(properties);
}

void BluetoothMediaPlayer::updateMetadataFromProperties(const QVariantMap &properties)
{
    QVariantMap metadata;
    QVariantMap track;

    const QVariant trackVariant = properties.value("Track");
    if (trackVariant.isValid()) {
        const QVariantMap bluezTrack = mapFromVariant(trackVariant);

        const QString title = unwrapVariant(bluezTrack.value("Title")).toString();
        const QString artist = unwrapVariant(bluezTrack.value("Artist")).toString();
        const QString album = unwrapVariant(bluezTrack.value("Album")).toString();
        const int duration = unwrapVariant(bluezTrack.value("Duration")).toInt();
        m_durationMs = duration;

        track.insert("title", title);
        track.insert("artist", artist);
        track.insert("album", album);
        track.insert("duration", duration);
        metadata.insert("track", track);
    }

    const QVariant positionVariant = properties.value("Position");
    if (positionVariant.isValid()) {
        m_positionMs = unwrapVariant(positionVariant).toInt();
        metadata.insert("position", m_positionMs);
    }

    const QVariant statusVariant = properties.value("Status");
    if (statusVariant.isValid()) {
        const QString bluezStatus = unwrapVariant(statusVariant).toString().toLower();
        const QString status = bluezStatus == "playing" ? "playing"
            : (bluezStatus == "paused" ? "paused" : "stopped");
        metadata.insert("status", status);

        if (m_status != status) {
            if (status == QLatin1String("playing"))
                requestExternalMediaPause();

            m_status = status;
            m_refreshTimer.start(m_connected ? 500 : 2000);
            emit statusChanged(m_status);
        }
    }

    if (!metadata.isEmpty()) {
        metadata.insert("connected", m_connected);
        emitMetadataIfChanged(metadata);
    }
}

void BluetoothMediaPlayer::emitMetadataIfChanged(QVariantMap metadata)
{
    if (!metadata.contains("connected"))
        metadata.insert("connected", m_connected);

    if (metadata == m_lastMetadata)
        return;

    m_lastMetadata = metadata;
    emit metadataChanged(metadata);
}

QVariantMap BluetoothMediaPlayer::getAllProperties(const QString &path, const QString &interface) const
{
    QDBusInterface properties(BluezService,
                              path,
                              PropertiesInterface,
                              QDBusConnection::systemBus());
    if (!properties.isValid()) {
        qWarning() << "BlueZ Properties interface is unavailable for" << path
                   << properties.lastError().message();
        return {};
    }

    QDBusReply<QVariantMap> reply = properties.call("GetAll", interface);
    if (!reply.isValid()) {
        qWarning() << "Failed to query BlueZ properties for" << path << interface
                   << reply.error().message();
        return {};
    }

    return reply.value();
}

bool BluetoothMediaPlayer::callMethod(const QString &path,
                                      const QString &interface,
                                      const QString &method,
                                      const QVariantList &arguments,
                                      bool logErrors) const
{
    if (path.isEmpty())
        return false;

    QDBusMessage message = QDBusMessage::createMethodCall(BluezService, path, interface, method);
    message.setArguments(arguments);

    const QDBusMessage reply = QDBusConnection::systemBus().call(message, QDBus::Block, 5000);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        if (logErrors)
            qWarning() << "BlueZ call failed" << path << interface << method << reply.errorMessage();
        return false;
    }

    return true;
}

bool BluetoothMediaPlayer::setProperty(const QString &path,
                                       const QString &interface,
                                       const QString &name,
                                       const QVariant &value,
                                       bool logErrors) const
{
    if (path.isEmpty())
        return false;

    QDBusMessage message = QDBusMessage::createMethodCall(BluezService,
                                                          path,
                                                          PropertiesInterface,
                                                          "Set");
    message.setArguments({interface, name, QVariant::fromValue(QDBusVariant(value))});

    const QDBusMessage reply = QDBusConnection::systemBus().call(message, QDBus::Block, 5000);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        if (logErrors)
            qWarning() << "BlueZ property set failed" << path << interface << name << reply.errorMessage();
        return false;
    }

    return true;
}

bool BluetoothMediaPlayer::trySetPosition(int targetMs)
{
    if (m_playerPath.isEmpty())
        return false;

    return setProperty(m_playerPath,
                       MediaPlayerInterface,
                       "Position",
                       QVariant::fromValue(static_cast<quint32>(targetMs)),
                       false);
}

bool BluetoothMediaPlayer::startTimedSeek(bool forward, int targetMs, int holdMs)
{
    if (m_playerPath.isEmpty())
        return false;

    if (m_seekReleaseTimer.isActive())
        finishSeek();

    const uchar key = forward ? AvrcpFastForwardKey : AvrcpRewindKey;
    if (callMethod(m_playerPath, MediaPlayerInterface, "Hold", {QVariant::fromValue(key)}, false)) {
        m_seekStopMethod = QStringLiteral("Release");
    } else {
        const QString startMethod = forward ? QStringLiteral("FastForward") : QStringLiteral("Rewind");
        if (!callMethod(m_playerPath, MediaPlayerInterface, startMethod, {}, false))
            return false;

        m_seekStopMethod = m_status == QLatin1String("paused") ? QStringLiteral("Pause")
            : (m_status == QLatin1String("stopped") ? QStringLiteral("Stop") : QStringLiteral("Play"));
    }

    if (m_seekStopMethod.isEmpty())
        return false;

    m_pendingSeekTargetMs = targetMs;
    m_pendingSeekForward = forward;
    m_seekReleaseTimer.start(holdMs);
    m_seekPollTimer.start();
    return true;
}

bool BluetoothMediaPlayer::callDeviceProfile(const QString &method, const QString &uuid) const
{
    return callMethod(m_devicePath, DeviceInterface, method, {uuid});
}

void BluetoothMediaPlayer::callPlayerMethod(const QString &method)
{
    if (m_playerPath.isEmpty())
        refreshObjects();

    if (m_playerPath.isEmpty()) {
        qWarning() << "No Bluetooth media player found for" << method;
        return;
    }

    callMethod(m_playerPath, MediaPlayerInterface, method);
}

void BluetoothMediaPlayer::requestExternalMediaPause() const
{
    QDBusMessage message = QDBusMessage::createMethodCall(SocialService,
                                                          SocialObjectPath,
                                                          SocialInterface,
                                                          "Pause");
    const QDBusMessage reply = QDBusConnection::systemBus().call(message, QDBus::Block, 1000);
    if (reply.type() == QDBusMessage::ErrorMessage
        && reply.errorName() != QLatin1String("org.freedesktop.DBus.Error.ServiceUnknown")
        && reply.errorName() != QLatin1String("org.freedesktop.DBus.Error.NameHasNoOwner")) {
        qWarning() << "Failed to pause external media before Bluetooth playback"
                   << reply.errorName() << reply.errorMessage();
    }
}

void BluetoothMediaPlayer::watchProperties(const QString &path)
{
    if (path.isEmpty() || m_watchedPaths.contains(path))
        return;

    const bool ok = QDBusConnection::systemBus().connect(BluezService,
                                                        path,
                                                        PropertiesInterface,
                                                        "PropertiesChanged",
                                                        this,
                                                        SLOT(onPropertiesChanged(QString,QVariantMap,QStringList)));
    if (!ok)
        qWarning() << "Failed to subscribe to BlueZ property changes for" << path;
    else
        m_watchedPaths.insert(path);
}

QString BluetoothMediaPlayer::devicePathFromPlayerPath(const QString &playerPath) const
{
    const int index = playerPath.lastIndexOf('/');
    if (index <= 0)
        return {};
    return playerPath.left(index);
}

QString BluetoothMediaPlayer::deviceAddressFromPath(const QString &devicePath) const
{
    const int index = devicePath.lastIndexOf('/');
    const QString node = index >= 0 ? devicePath.mid(index + 1) : devicePath;
    if (node.isEmpty())
        return {};

    if (node.startsWith(QLatin1String("dev_"))) {
        QString address = node.mid(4).replace('_', ':');
        if (address.count(':') == 5)
            return address.toUpper();
    }

    return {};
}

bool BluetoothMediaPlayer::deviceHasMediaUuid(const QVariantMap &deviceProperties) const
{
    const QStringList uuids = stringListFromVariant(deviceProperties.value("UUIDs"));
    return uuids.contains(A2dpSourceUuid, Qt::CaseInsensitive)
        || uuids.contains(AvrcpRemoteControlUuid, Qt::CaseInsensitive);
}

QStringList BluetoothMediaPlayer::stringListFromVariant(const QVariant &value) const
{
    const QVariant unwrapped = unwrapVariant(value);
    if (unwrapped.canConvert<QStringList>())
        return unwrapped.toStringList();

    QStringList result;
    const QVariantList list = unwrapped.toList();
    for (const QVariant &item : list)
        result.append(unwrapVariant(item).toString());
    return result;
}

QVariantMap BluetoothMediaPlayer::mapFromVariant(const QVariant &value) const
{
    const QVariant unwrapped = unwrapVariant(value);
    if (unwrapped.canConvert<QVariantMap>())
        return unwrapped.toMap();

    if (unwrapped.canConvert<QDBusArgument>()) {
        QVariantMap result;
        const QDBusArgument argument = unwrapped.value<QDBusArgument>();
        argument >> result;
        return result;
    }

    return {};
}

QVariant BluetoothMediaPlayer::unwrapVariant(const QVariant &value) const
{
    if (value.canConvert<QDBusVariant>())
        return value.value<QDBusVariant>().variant();
    return value;
}

QVariantMap BluetoothMediaPlayer::emptyMetadata() const
{
    QVariantMap track;
    track.insert("title", "");
    track.insert("artist", "");
    track.insert("album", "");
    track.insert("duration", 0);

    QVariantMap metadata;
    metadata.insert("track", track);
    metadata.insert("position", 0);
    metadata.insert("status", "stopped");
    metadata.insert("connected", false);
    return metadata;
}
