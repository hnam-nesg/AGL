#ifndef BLUETOOTHMEDIAPLAYER_H
#define BLUETOOTHMEDIAPLAYER_H

#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QVariantMap>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusObjectPath>

using BluezInterfaceMap = QMap<QString, QVariantMap>;
using BluezManagedObjectMap = QMap<QDBusObjectPath, BluezInterfaceMap>;

Q_DECLARE_METATYPE(BluezInterfaceMap)
Q_DECLARE_METATYPE(BluezManagedObjectMap)

QDBusArgument &operator<<(QDBusArgument &argument, const BluezInterfaceMap &interfaces);
const QDBusArgument &operator>>(const QDBusArgument &argument, BluezInterfaceMap &interfaces);
QDBusArgument &operator<<(QDBusArgument &argument, const BluezManagedObjectMap &objects);
const QDBusArgument &operator>>(const QDBusArgument &argument, BluezManagedObjectMap &objects);

class BluetoothMediaPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit BluetoothMediaPlayer(QObject *parent = nullptr);

    bool connected() const;
    QString status() const;
    QString deviceAddress() const;

    Q_INVOKABLE void start();

    Q_INVOKABLE void connect();
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void connectBluetooth();
    Q_INVOKABLE void disconnectBluetooth();

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void next();
    Q_INVOKABLE void seek(int milliseconds);
    Q_INVOKABLE void refreshPosition();
    Q_INVOKABLE void fastforward(int milliseconds);
    Q_INVOKABLE void rewind(int milliseconds);
    Q_INVOKABLE void picktrack(int track);
    Q_INVOKABLE void volume(int volume);
    Q_INVOKABLE void loop(const QString &state);
    Q_INVOKABLE void shuffle(bool enabled);

signals:
    void metadataChanged(QVariantMap metadata);
    void connectedChanged(bool connected);
    void statusChanged(QString status);
    void deviceAddressChanged(QString deviceAddress);

private slots:
    void refresh();
    void onPropertiesChanged(const QString &interface,
                             const QVariantMap &changed,
                             const QStringList &invalidated);
    void finishSeek();
    void updateSeekProgress();

private:
    bool refreshObjects();
    void refreshPlayerProperties();
    void updateMetadataFromProperties(const QVariantMap &properties);
    void emitMetadataIfChanged(QVariantMap metadata);

    QVariantMap getAllProperties(const QString &path, const QString &interface) const;
    bool callMethod(const QString &path,
                    const QString &interface,
                    const QString &method,
                    const QVariantList &arguments = {},
                    bool logErrors = true) const;
    bool setProperty(const QString &path,
                     const QString &interface,
                     const QString &name,
                     const QVariant &value,
                     bool logErrors = true) const;
    bool trySetPosition(int targetMs);
    bool startTimedSeek(bool forward, int targetMs, int holdMs);
    bool callDeviceProfile(const QString &method, const QString &uuid) const;
    void callPlayerMethod(const QString &method);
    void requestExternalMediaPause() const;

    void watchProperties(const QString &path);
    QString devicePathFromPlayerPath(const QString &playerPath) const;
    QString deviceAddressFromPath(const QString &devicePath) const;
    bool deviceHasMediaUuid(const QVariantMap &deviceProperties) const;
    QStringList stringListFromVariant(const QVariant &value) const;
    QVariantMap mapFromVariant(const QVariant &value) const;
    QVariant unwrapVariant(const QVariant &value) const;
    QVariantMap emptyMetadata() const;

private:
    QTimer m_refreshTimer;
    QTimer m_seekReleaseTimer;
    QTimer m_seekPollTimer;
    QSet<QString> m_watchedPaths;

    QString m_devicePath;
    QString m_deviceAddress;
    QString m_playerPath;
    QString m_status = "stopped";
    int m_positionMs = 0;
    int m_durationMs = 0;
    int m_pendingSeekTargetMs = -1;
    bool m_pendingSeekForward = true;
    bool m_connected = false;
    bool m_started = false;
    QString m_seekStopMethod;

    QVariantMap m_lastMetadata;
};

#endif // BLUETOOTHMEDIAPLAYER_H
