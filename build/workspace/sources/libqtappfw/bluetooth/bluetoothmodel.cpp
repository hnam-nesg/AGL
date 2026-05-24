/*
 * Copyright (C) 2019-2021 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bluetoothmodel.h"
#include <QDebug>

BluetoothDevice::BluetoothDevice(const QString &id,
                                 const QString &address,
                                 const QString &name,
                                 const bool paired,
                                 const bool connected)
    : m_id(id), m_address(address), m_name(name), m_paired(paired),
      m_connected(connected)
{
}

QString BluetoothDevice::id() const
{
	return m_id;
}

QString BluetoothDevice::address() const
{
	return m_address;
}

QString BluetoothDevice::name() const
{
	return m_name;
}

bool BluetoothDevice::paired() const
{
	return m_paired;
}

bool BluetoothDevice::connected() const
{
	return m_connected;
}

void BluetoothDevice::setId(const QString id)
{
	m_id = id;
}

void BluetoothDevice::setAddress(const QString address)
{
	m_address = address;
}

void BluetoothDevice::setName(const QString name)
{
	m_name = name;
}

void BluetoothDevice::setPaired(const bool paired)
{
	m_paired = paired;
}

void BluetoothDevice::setConnected(const bool connected)
{
	m_connected = connected;
}

BluetoothModel::BluetoothModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void BluetoothModel::addDevice(BluetoothDevice *device)
{
	if (!device)
		return;

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_devices << device;
	endInsertRows();
}

void BluetoothModel::removeDevice(BluetoothDevice *device)
{
	if (!device)
		return;

	int row = m_devices.indexOf(device);
	if (row < 0)
		return;
	beginRemoveRows(QModelIndex(), row, row);
	m_devices.removeAt(row);
	endRemoveRows();
	delete device;
}

void BluetoothModel::removeAllDevices()
{
	if (!m_devices.count())
		return;

	beginRemoveRows(QModelIndex(), 0, m_devices.count() - 1);
	qDeleteAll(m_devices.begin(), m_devices.end());
	m_devices.clear();
	endRemoveRows();
}

int BluetoothModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_devices.count();
}

QVariant BluetoothModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_devices.count())
		return QVariant();

	const BluetoothDevice *device = m_devices[index.row()];

	switch (role) {
        case IdRole:
		return device->id();
        case AddressRole:
		return device->address();
        case NameRole:
		return device->name();
        case PairedRole:
		return device->paired();
        case ConnectedRole:
		return device->connected();
	}

	return QVariant();
}

QHash<int, QByteArray> BluetoothModel::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[IdRole] = "id";
	roles[AddressRole] = "address";
	roles[NameRole] = "name";
	roles[PairedRole] = "paired";
	roles[ConnectedRole] = "connected";

	return roles;
}

QModelIndex BluetoothModel::indexOf(BluetoothDevice *device)
{
	int row = m_devices.indexOf(device);

	return index(row);
}

BluetoothDevice *BluetoothModel::getDevice(QString id)
{
	for (auto device : m_devices) {
		if (device->id() == id)
			return device;
	}

	return nullptr;
}

BluetoothDevice *BluetoothModel::updateDeviceProperties(BluetoothDevice *device, const gchar *dev_str, GVariant *properties)
{
	GVariantDict *props_dict = g_variant_dict_new(properties);
	if (!props_dict)
		return nullptr;

	QString id(dev_str);
	if (id.isEmpty()) {
		g_variant_dict_unref(props_dict);
		return nullptr;
	}

	gchar *p = NULL;
	QString address;
        if (g_variant_dict_lookup(props_dict, "Address", "s", &p)) {
		address = QString(p);
		g_free(p);
	}

	p = NULL;
	QString name;
        if (g_variant_dict_lookup(props_dict, "Name", "s", &p)) {
		name = QString(p);
		g_free(p);
	}

	gboolean paired = FALSE;
	bool have_paired = false;
        if (g_variant_dict_lookup(props_dict, "Paired", "b", &paired)) {
		have_paired = true;
	}

	gboolean connected = FALSE;
	bool have_connected = false;
        if (g_variant_dict_lookup(props_dict, "Connected", "b", &connected)) {
		have_connected = true;
	}

	g_variant_dict_unref(props_dict);

	if (device == nullptr) {
		// Create new device object
		return new BluetoothDevice(id, address, name, paired, connected);
	}

	device->setId(id);

	if (!address.isEmpty())
		device->setAddress(address);

	if (!name.isEmpty())
		device->setName(name);

	if (have_paired)
		device->setPaired(paired);

	if (have_connected)
		device->setConnected(connected);

	emit dataChanged(indexOf(device), indexOf(device));

	return device;
}

BluetoothModelFilter::BluetoothModelFilter(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool BluetoothModelFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	BluetoothModel *model = qobject_cast<BluetoothModel *>(sourceModel());
	QModelIndex index = model->index(sourceRow);
	bool paired = model->data(index, BluetoothModel::BluetoothRoles::PairedRole).toBool();

	return ((paired ? "true" : "false") == filterRegularExpression().pattern());
}
