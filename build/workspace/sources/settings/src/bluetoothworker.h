#ifndef BLUETOOTHWORKER_H
#define BLUETOOTHWORKER_H

#include "bluetoothdevicemodel.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include <bluez-glib.h>

class BluetoothWorker : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothWorker(QObject *parent = nullptr);
    ~BluetoothWorker() override;

public slots:
    void start();
    void refreshDevices();
    void setPower(bool state);
    void setDiscoverable(bool state);
    void startDiscovery();
    void stopDiscovery();
    void pairDevice(QString id);
    void connectDevice(QString id);
    void disconnectDevice(QString id);
    void removeDevice(QString id);
    void sendConfirmation(int pincode);

signals:
    void initialized(bool ok);
    void devicesChanged(QList<BluetoothDeviceInfo> devices);
    void deviceUpdated(BluetoothDeviceInfo device);
    void deviceRemoved(QString id);
    void powerChanged(bool state);
    void discoverableChanged(bool state);
    void discoveryChanged(bool state);
    void requestConfirmationEvent(QString pincode);
    void errorOccurred(const QString &message);

private:
    static void initCallback(gchar *adapter, gboolean status, gpointer userData);
    static void adapterEventCallback(gchar *adapter,
                                     bluez_event_t event,
                                     GVariant *properties,
                                     gpointer userData);
    static void deviceEventCallback(gchar *adapter,
                                    gchar *device,
                                    bluez_event_t event,
                                    GVariant *properties,
                                    gpointer userData);
    static void agentEventCallback(gchar *device,
                                   bluez_agent_event_t event,
                                   GVariant *properties,
                                   gpointer userData);
    static void deviceConnectCallback(gchar *device, gboolean status, gpointer userData);

    void handleInit(gchar *adapter, gboolean status);
    void handleAdapterEvent(gchar *adapter, bluez_event_t event, GVariant *properties);
    void handleDeviceEvent(gchar *adapter, gchar *device, bluez_event_t event, GVariant *properties);
    void handleAgentEvent(gchar *device, bluez_agent_event_t event, int pincode);
    void handleConnectResult(gchar *device, gboolean status);

    QList<BluetoothDeviceInfo> readDeviceList();
    BluetoothDeviceInfo deviceFromVariant(const QString &id,
                                          GVariant *properties,
                                          const BluetoothDeviceInfo &base = BluetoothDeviceInfo()) const;
    QString normalizeDeviceId(QString id) const;
    QString deviceIdToAddress(QString id) const;
    QString deviceIdToBluezCardName(QString id) const;
    QString connectedDeviceId() const;
    QString connectedDeviceId(const QList<BluetoothDeviceInfo> &devices) const;
    bool hasOtherConnectedDevice(const QList<BluetoothDeviceInfo> &devices,
                                 const QString &deviceId,
                                 QString *connectedDeviceId = nullptr) const;
    void disconnectDeviceSilently(const QString &deviceId);
    void enforceSingleConnectedDevice(const QString &preferredDeviceId = QString());

    bool runShellCommand(const QString &command, QString *stdOut = nullptr, QString *stdErr = nullptr) const;

    void trustDevice(QString id, bool reportErrors = true);
    void forceA2dpSink(QString id);
    void setDiscoveryFilter();

    bool m_started;
    bool m_power;
    bool m_discoverable;
    bool m_discovering;
    QString m_pendingConnectDeviceId;
    QString m_activeConnectedDeviceId;
    QHash<QString, BluetoothDeviceInfo> m_devices;
};

#endif // BLUETOOTHWORKER_H
