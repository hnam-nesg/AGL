#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include "bluetoothdevicemodel.h"

#include <QObject>
#include <QThread>
#include <QQmlContext>

class BluetoothWorker;

class BluetoothManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool discovering READ discovering NOTIFY discoveringChanged)
    Q_PROPERTY(bool power READ power WRITE setPower NOTIFY powerChanged)
    Q_PROPERTY(bool discoverable READ discoverable WRITE setDiscoverable NOTIFY discoverableChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

public:
    explicit BluetoothManager(QQmlContext *context, QObject *parent = nullptr);
    ~BluetoothManager() override;

    bool discovering() const;
    bool power() const;
    bool discoverable() const;
    bool ready() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void refresh_devices();

    Q_INVOKABLE void start_discovery();
    Q_INVOKABLE void stop_discovery();

    Q_INVOKABLE void pair(QString id);
    Q_INVOKABLE void connect(QString id);
    Q_INVOKABLE void disconnect(QString id);

    /*
     * Remove / Forget / Unpair device.
     * QML gọi bluetooth.remove(devId).
     * Backend sẽ chạy bluetoothctl remove <MAC>.
     */
    Q_INVOKABLE void remove(QString id);

    Q_INVOKABLE void setPower(bool state);
    Q_INVOKABLE void setDiscoverable(bool state);
    Q_INVOKABLE void send_confirmation(int pincode);

signals:
    void requestStart();
    void requestRefreshDevices();
    void requestSetPower(bool state);
    void requestSetDiscoverable(bool state);
    void requestStartDiscovery();
    void requestStopDiscovery();
    void requestPairDevice(QString id);
    void requestConnectDevice(QString id);
    void requestDisconnectDevice(QString id);
    void requestRemoveDevice(QString id);
    void requestSendConfirmation(int pincode);

    void discoveringChanged();
    void powerChanged(bool state);
    void discoverableChanged(bool state);
    void readyChanged();
    void errorOccurred(const QString &message);

    void requestConfirmationEvent(QString pincode);

private slots:
    void onWorkerInitialized(bool ok);
    void onWorkerDevicesChanged(const QList<BluetoothDeviceInfo> &devices);
    void onWorkerDeviceUpdated(const BluetoothDeviceInfo &device);
    void onWorkerDeviceRemoved(const QString &id);
    void onWorkerError(const QString &message);
    void onWorkerPowerChanged(bool state);
    void onWorkerDiscoverableChanged(bool state);
    void onWorkerDiscoveryChanged(bool state);
    void onWorkerRequestConfirmationEvent(QString pincode);

private:
    QThread *m_workerThread;
    BluetoothWorker *m_worker;
    BluetoothDeviceModel *m_pairedModel;
    BluetoothDeviceModel *m_discoveryModel;

    bool m_discovering;
    bool m_power;
    bool m_discoverable;
    bool m_started;
    bool m_backendReady;
};

#endif // BLUETOOTHMANAGER_H
