#ifndef TELEPHONY_H
#define TELEPHONY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>
#include <QDBusObjectPath>
#include <QDBusVariant>

class Telephony : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)
    Q_PROPERTY(QString callState READ callState NOTIFY callStateChanged)
    Q_PROPERTY(QString callClip READ callClip NOTIFY callClipChanged)
    Q_PROPERTY(QString callColp READ callColp NOTIFY callColpChanged)
    Q_PROPERTY(QString bluetoothAddress READ bluetoothAddress NOTIFY bluetoothAddressChanged)

public:
    explicit Telephony(QObject *parent = nullptr);
    ~Telephony() override;

    bool connected() const { return m_connected; }
    bool online() const { return m_online; }

    QString callState() const { return m_call_state; }
    QString callClip() const { return m_clip; }
    QString callColp() const { return m_colp; }
    QString bluetoothAddress() const { return m_bluetoothAddress; }

    Q_INVOKABLE bool autoSelectBluetoothDevice();
    Q_INVOKABLE void setBluetoothDevice(QString deviceIdOrAddress);
    Q_INVOKABLE bool dial(QString number);
    Q_INVOKABLE void answer();
    Q_INVOKABLE void hangup();

signals:
    void connectedChanged(bool state);
    void onlineChanged(bool state);
    void callStateChanged(QString state);
    void callClipChanged(QString clip);
    void callColpChanged(QString colp);
    void bluetoothAddressChanged(QString address);
    void errorOccurred(QString message);

private slots:
    void onCallAdded(const QDBusObjectPath &path, const QVariantMap &properties);
    void onCallRemoved(const QDBusObjectPath &path);
    void onVoiceCallPropertyChanged(const QString &name, const QDBusVariant &value);
    void onOfonoModemAdded(const QDBusObjectPath &path, const QVariantMap &properties);
    void onOfonoModemRemoved(const QDBusObjectPath &path);
    void onBluezPropertiesChanged(const QString &interfaceName,
                                  const QVariantMap &changedProperties,
                                  const QStringList &invalidatedProperties);
    void onBluezInterfacesAdded(const QDBusObjectPath &path,
                                const QVariantMap &interfaces);
    void onBluezInterfacesRemoved(const QDBusObjectPath &path,
                                  const QStringList &interfaces);
    void refreshBluetoothDeviceSelection();

private:
    void setConnected(bool state);
    void setOnlineState(bool state);
    void setCallState(const QString &callState);
    void setCallClip(const QString &clip);
    void setCallColp(const QString &colp);

    QString normalizeBluetoothAddress(QString deviceIdOrAddress) const;
    QString bluetoothAddressToBluezDeviceName(QString address) const;
    QString bluetoothAddressToOfonoHfpPath(QString address) const;

    bool autoSelectBluetoothDeviceInternal(bool emitErrors);
    bool selectedBluetoothDeviceStillConnected() const;
    void clearSelectedBluetoothDevice(const QString &reason);
    void connectDeviceChangeSignals();

    void disconnectVoiceCallManagerSignals();
    void connectVoiceCallManagerSignals();
    void disconnectCurrentVoiceCallSignals();
    void connectCurrentVoiceCallSignals();
    void applyVoiceCallState(const QString &state,
                             const QString &lineId = QString(),
                             const QString &name = QString());

    bool validateVoiceCallManagerPath();

private:
    QString m_bluetoothAddress;
    QString m_voiceCallManagerPath;
    QString m_currentCallPath;

    bool m_connected = false;
    bool m_online = false;
    QTimer m_deviceRefreshTimer;

    QString m_call_state;
    QString m_clip;
    QString m_colp;
};

#endif // TELEPHONY_H
