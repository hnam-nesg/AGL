#ifndef BLUETOOTHDEVICEMODEL_H
#define BLUETOOTHDEVICEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QMetaType>
#include <QString>

struct BluetoothDeviceInfo
{
    QString id;
    QString address;
    QString name;
    bool paired = false;
    bool connected = false;
};

class BluetoothDeviceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum BluetoothRoles {
        IdRole = Qt::UserRole + 1,
        AddressRole,
        NameRole,
        PairedRole,
        ConnectedRole
    };

    enum DeviceFilter {
        PairedDevices,
        DiscoveryDevices
    };

    explicit BluetoothDeviceModel(DeviceFilter filter, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setDevices(const QList<BluetoothDeviceInfo> &devices);
    void clear();
    void upsertDevice(const BluetoothDeviceInfo &device);
    void removeDevice(const QString &id);

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    bool acceptsDevice(const BluetoothDeviceInfo &device) const;
    int indexOf(const QString &id) const;

    DeviceFilter m_filter;
    QList<BluetoothDeviceInfo> m_devices;
};

Q_DECLARE_METATYPE(BluetoothDeviceInfo)

#endif // BLUETOOTHDEVICEMODEL_H
