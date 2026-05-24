#include "mediadbusservice.h"

#include "audiocontroller.h"
#include "bluetoothmediaplayer.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QtGlobal>

namespace {
constexpr auto kServiceName = "org.agl.media";
constexpr auto kObjectPath = "/org/agl/media";
constexpr int kVolumeStep = 10;
}

MediaDbusService::MediaDbusService(BluetoothMediaPlayer *player,
                                   AudioController *audio,
                                   QObject *parent)
    : QObject(parent)
    , m_player(player)
    , m_audio(audio)
{
    if (m_player) {
        QObject::connect(m_player, &BluetoothMediaPlayer::metadataChanged,
                         this, [this](const QVariantMap &metadata) {
            emit MetadataChanged(metadata);
            emitStateChanged();
        });
        QObject::connect(m_player, &BluetoothMediaPlayer::connectedChanged,
                         this, &MediaDbusService::emitStateChanged);
        QObject::connect(m_player, &BluetoothMediaPlayer::statusChanged,
                         this, &MediaDbusService::emitStateChanged);
    }

    if (m_audio)
        QObject::connect(m_audio, &AudioController::volumeChanged,
                         this, &MediaDbusService::emitStateChanged);
    if (m_audio)
        QObject::connect(m_audio, &AudioController::mutedChanged,
                         this, &MediaDbusService::emitStateChanged);
}

bool MediaDbusService::registerService()
{
    const bool systemOk = registerOnConnection(QStringLiteral("system"));
    const bool sessionOk = registerOnConnection(QStringLiteral("session"));

    if (!systemOk && !sessionOk) {
        emitError(QStringLiteral("Cannot register org.agl.media on system or session D-Bus"));
        return false;
    }

    return true;
}

QString MediaDbusService::Ping() const
{
    return QString::fromLatin1(kServiceName);
}

QVariantMap MediaDbusService::State() const
{
    QVariantMap state;
    state.insert(QStringLiteral("service"), QString::fromLatin1(kServiceName));
    state.insert(QStringLiteral("connected"), m_player ? m_player->connected() : false);
    state.insert(QStringLiteral("status"), m_player ? m_player->status() : QStringLiteral("unavailable"));
    state.insert(QStringLiteral("volume"), m_audio ? qRound(m_audio->volume() * 100.0) : -1);
    state.insert(QStringLiteral("muted"), m_audio ? m_audio->muted() : false);
    return state;
}

bool MediaDbusService::Play()
{
    if (!ensurePlayerReady(QStringLiteral("Play")))
        return false;

    emit CommandReceived(QStringLiteral("Play"));
    m_player->play();
    return true;
}

bool MediaDbusService::Pause()
{
    if (!ensurePlayerReady(QStringLiteral("Pause")))
        return false;

    emit CommandReceived(QStringLiteral("Pause"));
    m_player->pause();
    return true;
}

bool MediaDbusService::Stop()
{
    if (!ensurePlayerReady(QStringLiteral("Stop")))
        return false;

    emit CommandReceived(QStringLiteral("Stop"));
    m_player->stop();
    return true;
}

bool MediaDbusService::TogglePlayPause()
{
    if (!m_player)
        return ensurePlayerReady(QStringLiteral("TogglePlayPause"));

    return m_player->status() == QLatin1String("playing") ? Pause() : Play();
}

bool MediaDbusService::Next()
{
    if (!ensurePlayerReady(QStringLiteral("Next")))
        return false;

    emit CommandReceived(QStringLiteral("Next"));
    m_player->next();
    return true;
}

bool MediaDbusService::Previous()
{
    if (!ensurePlayerReady(QStringLiteral("Previous")))
        return false;

    emit CommandReceived(QStringLiteral("Previous"));
    m_player->previous();
    return true;
}

bool MediaDbusService::Seek(int milliseconds)
{
    if (!ensurePlayerReady(QStringLiteral("Seek")))
        return false;

    if (milliseconds < 0) {
        emitError(QStringLiteral("Seek milliseconds must be >= 0"));
        return false;
    }

    emit CommandReceived(QStringLiteral("Seek"));
    m_player->seek(milliseconds);
    return true;
}

bool MediaDbusService::SetVolume(int percent)
{
    return setVolumePercent(percent);
}

bool MediaDbusService::VolumeUp()
{
    if (!ensureAudioReady(QStringLiteral("VolumeUp")))
        return false;

    return setVolumePercent(qRound(m_audio->volume() * 100.0) + kVolumeStep);
}

bool MediaDbusService::VolumeDown()
{
    if (!ensureAudioReady(QStringLiteral("VolumeDown")))
        return false;

    return setVolumePercent(qRound(m_audio->volume() * 100.0) - kVolumeStep);
}

bool MediaDbusService::Mute()
{
    return SetMute(true);
}

bool MediaDbusService::Unmute()
{
    return SetMute(false);
}

bool MediaDbusService::SetMute(bool mute)
{
    if (!ensureAudioReady(QStringLiteral("SetMute")))
        return false;

    emit CommandReceived(mute ? QStringLiteral("Mute") : QStringLiteral("Unmute"));
    m_audio->setVolumeMute(mute);
    return true;
}

bool MediaDbusService::ConnectBluetooth()
{
    if (!m_player) {
        emitError(QStringLiteral("Bluetooth media backend is unavailable"));
        return false;
    }

    emit CommandReceived(QStringLiteral("ConnectBluetooth"));
    m_player->connectBluetooth();
    return true;
}

bool MediaDbusService::DisconnectBluetooth()
{
    if (!m_player) {
        emitError(QStringLiteral("Bluetooth media backend is unavailable"));
        return false;
    }

    emit CommandReceived(QStringLiteral("DisconnectBluetooth"));
    m_player->disconnectBluetooth();
    return true;
}

bool MediaDbusService::registerOnConnection(const QString &busName)
{
    QDBusConnection connection = busName == QLatin1String("system")
        ? QDBusConnection::systemBus()
        : QDBusConnection::sessionBus();

    if (!connection.isConnected()) {
        qWarning() << "D-Bus" << busName << "bus is unavailable:"
                   << connection.lastError().message();
        return false;
    }

    if (!connection.registerService(QString::fromLatin1(kServiceName))) {
        qWarning() << "Cannot register D-Bus service" << kServiceName
                   << "on" << busName << "bus:"
                   << connection.lastError().message();
        return false;
    }

    const bool objectOk = connection.registerObject(
        QString::fromLatin1(kObjectPath),
        this,
        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);

    if (!objectOk) {
        qWarning() << "Cannot register D-Bus object" << kObjectPath
                   << "on" << busName << "bus:"
                   << connection.lastError().message();
        connection.unregisterService(QString::fromLatin1(kServiceName));
        return false;
    }

    qInfo() << "D-Bus media service is ready on" << busName << "bus:"
            << kServiceName << kObjectPath;
    return true;
}

bool MediaDbusService::ensurePlayerReady(const QString &command)
{
    if (!m_player) {
        emitError(QStringLiteral("Bluetooth media backend is unavailable"));
        return false;
    }

    m_player->start();

    if (!m_player->connected()) {
        emitError(command + QStringLiteral(" failed: no Bluetooth media player connected"));
        return false;
    }

    return true;
}

bool MediaDbusService::ensureAudioReady(const QString &command)
{
    if (!m_audio) {
        emitError(command + QStringLiteral(" failed: audio backend is unavailable"));
        return false;
    }

    return true;
}

bool MediaDbusService::setVolumePercent(int percent)
{
    if (!ensureAudioReady(QStringLiteral("SetVolume")))
        return false;

    const int normalized = qBound(0, percent, 100);
    emit CommandReceived(QStringLiteral("SetVolume"));
    m_audio->setVolume(normalized / 100.0);
    return true;
}

void MediaDbusService::emitError(const QString &message)
{
    qWarning() << "org.agl.media:" << message;
    emit Error(message);
}

void MediaDbusService::emitStateChanged()
{
    emit StateChanged(State());
}
