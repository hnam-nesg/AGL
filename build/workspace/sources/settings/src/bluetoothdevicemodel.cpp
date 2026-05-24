#include "bluetoothdevicemodel.h"

BluetoothDeviceModel::BluetoothDeviceModel(DeviceFilter filter, QObject *parent)
    : QAbstractListModel(parent)
    , m_filter(filter)
{
}

int BluetoothDeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_devices.count();
}

QVariant BluetoothDeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_devices.count())
        return QVariant();

    const BluetoothDeviceInfo &device = m_devices.at(index.row());

    switch (role) {
    case IdRole:
        return device.id;
    case AddressRole:
        return device.address;
    case NameRole:
        return device.name;
    case PairedRole:
        return device.paired;
    case ConnectedRole:
        return device.connected;
    default:
        return QVariant();
    }
}

void BluetoothDeviceModel::setDevices(const QList<BluetoothDeviceInfo> &devices)
{
    QList<BluetoothDeviceInfo> filteredDevices;
    for (const BluetoothDeviceInfo &device : devices) {
        if (acceptsDevice(device))
            filteredDevices.append(device);
    }

    beginResetModel();
    m_devices = filteredDevices;
    endResetModel();
}

void BluetoothDeviceModel::clear()
{
    if (m_devices.isEmpty())
        return;

    beginResetModel();
    m_devices.clear();
    endResetModel();
}

void BluetoothDeviceModel::upsertDevice(const BluetoothDeviceInfo &device)
{
    const int row = indexOf(device.id);
    const bool accepted = acceptsDevice(device);

    if (!accepted) {
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            m_devices.removeAt(row);
            endRemoveRows();
        }
        return;
    }

    if (row < 0) {
        const int insertRow = m_devices.count();
        beginInsertRows(QModelIndex(), insertRow, insertRow);
        m_devices.append(device);
        endInsertRows();
        return;
    }

    m_devices[row] = device;
    emit dataChanged(index(row, 0), index(row, 0));
}

void BluetoothDeviceModel::removeDevice(const QString &id)
{
    const int row = indexOf(id);
    if (row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_devices.removeAt(row);
    endRemoveRows();
}

QHash<int, QByteArray> BluetoothDeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[AddressRole] = "address";
    roles[NameRole] = "name";
    roles[PairedRole] = "paired";
    roles[ConnectedRole] = "connected";
    return roles;
}

bool BluetoothDeviceModel::acceptsDevice(const BluetoothDeviceInfo &device) const
{
    if (m_filter == PairedDevices)
        return device.paired;

    return !device.paired;
}

int BluetoothDeviceModel::indexOf(const QString &id) const
{
    for (int i = 0; i < m_devices.count(); ++i) {
        if (m_devices.at(i).id == id)
            return i;
    }

    return -1;
}
