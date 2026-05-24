#include "bluetoothmanager.h"
#include "bluetoothworker.h"

#include <QDebug>
#include <QMetaType>

BluetoothManager::BluetoothManager(QQmlContext *context, QObject *parent)
    : QObject(parent)
    , m_workerThread(new QThread(this))
    , m_worker(new BluetoothWorker())
    , m_pairedModel(new BluetoothDeviceModel(BluetoothDeviceModel::PairedDevices, this))
    , m_discoveryModel(new BluetoothDeviceModel(BluetoothDeviceModel::DiscoveryDevices, this))
    , m_discovering(false)
    , m_power(false)
    , m_discoverable(false)
    , m_started(false)
    , m_backendReady(false)
{
    qDebug() << "[BluetoothManager] created in thread:" << QThread::currentThread();

    qRegisterMetaType<BluetoothDeviceInfo>("BluetoothDeviceInfo");
    qRegisterMetaType<QList<BluetoothDeviceInfo>>("QList<BluetoothDeviceInfo>");

    context->setContextProperty("BluetoothPairedModel", m_pairedModel);
    context->setContextProperty("BluetoothDiscoveryModel", m_discoveryModel);

    m_worker->moveToThread(m_workerThread);

    QObject::connect(m_workerThread, &QThread::finished,
                     m_worker, &QObject::deleteLater);

    QObject::connect(this, &BluetoothManager::requestStart,
                     m_worker, &BluetoothWorker::start,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestRefreshDevices,
                     m_worker, &BluetoothWorker::refreshDevices,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestSetPower,
                     m_worker, &BluetoothWorker::setPower,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestSetDiscoverable,
                     m_worker, &BluetoothWorker::setDiscoverable,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestStartDiscovery,
                     m_worker, &BluetoothWorker::startDiscovery,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestStopDiscovery,
                     m_worker, &BluetoothWorker::stopDiscovery,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestPairDevice,
                     m_worker, &BluetoothWorker::pairDevice,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestConnectDevice,
                     m_worker, &BluetoothWorker::connectDevice,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestDisconnectDevice,
                     m_worker, &BluetoothWorker::disconnectDevice,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestRemoveDevice,
                     m_worker, &BluetoothWorker::removeDevice,
                     Qt::QueuedConnection);

    QObject::connect(this, &BluetoothManager::requestSendConfirmation,
                     m_worker, &BluetoothWorker::sendConfirmation,
                     Qt::QueuedConnection);

    QObject::connect(m_worker, &BluetoothWorker::initialized,
                     this, &BluetoothManager::onWorkerInitialized,
                     Qt::QueuedConnection);

    QObject::connect(m_worker, &BluetoothWorker::devicesChanged,
                     this, &BluetoothManager::onWorkerDevicesChanged,
                     Qt::QueuedConnection);

    QObject::connect(m_worker, &BluetoothWorker::deviceUpdated,
                     this, &BluetoothManager::onWorkerDeviceUpdated,
                     Qt::QueuedConnection);

    QObject::connect(m_worker, &BluetoothWorker::deviceRemoved,
                     this, &BluetoothManager::onWorkerDeviceRemoved,
                     Qt::QueuedConnection);

    QObject::connect(m_worker, &BluetoothWorker::powerChanged,
                     this, &BluetoothManager::onWorkerPowerChanged,
                     Qt::QueuedConnection);

    QObject::connect(m_worker, &BluetoothWorker::discoverableChanged,
                     this, &BluetoothManager::onWorkerDiscoverableChanged,
                     Qt::QueuedConnection);

    QObject::connect(m_worker, &BluetoothWorker::discoveryChanged,
                     this, &BluetoothManager::onWorkerDiscoveryChanged,
                     Qt::QueuedConnection);

    QObject::connect(m_worker, &BluetoothWorker::requestConfirmationEvent,
                     this, &BluetoothManager::onWorkerRequestConfirmationEvent,
                     Qt::QueuedConnection);

    QObject::connect(m_worker, &BluetoothWorker::errorOccurred,
                     this, &BluetoothManager::onWorkerError,
                     Qt::QueuedConnection);

    m_workerThread->start();

    qDebug() << "[BluetoothManager] worker thread started:" << m_workerThread;

    start();
}

BluetoothManager::~BluetoothManager()
{
    qDebug() << "[BluetoothManager] shutting down worker thread";

    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }

    qDebug() << "[BluetoothManager] worker thread stopped";
}

bool BluetoothManager::discovering() const
{
    return m_discovering;
}

bool BluetoothManager::power() const
{
    return m_power;
}

bool BluetoothManager::discoverable() const
{
    return m_discoverable;
}

bool BluetoothManager::ready() const
{
    return m_backendReady;
}

void BluetoothManager::start()
{
    qDebug() << "[BluetoothManager] start() called";

    if (m_started) {
        qDebug() << "[BluetoothManager] start() ignored: already started";
        return;
    }

    m_started = true;
    emit requestStart();
}

void BluetoothManager::refresh_devices()
{
    qDebug() << "[BluetoothManager] refresh_devices() called";

    if (!m_backendReady)
        return;

    emit requestRefreshDevices();
}

void BluetoothManager::start_discovery()
{
    qDebug() << "[BluetoothManager] start_discovery() called";

    if (!m_backendReady) {
        emit errorOccurred("Cannot start discovery: Bluetooth backend not started");
        return;
    }

    if (m_discovering) {
        qDebug() << "[BluetoothManager] discovery already active";
        return;
    }

    emit requestStartDiscovery();
}

void BluetoothManager::stop_discovery()
{
    qDebug() << "[BluetoothManager] stop_discovery() called";

    if (!m_backendReady) {
        qDebug() << "[BluetoothManager] stop_discovery ignored: backend not started";
        return;
    }

    emit requestStopDiscovery();
}

void BluetoothManager::pair(QString id)
{
    qDebug() << "[BluetoothManager] pair() called, id =" << id;

    if (id.trimmed().isEmpty()) {
        emit errorOccurred("Cannot pair: empty bluetooth device id");
        return;
    }

    if (!m_backendReady) {
        emit errorOccurred("Cannot pair: Bluetooth backend not started");
        return;
    }

    emit requestPairDevice(id);
}

void BluetoothManager::connect(QString id)
{
    qDebug() << "[BluetoothManager] connect() called, id =" << id;

    if (id.trimmed().isEmpty()) {
        emit errorOccurred("Cannot connect: empty bluetooth device id");
        return;
    }

    if (!m_backendReady) {
        emit errorOccurred("Cannot connect: Bluetooth backend not started");
        return;
    }

    emit requestConnectDevice(id);
}

void BluetoothManager::disconnect(QString id)
{
    qDebug() << "[BluetoothManager] disconnect() called, id =" << id;

    if (id.trimmed().isEmpty()) {
        emit errorOccurred("Cannot disconnect: empty bluetooth device id");
        return;
    }

    if (!m_backendReady) {
        emit errorOccurred("Cannot disconnect: Bluetooth backend not started");
        return;
    }

    emit requestDisconnectDevice(id);
}

void BluetoothManager::remove(QString id)
{
    qDebug() << "[BluetoothManager] remove() called, id =" << id;

    if (id.trimmed().isEmpty()) {
        emit errorOccurred("Cannot remove: empty bluetooth device id");
        return;
    }

    stop_discovery();
    emit requestRemoveDevice(id);
}

void BluetoothManager::setPower(bool state)
{
    qDebug() << "[BluetoothManager] setPower() called, state =" << state;

    if (m_power == state) {
        qDebug() << "[BluetoothManager] power already =" << state;
        return;
    }

    if (!m_backendReady) {
        emit errorOccurred("Cannot set power: Bluetooth backend not started");
        return;
    }

    emit requestSetPower(state);
}

void BluetoothManager::setDiscoverable(bool state)
{
    qDebug() << "[BluetoothManager] setDiscoverable() called, state =" << state;

    if (m_discoverable == state) {
        qDebug() << "[BluetoothManager] discoverable already =" << state;
        return;
    }

    if (!m_backendReady) {
        emit errorOccurred("Cannot set discoverable: Bluetooth backend not started");
        return;
    }

    emit requestSetDiscoverable(state);
}

void BluetoothManager::send_confirmation(int pincode)
{
    qDebug() << "[BluetoothManager] send_confirmation() called, pincode =" << pincode;

    if (!m_backendReady) {
        emit errorOccurred("Cannot send confirmation: Bluetooth backend not started");
        return;
    }

    emit requestSendConfirmation(pincode);
}

void BluetoothManager::onWorkerInitialized(bool ok)
{
    qDebug() << "[BluetoothManager] onWorkerInitialized:" << ok;

    m_backendReady = ok;
    emit readyChanged();

    if (!ok)
        emit errorOccurred("Bluetooth backend initialization failed");
}

void BluetoothManager::onWorkerDevicesChanged(const QList<BluetoothDeviceInfo> &devices)
{
    qDebug() << "[BluetoothManager] onWorkerDevicesChanged count =" << devices.count();

    m_pairedModel->setDevices(devices);
    m_discoveryModel->setDevices(devices);
}

void BluetoothManager::onWorkerDeviceUpdated(const BluetoothDeviceInfo &device)
{
    qDebug() << "[BluetoothManager] onWorkerDeviceUpdated id =" << device.id
             << "paired =" << device.paired
             << "connected =" << device.connected;

    m_pairedModel->upsertDevice(device);
    m_discoveryModel->upsertDevice(device);
}

void BluetoothManager::onWorkerDeviceRemoved(const QString &id)
{
    qDebug() << "[BluetoothManager] onWorkerDeviceRemoved id =" << id;

    m_pairedModel->removeDevice(id);
    m_discoveryModel->removeDevice(id);
}

void BluetoothManager::onWorkerPowerChanged(bool state)
{
    qDebug() << "[BluetoothManager] onWorkerPowerChanged:" << state;

    if (m_power == state)
        return;

    m_power = state;
    emit powerChanged(state);

    if (!state) {
        m_pairedModel->clear();
        m_discoveryModel->clear();
    } else {
        emit requestRefreshDevices();
    }
}

void BluetoothManager::onWorkerDiscoverableChanged(bool state)
{
    qDebug() << "[BluetoothManager] onWorkerDiscoverableChanged:" << state;

    if (m_discoverable == state)
        return;

    m_discoverable = state;
    emit discoverableChanged(state);
}

void BluetoothManager::onWorkerDiscoveryChanged(bool state)
{
    qDebug() << "[BluetoothManager] onWorkerDiscoveryChanged:" << state;

    if (m_discovering == state)
        return;

    m_discovering = state;
    emit discoveringChanged();
}

void BluetoothManager::onWorkerRequestConfirmationEvent(QString pincode)
{
    qDebug() << "[BluetoothManager] onWorkerRequestConfirmationEvent:" << pincode;
    emit requestConfirmationEvent(pincode);
}

void BluetoothManager::onWorkerError(const QString &message)
{
    qDebug() << "[BluetoothManager] worker error:" << message;
    emit errorOccurred(message);
}
