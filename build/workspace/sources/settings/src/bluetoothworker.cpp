#include "bluetoothworker.h"

#include <QDebug>
#include <QMetaObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QThread>
#include <QTimer>

BluetoothWorker::BluetoothWorker(QObject *parent)
    : QObject(parent)
    , m_started(false)
    , m_power(false)
    , m_discoverable(false)
    , m_discovering(false)
    , m_pendingConnectDeviceId()
    , m_activeConnectedDeviceId()
{
    qDebug() << "[BluetoothWorker] created in thread:" << QThread::currentThread();
}

BluetoothWorker::~BluetoothWorker()
{
    qDebug() << "[BluetoothWorker] destroyed in thread:" << QThread::currentThread();
}

void BluetoothWorker::initCallback(gchar *adapter, gboolean status, gpointer userData)
{
    BluetoothWorker *worker = static_cast<BluetoothWorker *>(userData);
    if (!worker)
        return;

    QMetaObject::invokeMethod(worker,
                              [worker, adapterName = QString::fromUtf8(adapter ? adapter : ""), status]() {
                                  QByteArray adapterBytes = adapterName.toUtf8();
                                  worker->handleInit(adapterBytes.data(), status);
                              },
                              Qt::QueuedConnection);
}

void BluetoothWorker::adapterEventCallback(gchar *adapter,
                                           bluez_event_t event,
                                           GVariant *properties,
                                           gpointer userData)
{
    BluetoothWorker *worker = static_cast<BluetoothWorker *>(userData);
    if (!worker)
        return;

    GVariant *propertiesRef = properties ? g_variant_ref(properties) : nullptr;
    QMetaObject::invokeMethod(worker,
                              [worker, adapterName = QString::fromUtf8(adapter ? adapter : ""), event, propertiesRef]() {
                                  QByteArray adapterBytes = adapterName.toUtf8();
                                  worker->handleAdapterEvent(adapterBytes.data(), event, propertiesRef);
                                  if (propertiesRef)
                                      g_variant_unref(propertiesRef);
                              },
                              Qt::QueuedConnection);
}

void BluetoothWorker::deviceEventCallback(gchar *adapter,
                                          gchar *device,
                                          bluez_event_t event,
                                          GVariant *properties,
                                          gpointer userData)
{
    BluetoothWorker *worker = static_cast<BluetoothWorker *>(userData);
    if (!worker)
        return;

    GVariant *propertiesRef = properties ? g_variant_ref(properties) : nullptr;
    QMetaObject::invokeMethod(worker,
                              [worker,
                               adapterName = QString::fromUtf8(adapter ? adapter : ""),
                               deviceName = QString::fromUtf8(device ? device : ""),
                               event,
                               propertiesRef]() {
                                  QByteArray adapterBytes = adapterName.toUtf8();
                                  QByteArray deviceBytes = deviceName.toUtf8();
                                  worker->handleDeviceEvent(adapterBytes.data(), deviceBytes.data(), event, propertiesRef);
                                  if (propertiesRef)
                                      g_variant_unref(propertiesRef);
                              },
                              Qt::QueuedConnection);
}

void BluetoothWorker::agentEventCallback(gchar *device,
                                         bluez_agent_event_t event,
                                         GVariant *properties,
                                         gpointer userData)
{
    BluetoothWorker *worker = static_cast<BluetoothWorker *>(userData);
    if (!worker)
        return;

    int pincode = 0;
    if (event == BLUEZ_AGENT_EVENT_REQUEST_CONFIRMATION && properties) {
        const gchar *path = nullptr;
        g_variant_get(properties, "(ou)", &path, &pincode);
    }

    QMetaObject::invokeMethod(worker,
                              [worker, deviceName = QString::fromUtf8(device ? device : ""), event, pincode]() {
                                  QByteArray deviceBytes = deviceName.toUtf8();
                                  worker->handleAgentEvent(deviceBytes.data(), event, pincode);
                              },
                              Qt::QueuedConnection);
}

void BluetoothWorker::deviceConnectCallback(gchar *device, gboolean status, gpointer userData)
{
    BluetoothWorker *worker = static_cast<BluetoothWorker *>(userData);
    if (!worker)
        return;

    QMetaObject::invokeMethod(worker,
                              [worker, deviceName = QString::fromUtf8(device ? device : ""), status]() {
                                  QByteArray deviceBytes = deviceName.toUtf8();
                                  worker->handleConnectResult(deviceBytes.data(), status);
                              },
                              Qt::QueuedConnection);
}

void BluetoothWorker::handleInit(gchar *adapter, gboolean status)
{
    qDebug() << "[BluetoothWorker] handleInit adapter =" << (adapter ? adapter : "")
             << "status =" << status
             << "thread =" << QThread::currentThread();

    if (!status) {
        emit initialized(false);
        emit errorOccurred("BlueZ initialization failed");
        return;
    }

    GVariant *reply = nullptr;
    if (bluez_adapter_get_state(nullptr, &reply) && reply) {
        GVariantDict *propsDict = g_variant_dict_new(reply);
        gboolean powered = FALSE;
        gboolean discoverable = FALSE;

        if (g_variant_dict_lookup(propsDict, "Powered", "b", &powered)) {
            m_power = powered;
            emit powerChanged(m_power);
        }

        if (g_variant_dict_lookup(propsDict, "Discoverable", "b", &discoverable)) {
            m_discoverable = discoverable;
            emit discoverableChanged(m_discoverable);
        }

        g_variant_dict_unref(propsDict);
        g_variant_unref(reply);
    }

    emit devicesChanged(readDeviceList());
    enforceSingleConnectedDevice();
    emit initialized(true);
}

void BluetoothWorker::handleAdapterEvent(gchar *adapter, bluez_event_t event, GVariant *properties)
{
    qDebug() << "[BluetoothWorker] handleAdapterEvent adapter =" << (adapter ? adapter : "")
             << "event =" << event
             << "thread =" << QThread::currentThread();

    if (event != BLUEZ_EVENT_CHANGE || !properties)
        return;

    GVariantDict *propsDict = g_variant_dict_new(properties);
    if (!propsDict)
        return;

    gboolean powered = FALSE;
    if (g_variant_dict_lookup(propsDict, "Powered", "b", &powered) && m_power != powered) {
        m_power = powered;
        emit powerChanged(m_power);

        if (!m_power) {
            m_discovering = false;
            m_discoverable = false;
            m_pendingConnectDeviceId.clear();
            m_activeConnectedDeviceId.clear();
            m_devices.clear();
            emit discoveryChanged(false);
            emit discoverableChanged(false);
            emit devicesChanged(QList<BluetoothDeviceInfo>());
        } else {
            emit devicesChanged(readDeviceList());
            enforceSingleConnectedDevice();
        }
    }

    gboolean discoverable = FALSE;
    if (g_variant_dict_lookup(propsDict, "Discoverable", "b", &discoverable) &&
        m_discoverable != discoverable) {
        m_discoverable = discoverable;
        emit discoverableChanged(m_discoverable);
    }

    gboolean discovering = FALSE;
    if (g_variant_dict_lookup(propsDict, "Discovering", "b", &discovering) &&
        m_discovering != discovering) {
        m_discovering = discovering;
        emit discoveryChanged(m_discovering);
    }

    g_variant_dict_unref(propsDict);
}

void BluetoothWorker::handleDeviceEvent(gchar *adapter,
                                        gchar *device,
                                        bluez_event_t event,
                                        GVariant *properties)
{
    Q_UNUSED(adapter);

    const QString rawId = QString::fromUtf8(device ? device : "");
    const QString id = normalizeDeviceId(rawId);
    qDebug() << "[BluetoothWorker] handleDeviceEvent id =" << id
             << "rawId =" << rawId
             << "event =" << event
             << "thread =" << QThread::currentThread();

    if (id.isEmpty())
        return;

    if (event == BLUEZ_EVENT_REMOVE) {
        m_devices.remove(id);
        if (m_pendingConnectDeviceId == id)
            m_pendingConnectDeviceId.clear();
        if (m_activeConnectedDeviceId == id)
            m_activeConnectedDeviceId.clear();
        emit deviceRemoved(id);
        return;
    }

    if (!properties)
        return;

    BluetoothDeviceInfo deviceInfo = deviceFromVariant(id, properties, m_devices.value(id));
    m_devices.insert(id, deviceInfo);
    emit deviceUpdated(deviceInfo);

    if (deviceInfo.connected) {
        const QString preferredId = m_pendingConnectDeviceId == id ? id : m_activeConnectedDeviceId;
        enforceSingleConnectedDevice(preferredId);
    } else {
        if (m_pendingConnectDeviceId == id)
            m_pendingConnectDeviceId.clear();
        if (m_activeConnectedDeviceId == id)
            m_activeConnectedDeviceId = connectedDeviceId();
    }
}

void BluetoothWorker::handleAgentEvent(gchar *device, bluez_agent_event_t event, int pincode)
{
    Q_UNUSED(device);

    if (event != BLUEZ_AGENT_EVENT_REQUEST_CONFIRMATION)
        return;

    emit requestConfirmationEvent(QString::number(pincode));
}

void BluetoothWorker::handleConnectResult(gchar *device, gboolean status)
{
    const QString id = normalizeDeviceId(QString::fromUtf8(device ? device : ""));
    qDebug() << "[BluetoothWorker] connect result id =" << id << "status =" << status;

    if (!status) {
        if (m_pendingConnectDeviceId == id)
            m_pendingConnectDeviceId.clear();
        emit errorOccurred("Bluetooth connect failed");
    } else if (m_pendingConnectDeviceId == id) {
        m_pendingConnectDeviceId.clear();
    }

    emit devicesChanged(readDeviceList());
    enforceSingleConnectedDevice(status ? id : QString());
}

QList<BluetoothDeviceInfo> BluetoothWorker::readDeviceList()
{
    QList<BluetoothDeviceInfo> devices;
    QHash<QString, BluetoothDeviceInfo> deviceCache;

    GVariant *reply = nullptr;
    if (!bluez_adapter_get_devices(nullptr, &reply) || !reply)
        return devices;

    GVariantIter *array = nullptr;
    g_variant_get(reply, "a{sv}", &array);

    const gchar *key = nullptr;
    GVariant *var = nullptr;
    while (array && g_variant_iter_next(array, "{&sv}", &key, &var)) {
        BluetoothDeviceInfo device = deviceFromVariant(QString::fromUtf8(key ? key : ""), var);
        if (!device.id.isEmpty()) {
            devices.append(device);
            deviceCache.insert(device.id, device);
        }
        g_variant_unref(var);
    }

    if (array)
        g_variant_iter_free(array);
    g_variant_unref(reply);

    m_devices = deviceCache;
    m_activeConnectedDeviceId = connectedDeviceId(devices);

    return devices;
}

BluetoothDeviceInfo BluetoothWorker::deviceFromVariant(const QString &id,
                                                       GVariant *properties,
                                                       const BluetoothDeviceInfo &base) const
{
    BluetoothDeviceInfo device = base;
    device.id = normalizeDeviceId(id);

    GVariantDict *propsDict = g_variant_dict_new(properties);
    if (!propsDict)
        return device;

    gchar *stringValue = nullptr;
    if (g_variant_dict_lookup(propsDict, "Address", "s", &stringValue)) {
        device.address = QString::fromUtf8(stringValue);
        g_free(stringValue);
    }

    stringValue = nullptr;
    if (g_variant_dict_lookup(propsDict, "Name", "s", &stringValue)) {
        device.name = QString::fromUtf8(stringValue);
        g_free(stringValue);
    }

    gboolean boolValue = FALSE;
    if (g_variant_dict_lookup(propsDict, "Paired", "b", &boolValue))
        device.paired = boolValue;

    boolValue = FALSE;
    if (g_variant_dict_lookup(propsDict, "Connected", "b", &boolValue))
        device.connected = boolValue;

    g_variant_dict_unref(propsDict);
    return device;
}

QString BluetoothWorker::normalizeDeviceId(QString id) const
{
    id = id.trimmed();
    if (id.isEmpty())
        return "";

    int devIndex = id.indexOf("dev_");
    if (devIndex >= 0)
        return id.mid(devIndex);

    const QString address = deviceIdToAddress(id);
    if (!address.isEmpty())
        return "dev_" + address.toUpper().replace(":", "_");

    return id;
}

void BluetoothWorker::start()
{
    qDebug() << "[BluetoothWorker] start() thread =" << QThread::currentThread();

    if (m_started) {
        emit initialized(true);
        return;
    }

    bluez_add_adapter_event_callback(adapterEventCallback, this);
    bluez_add_device_event_callback(deviceEventCallback, this);
    bluez_add_agent_event_callback(agentEventCallback, this);

    /*
     * bluez_init() có thể chờ D-Bus/agent đến 10s. Chạy ở worker thread để
     * Settings UI không đứng lúc app khởi động hoặc lúc mở màn hình Device.
     */
    const gboolean ok = bluez_init(TRUE, FALSE, initCallback, this);
    if (!ok) {
        emit initialized(false);
        emit errorOccurred("Failed to initialize BlueZ");
        return;
    }

    m_started = true;
}

void BluetoothWorker::refreshDevices()
{
    if (!m_started)
        return;

    emit devicesChanged(readDeviceList());
    enforceSingleConnectedDevice();
}

void BluetoothWorker::setPower(bool state)
{
    qDebug() << "[BluetoothWorker] setPower() state =" << state
             << "thread =" << QThread::currentThread();

    if (!m_started) {
        emit errorOccurred("Cannot set power: Bluetooth backend not started");
        return;
    }

    const gboolean ok = bluez_adapter_set_powered(nullptr, state ? TRUE : FALSE);
    if (!ok) {
        emit errorOccurred("Failed to set Bluetooth power");
        return;
    }

    if (m_power != state) {
        m_power = state;
        emit powerChanged(state);
    }

    if (!state) {
        m_discovering = false;
        m_discoverable = false;
        m_pendingConnectDeviceId.clear();
        m_activeConnectedDeviceId.clear();
        m_devices.clear();
        emit discoveryChanged(false);
        emit discoverableChanged(false);
        emit devicesChanged(QList<BluetoothDeviceInfo>());
    }
}

void BluetoothWorker::setDiscoverable(bool state)
{
    qDebug() << "[BluetoothWorker] setDiscoverable() state =" << state
             << "thread =" << QThread::currentThread();

    if (!m_started) {
        emit errorOccurred("Cannot set discoverable: Bluetooth backend not started");
        return;
    }

    const gboolean ok = bluez_adapter_set_discoverable(nullptr, state ? TRUE : FALSE);
    if (!ok) {
        emit errorOccurred("Failed to set Bluetooth discoverable");
        return;
    }

    if (m_discoverable != state) {
        m_discoverable = state;
        emit discoverableChanged(state);
    }
}

void BluetoothWorker::startDiscovery()
{
    qDebug() << "[BluetoothWorker] startDiscovery() thread =" << QThread::currentThread();

    if (!m_started) {
        emit errorOccurred("Cannot start discovery: Bluetooth backend not started");
        return;
    }

    if (m_discovering)
        return;

    setDiscoveryFilter();

    const gboolean ok = bluez_adapter_set_discovery(nullptr, TRUE);
    if (!ok) {
        emit errorOccurred("Failed to start Bluetooth discovery");
        return;
    }

    m_discovering = true;
    emit discoveryChanged(true);
}

void BluetoothWorker::stopDiscovery()
{
    qDebug() << "[BluetoothWorker] stopDiscovery() thread =" << QThread::currentThread();

    if (!m_started)
        return;

    const gboolean ok = bluez_adapter_set_discovery(nullptr, FALSE);
    if (!ok)
        qDebug() << "[BluetoothWorker] stopDiscovery failed or discovery was not active";

    if (m_discovering) {
        m_discovering = false;
        emit discoveryChanged(false);
    }
}

QString BluetoothWorker::deviceIdToAddress(QString id) const
{
    id = id.trimmed();

    /*
     * Input có thể là:
     * 1. 88:A4:79:65:91:34
     * 2. dev_88_A4_79_65_91_34
     * 3. /org/bluez/hci0/dev_88_A4_79_65_91_34
     */

    static const QRegularExpression macRe(
        R"(([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2})"
    );

    QRegularExpressionMatch macMatch = macRe.match(id);
    if (macMatch.hasMatch())
        return macMatch.captured(0).toUpper();

    int devIndex = id.indexOf("dev_");
    if (devIndex >= 0) {
        QString dev = id.mid(devIndex + 4);
        dev = dev.replace("_", ":");

        QRegularExpressionMatch devMacMatch = macRe.match(dev);
        if (devMacMatch.hasMatch())
            return devMacMatch.captured(0).toUpper();
    }

    return "";
}

QString BluetoothWorker::deviceIdToBluezCardName(QString id) const
{
    QString address = deviceIdToAddress(id);
    if (address.isEmpty())
        return "";

    return "bluez_card." + address.replace(":", "_");
}

QString BluetoothWorker::connectedDeviceId() const
{
    if (!m_activeConnectedDeviceId.isEmpty() &&
        m_devices.value(m_activeConnectedDeviceId).connected) {
        return m_activeConnectedDeviceId;
    }

    for (auto it = m_devices.constBegin(); it != m_devices.constEnd(); ++it) {
        if (it.value().connected)
            return it.key();
    }

    return "";
}

QString BluetoothWorker::connectedDeviceId(const QList<BluetoothDeviceInfo> &devices) const
{
    if (!m_activeConnectedDeviceId.isEmpty()) {
        for (const BluetoothDeviceInfo &device : devices) {
            if (device.id == m_activeConnectedDeviceId && device.connected)
                return device.id;
        }
    }

    for (const BluetoothDeviceInfo &device : devices) {
        if (device.connected)
            return device.id;
    }

    return "";
}

bool BluetoothWorker::hasOtherConnectedDevice(const QList<BluetoothDeviceInfo> &devices,
                                              const QString &deviceId,
                                              QString *connectedDeviceId) const
{
    for (const BluetoothDeviceInfo &device : devices) {
        if (!device.connected || device.id == deviceId)
            continue;

        if (connectedDeviceId)
            *connectedDeviceId = device.id;
        return true;
    }

    if (connectedDeviceId)
        connectedDeviceId->clear();
    return false;
}

void BluetoothWorker::disconnectDeviceSilently(const QString &deviceId)
{
    if (deviceId.isEmpty())
        return;

    qDebug() << "[BluetoothWorker] auto-disconnect extra connected device =" << deviceId;

    const QByteArray deviceBytes = deviceId.toLocal8Bit();
    const gboolean ok = bluez_device_disconnect(deviceBytes.constData(), nullptr);
    if (!ok)
        qDebug() << "[BluetoothWorker] auto-disconnect failed for" << deviceId;
}

void BluetoothWorker::enforceSingleConnectedDevice(const QString &preferredDeviceId)
{
    QString keepDeviceId = normalizeDeviceId(preferredDeviceId);
    if (keepDeviceId.isEmpty())
        keepDeviceId = connectedDeviceId();

    QStringList extraDeviceIds;
    for (auto it = m_devices.constBegin(); it != m_devices.constEnd(); ++it) {
        if (!it.value().connected)
            continue;

        if (keepDeviceId.isEmpty()) {
            keepDeviceId = it.key();
            continue;
        }

        if (it.key() == keepDeviceId)
            continue;

        extraDeviceIds.append(it.key());
    }

    m_activeConnectedDeviceId = keepDeviceId;

    for (const QString &extraDeviceId : extraDeviceIds) {
        disconnectDeviceSilently(extraDeviceId);
        BluetoothDeviceInfo info = m_devices.value(extraDeviceId);
        if (!info.id.isEmpty()) {
            info.connected = false;
            m_devices.insert(extraDeviceId, info);
            emit deviceUpdated(info);
        }
    }

    if (!extraDeviceIds.isEmpty())
        emit devicesChanged(readDeviceList());
}

bool BluetoothWorker::runShellCommand(const QString &command, QString *stdOut, QString *stdErr) const
{
    qDebug() << "[BluetoothWorker] runShellCommand:" << command;

    QProcess process;
    process.setProgram("/bin/sh");
    process.setArguments(QStringList() << "-lc" << command);

    /*
     * AGL/PipeWire của bạn đang thường dùng /run/user/0 khi chạy root.
     * Nếu sau này chạy bằng agl-driver, đổi thành /run/user/<uid>.
     */
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!env.contains("XDG_RUNTIME_DIR"))
        env.insert("XDG_RUNTIME_DIR", "/run/user/0");

    process.setProcessEnvironment(env);
    process.start();

    if (!process.waitForStarted(2000)) {
        if (stdErr)
            *stdErr = "Failed to start process";
        qDebug() << "[BluetoothWorker] command failed to start";
        return false;
    }

    if (!process.waitForFinished(8000)) {
        process.kill();
        process.waitForFinished(1000);

        if (stdErr)
            *stdErr = "Process timeout";

        qDebug() << "[BluetoothWorker] command timeout";
        return false;
    }

    QString out = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    QString err = QString::fromUtf8(process.readAllStandardError()).trimmed();

    if (stdOut)
        *stdOut = out;
    if (stdErr)
        *stdErr = err;

    qDebug() << "[BluetoothWorker] command exitCode =" << process.exitCode()
             << "stdout =" << out
             << "stderr =" << err;

    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

void BluetoothWorker::trustDevice(QString id, bool reportErrors)
{
    QString address = deviceIdToAddress(id);

    if (address.isEmpty()) {
        qDebug() << "[BluetoothWorker] trustDevice skipped: cannot parse address from" << id;
        return;
    }

    QString out;
    QString err;

    QString command = QString("bluetoothctl trust %1").arg(address);
    bool ok = runShellCommand(command, &out, &err);

    if (!ok) {
        qDebug() << "[BluetoothWorker] trustDevice failed:" << err;
        if (reportErrors)
            emit errorOccurred("Failed to trust Bluetooth device: " + err);
        return;
    }

    qDebug() << "[BluetoothWorker] trustDevice ok:" << address;
}

void BluetoothWorker::forceA2dpSink(QString id)
{
    QString address = deviceIdToAddress(id);
    QString cardName = deviceIdToBluezCardName(id);

    if (address.isEmpty() || cardName.isEmpty()) {
        qDebug() << "[BluetoothWorker] forceA2dpSink skipped: cannot parse from" << id;
        return;
    }

    QString out;
    QString err;

    /*
     * Image AGL của bạn:
     * - có wpctl
     * - có pw-cli
     * - không có pactl
     *
     * wpctl không có lệnh set-card-profile đơn giản như pactl.
     * Vì vậy ở đây:
     * 1. Nếu sau này image có pactl thì dùng pactl.
     * 2. Nếu không có pactl thì dump wpctl/pw-cli để xem PipeWire đang tạo node nào.
     * 3. Không emit error fatal vì thiếu pactl không phải lỗi Bluetooth connect.
     */
    QString command =
        "export XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-/run/user/0}; "
        "echo '[forceA2dpSink] card=" + cardName + " address=" + address + "'; "
        "if command -v pactl >/dev/null 2>&1; then "
        "  echo '[forceA2dpSink] using pactl'; "
        "  pactl set-card-profile " + cardName + " a2dp-sink; "
        "elif command -v wpctl >/dev/null 2>&1; then "
        "  echo '[forceA2dpSink] pactl not found; dumping wpctl status'; "
        "  wpctl status; "
        "  echo '[forceA2dpSink] bluez nodes from pw-cli:'; "
        "  pw-cli ls Node 2>/dev/null | grep -i -A25 -B5 'bluez\\|a2dp\\|sco\\|hfp\\|headset' || true; "
        "  echo '[forceA2dpSink] bluez devices from pw-cli:'; "
        "  pw-cli ls Device 2>/dev/null | grep -i -A25 -B5 'bluez\\|a2dp\\|sco\\|hfp\\|headset' || true; "
        "else "
        "  echo '[forceA2dpSink] neither pactl nor wpctl found'; "
        "fi";

    bool ok = runShellCommand(command, &out, &err);

    qDebug() << "[BluetoothWorker] forceA2dpSink result for" << cardName
             << "ok =" << ok
             << "stdout =" << out
             << "stderr =" << err;

    /*
     * Không emit errorOccurred ở đây.
     * Vì trên image này không có pactl là bình thường.
     */
}

void BluetoothWorker::pairDevice(QString id)
{
    qDebug() << "[BluetoothWorker] pairDevice() id =" << id
             << "thread =" << QThread::currentThread();

    const QString deviceId = normalizeDeviceId(id);
    if (deviceId.isEmpty()) {
        emit errorOccurred("Cannot pair: empty bluetooth device id");
        return;
    }

    if (!m_started) {
        emit errorOccurred("Cannot pair: Bluetooth backend not started");
        return;
    }

    /*
     * Pair cũng nên trust trước/sau. Nếu chưa paired, trust có thể fail nhẹ,
     * nhưng không ảnh hưởng pair.
     */
    trustDevice(deviceId, false);

    const QByteArray deviceBytes = deviceId.toLocal8Bit();
    const gboolean ok = bluez_device_pair(deviceBytes.constData(), nullptr, nullptr);
    if (!ok) {
        emit errorOccurred("Failed to start Bluetooth pairing");
        return;
    }

    QTimer::singleShot(3000, this, [this]() {
        emit devicesChanged(readDeviceList());
        enforceSingleConnectedDevice();
    });
}

void BluetoothWorker::connectDevice(QString id)
{
    qDebug() << "[BluetoothWorker] connectDevice() id =" << id
             << "thread =" << QThread::currentThread();

    const QString deviceId = normalizeDeviceId(id);
    if (deviceId.isEmpty()) {
        emit errorOccurred("Cannot connect: empty bluetooth device id");
        return;
    }

    if (!m_started) {
        emit errorOccurred("Cannot connect: Bluetooth backend not started");
        return;
    }

    if (!m_pendingConnectDeviceId.isEmpty()) {
        if (m_pendingConnectDeviceId == deviceId) {
            qDebug() << "[BluetoothWorker] connect ignored, connection already pending for" << deviceId;
            return;
        }

        qDebug() << "[BluetoothWorker] connect rejected, pending device =" << m_pendingConnectDeviceId
                 << "requested =" << deviceId;
        emit errorOccurred("Cannot connect: another Bluetooth connection is already in progress");
        return;
    }

    QList<BluetoothDeviceInfo> devices = readDeviceList();
    QString connectedId;
    bool hasOtherConnected = hasOtherConnectedDevice(devices, deviceId, &connectedId);
    if (!hasOtherConnected) {
        const QString cachedConnectedId = connectedDeviceId();
        if (!cachedConnectedId.isEmpty() && cachedConnectedId != deviceId) {
            connectedId = cachedConnectedId;
            hasOtherConnected = true;
        }
    }

    if (hasOtherConnected) {
        qDebug() << "[BluetoothWorker] connect rejected, already connected device =" << connectedId
                 << "requested =" << deviceId;
        emit devicesChanged(devices);
        emit errorOccurred("Cannot connect: another Bluetooth device is already connected");
        return;
    }

    m_activeConnectedDeviceId = connectedDeviceId(devices);
    m_pendingConnectDeviceId = deviceId;

    /*
     * 1. Trust device để Media audio/profile ổn định hơn.
     */
    trustDevice(deviceId);

    const QByteArray deviceBytes = deviceId.toLocal8Bit();
    const gboolean ok = bluez_device_connect(deviceBytes.constData(), nullptr, deviceConnectCallback, this);
    if (!ok) {
        m_pendingConnectDeviceId.clear();
        emit errorOccurred("Failed to start Bluetooth connection");
        return;
    }

    /*
     * 2. Sau khi BlueZ/PipeWire có thời gian tạo card, ép về A2DP Sink.
     * Đây là bước xử lý lỗi "connect từ app thì tiếng rè" do rơi sang HFP/SCO.
     */
    QTimer::singleShot(2500, this, [this, deviceId]() {
        qDebug() << "[BluetoothWorker] delayed forceA2dpSink for" << deviceId;
        forceA2dpSink(deviceId);
    });

    /*
     * 3. Một số máy cần thêm lần thứ hai vì card tạo chậm.
     */
    QTimer::singleShot(5000, this, [this, deviceId]() {
        qDebug() << "[BluetoothWorker] second delayed forceA2dpSink for" << deviceId;
        forceA2dpSink(deviceId);
    });
}

void BluetoothWorker::disconnectDevice(QString id)
{
    qDebug() << "[BluetoothWorker] disconnectDevice() id =" << id
             << "thread =" << QThread::currentThread();

    const QString deviceId = normalizeDeviceId(id);
    if (deviceId.isEmpty()) {
        emit errorOccurred("Cannot disconnect: empty bluetooth device id");
        return;
    }

    if (!m_started) {
        emit errorOccurred("Cannot disconnect: Bluetooth backend not started");
        return;
    }

    const QByteArray deviceBytes = deviceId.toLocal8Bit();
    const gboolean ok = bluez_device_disconnect(deviceBytes.constData(), nullptr);
    if (!ok) {
        emit errorOccurred("Failed to disconnect Bluetooth device");
        return;
    }

    if (m_pendingConnectDeviceId == deviceId)
        m_pendingConnectDeviceId.clear();
    if (m_activeConnectedDeviceId == deviceId)
        m_activeConnectedDeviceId.clear();

    emit devicesChanged(readDeviceList());
    m_activeConnectedDeviceId = connectedDeviceId();
}

void BluetoothWorker::removeDevice(QString id)
{
    qDebug() << "[BluetoothWorker] removeDevice() id =" << id
             << "thread =" << QThread::currentThread();

    const QString deviceId = normalizeDeviceId(id);
    if (deviceId.isEmpty()) {
        emit errorOccurred("Cannot remove: empty bluetooth device id");
        return;
    }

    const QString address = deviceIdToAddress(deviceId);

    if (address.isEmpty()) {
        emit errorOccurred("Cannot remove: invalid bluetooth device id: " + deviceId);
        return;
    }

    QString out;
    QString err;

    /*
     * bluetoothctl remove <MAC> = unpair / forget device.
     * Disconnect trước để tránh lỗi thiết bị đang active.
     *
     * Dùng timeout shell nhẹ để không treo UI nếu bluetoothctl bị kẹt.
     */
    const QString command = QString(
        "echo '[removeDevice] address=%1'; "
        "bluetoothctl disconnect %1 >/dev/null 2>&1 || true; "
        "sleep 1; "
        "bluetoothctl remove %1"
    ).arg(address);

    const bool ok = runShellCommand(command, &out, &err);

    if (!ok) {
        qWarning() << "[BluetoothWorker] removeDevice failed:"
                   << "address =" << address
                   << "stdout =" << out
                   << "stderr =" << err;

        emit errorOccurred("Failed to remove Bluetooth device: " + err);
        return;
    }

    qDebug() << "[BluetoothWorker] removeDevice ok:"
             << "address =" << address
             << "stdout =" << out;

    if (m_pendingConnectDeviceId == deviceId)
        m_pendingConnectDeviceId.clear();
    if (m_activeConnectedDeviceId == deviceId)
        m_activeConnectedDeviceId.clear();

    emit deviceRemoved(deviceId);
    emit devicesChanged(readDeviceList());
    m_activeConnectedDeviceId = connectedDeviceId();
}

void BluetoothWorker::sendConfirmation(int pincode)
{
    qDebug() << "[BluetoothWorker] sendConfirmation() pincode =" << pincode
             << "thread =" << QThread::currentThread();

    if (!m_started) {
        emit errorOccurred("Cannot confirm pairing: Bluetooth backend not started");
        return;
    }

    const QByteArray pincodeBytes = QByteArray::number(pincode);
    const gboolean ok = bluez_confirm_pairing(pincodeBytes.constData());
    if (!ok)
        emit errorOccurred("Failed to confirm Bluetooth pairing");
}

void BluetoothWorker::setDiscoveryFilter()
{
    QList<QByteArray> uuidBytes = {
        QByteArrayLiteral("0000110a-0000-1000-8000-00805f9b34fb"),
        QByteArrayLiteral("0000110e-0000-1000-8000-00805f9b34fb"),
        QByteArrayLiteral("0000111f-0000-1000-8000-00805f9b34fb")
    };

    gchar **uuids = static_cast<gchar **>(g_malloc0((uuidBytes.count() + 1) * sizeof(gchar *)));
    for (int i = 0; i < uuidBytes.count(); ++i)
        uuids[i] = g_strdup(uuidBytes.at(i).constData());

    gchar *transport = g_strdup("bredr");
    bluez_adapter_set_discovery_filter(nullptr, uuids, transport);

    for (int i = 0; i < uuidBytes.count(); ++i)
        g_free(uuids[i]);
    g_free(uuids);
    g_free(transport);
}
