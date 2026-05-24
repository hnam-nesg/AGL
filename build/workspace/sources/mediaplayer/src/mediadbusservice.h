#ifndef MEDIADBUSSERVICE_H
#define MEDIADBUSSERVICE_H

#include <QObject>
#include <QVariantMap>

class AudioController;
class BluetoothMediaPlayer;

class MediaDbusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.agl.media")

public:
    explicit MediaDbusService(BluetoothMediaPlayer *player,
                              AudioController *audio,
                              QObject *parent = nullptr);

    bool registerService();

public slots:
    Q_SCRIPTABLE QString Ping() const;
    Q_SCRIPTABLE QVariantMap State() const;

    Q_SCRIPTABLE bool Play();
    Q_SCRIPTABLE bool Pause();
    Q_SCRIPTABLE bool Stop();
    Q_SCRIPTABLE bool TogglePlayPause();
    Q_SCRIPTABLE bool Next();
    Q_SCRIPTABLE bool Previous();
    Q_SCRIPTABLE bool Seek(int milliseconds);

    Q_SCRIPTABLE bool SetVolume(int percent);
    Q_SCRIPTABLE bool VolumeUp();
    Q_SCRIPTABLE bool VolumeDown();
    Q_SCRIPTABLE bool Mute();
    Q_SCRIPTABLE bool Unmute();
    Q_SCRIPTABLE bool SetMute(bool mute);

    Q_SCRIPTABLE bool ConnectBluetooth();
    Q_SCRIPTABLE bool DisconnectBluetooth();

signals:
    Q_SCRIPTABLE void CommandReceived(const QString &command);
    Q_SCRIPTABLE void Error(const QString &message);
    Q_SCRIPTABLE void StateChanged(const QVariantMap &state);
    Q_SCRIPTABLE void MetadataChanged(const QVariantMap &metadata);

private:
    bool registerOnConnection(const QString &busName);
    bool ensurePlayerReady(const QString &command);
    bool ensureAudioReady(const QString &command);
    bool setVolumePercent(int percent);
    void emitError(const QString &message);
    void emitStateChanged();

private:
    BluetoothMediaPlayer *m_player = nullptr;
    AudioController *m_audio = nullptr;
};

#endif // MEDIADBUSSERVICE_H
