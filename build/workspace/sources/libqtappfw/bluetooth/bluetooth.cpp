/*
 * Copyright (C) 2018-2022 Konsulko Group
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

#include <QDebug>

#include <bluez-glib.h>

#include "bluetooth.h"
#include "bluetoothmodel.h"
#include "bluetootheventhandler.h"


Bluetooth::Bluetooth(bool register_agent,
		     QQmlContext *context,
		     bool handle_media,
		     QObject * parent) :
	QObject(parent),
	m_context(context),
	m_agent(register_agent),
	m_handle_media(handle_media),
	m_connected(false),
	m_connected_device(""),
	m_media_connected(false)
{
	m_bluetooth = new BluetoothModel();
	BluetoothModelFilter *m_model = new BluetoothModelFilter();
	m_model->setSourceModel(m_bluetooth);
	m_model->setFilterFixedString("true");
	context->setContextProperty("BluetoothPairedModel", m_model);

	m_model = new BluetoothModelFilter();
	m_model->setSourceModel(m_bluetooth);
	m_model->setFilterFixedString("false");
	context->setContextProperty("BluetoothDiscoveryModel", m_model);

	m_event_handler = new BluetoothEventHandler(this, register_agent, handle_media);

	uuids.insert("a2dp", "0000110a-0000-1000-8000-00805f9b34fb");
	uuids.insert("avrcp", "0000110e-0000-1000-8000-00805f9b34fb");
	uuids.insert("hfp", "0000111f-0000-1000-8000-00805f9b34fb");
}

Bluetooth::~Bluetooth()
{
}

void Bluetooth::setPower(bool state)
{
	bluez_adapter_set_powered(NULL, state ? TRUE : FALSE);
}

void Bluetooth::setDiscoverable(bool state)
{
	bluez_adapter_set_discoverable(NULL, state);

	m_discoverable = state;

	emit discoverableChanged(state);
}

void Bluetooth::start()
{
	// NOTE: An explicit "start" is somewhat required at present as calling
	//       bluez_init in the constructor means state update signals get
	//       emitted before the QML in an app seems able to receive them.
	//       A slightly better alternative might be something like a
	//       "refresh" function, or documenting that apps must explicitly
	//       call getters of relevant values on start.

	bluez_init(m_agent, m_agent, m_event_handler->init_cb, m_event_handler);
}

void Bluetooth::discovery_command(bool state)
{
	set_discovery_filter();

	bluez_adapter_set_discovery(NULL, state ? TRUE : FALSE);
}

void Bluetooth::start_discovery()
{
	discovery_command(true);
}

void Bluetooth::stop_discovery()
{
	discovery_command(false);
}

void Bluetooth::remove_device(QString device)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	bluez_device_remove(device_cstr);
}

void Bluetooth::pair(QString device)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	bluez_device_pair(device_cstr, m_event_handler->device_pair_cb, m_event_handler);
}

void Bluetooth::cancel_pair(void)
{
	bluez_cancel_pairing();
}

void Bluetooth::connect(QString device, QString uuid)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	uuid = process_uuid(uuid);
	QByteArray uuid_ba = uuid.toLocal8Bit();
	const char *uuid_cstr = uuid_ba.data();

	bluez_device_connect(device_cstr,
			     uuid_cstr,
			     m_event_handler->device_connect_cb, 
			     m_event_handler); 
}

void Bluetooth::connect(QString device)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	bluez_device_connect(device_cstr,
			     NULL,
			     m_event_handler->device_connect_cb, 
			     m_event_handler); 
}

void Bluetooth::disconnect(QString device, QString uuid)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	uuid = process_uuid(uuid);
	QByteArray uuid_ba = uuid.toLocal8Bit();
	const char *uuid_cstr = uuid_ba.data();

	bluez_device_disconnect(device_cstr, uuid_cstr);
}

void Bluetooth::disconnect(QString device)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	bluez_device_disconnect(device_cstr, NULL);
}

void Bluetooth::send_confirmation(const int pincode)
{
	QString pincode_str;
	pincode_str.setNum(pincode);
	QByteArray pincode_ba = pincode_str.toLocal8Bit();
	const char *pincode_cstr = pincode_ba.data();

	bluez_confirm_pairing(pincode_cstr);
}

void Bluetooth::media_control(MediaAction action)
{
	QString action_name;
	bool action_allowed = true;
	bluez_media_control_t bluez_action;
	switch (action) {
	case Connect:
		bluez_action = BLUEZ_MEDIA_CONTROL_CONNECT;
		action_name = "Connect";
		action_allowed = !m_media_connected;
		break;
	case Disconnect:
		bluez_action = BLUEZ_MEDIA_CONTROL_DISCONNECT;
		action_name = "Disconnect";
		action_allowed = m_media_connected;
		break;
	case Play:
		bluez_action = BLUEZ_MEDIA_CONTROL_PLAY;
		action_name = "Play";
		break;
	case Pause:
		bluez_action = BLUEZ_MEDIA_CONTROL_PAUSE;
		action_name = "Pause";
		break;
	case Stop:
		bluez_action = BLUEZ_MEDIA_CONTROL_STOP;
		action_name = "Stop";
		break;
	case Next:
		bluez_action = BLUEZ_MEDIA_CONTROL_NEXT;
		action_name = "Next";
		break;
	case Previous:
		bluez_action = BLUEZ_MEDIA_CONTROL_PREVIOUS;
		action_name = "Previous";
		break;
	case FastForward:
		bluez_action = BLUEZ_MEDIA_CONTROL_FASTFORWARD;
		action_name = "Fastforward";
		break;
	case Rewind:
		bluez_action = BLUEZ_MEDIA_CONTROL_REWIND;
		action_name = "Rewind";
		break;
	case Loop:
		// Not implemented, but potentially possible with bluez-glib addition
	default:
		break;
	}
#ifdef BLUETOOTH_EVENT_DEBUG
	qDebug() << "Bluetooth::media_control: enter, action = " << action_name;
	qDebug() << "Bluetooth::media_control: m_connected = " << m_connected
		 << ", m_media_connected = " << m_media_connected;
#endif

	if (!(m_connected && action_allowed)) {
		qDebug() << "Bluetooth::media_control: not connected or invalid action!";
		return;
	}

	QByteArray device_ba = m_connected_device.toLocal8Bit();
	const char *device_cstr = device_ba.data();
	bluez_device_avrcp_controls(device_cstr, bluez_action);
}


// Private

void Bluetooth::init_adapter_state(const QString &adapter)
{
	// Get initial power state
	GVariant *reply = NULL;
	gboolean rc = bluez_adapter_get_state(NULL, &reply);
	if (rc && reply) {
		GVariantDict *props_dict = g_variant_dict_new(reply);
		gboolean powered = FALSE;
		if (g_variant_dict_lookup(props_dict, "Powered", "b", &powered)) {
			if (m_power != powered) {
				m_power = powered;
				emit powerChanged(m_power);
			}
		}
		g_variant_dict_unref(props_dict);
		g_variant_unref(reply);
	}

	// Get initial device list
	refresh_device_list();

	// Do a refresh of the media state to handle the situation where
	// a client app has been started after a phone has been connected
	// (and thus misses seeing the related events go by).
	if (m_handle_media)
		refresh_media_state();
}

void Bluetooth::refresh_device_list(void)
{
	gboolean rc;
	GVariant *reply = NULL;

	rc = bluez_adapter_get_devices(NULL, &reply);
	if(!rc)
		return;

	m_bluetooth->removeAllDevices();

	GVariantIter *array = NULL;
	g_variant_get(reply, "a{sv}", &array);
	const gchar *key = NULL;
	GVariant *var = NULL;
	while (g_variant_iter_next(array, "{&sv}", &key, &var)) {
		BluetoothDevice *device = m_bluetooth->updateDeviceProperties(nullptr, key, var);
		if (device) {
			m_bluetooth->addDevice(device);
			if (device->connected()) {
				update_connected_state(device->id(), true);
			}
		}

		g_variant_unref(var);
	}
	g_variant_iter_free(array);
	g_variant_unref(reply);
}

void Bluetooth::refresh_media_state()
{
	if (!(m_handle_media && m_connected && m_connected_device.count()))
		return;

	QByteArray device_ba = m_connected_device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	GVariant *reply = NULL;
	if (!bluez_get_media_control_properties(device_cstr, &reply))
		return;

	GVariantDict *props_dict = g_variant_dict_new(reply);
	if (!props_dict) {
		g_variant_unref(reply);
		return;
	}

	gboolean connected = FALSE;
	if (g_variant_dict_lookup(props_dict, "Connected", "b", &connected)) {
		update_media_connected_state(connected);

		GVariant *player_reply = NULL;
		if(bluez_get_media_player_properties(device_cstr, &player_reply)) {
			QVariantMap tmp;
			m_event_handler->parse_media_player_properties(player_reply, tmp);
			if (!tmp.empty())
				update_media_properties(tmp);

			g_variant_unref(player_reply);
		}
	}
	g_variant_dict_unref(props_dict);
	g_variant_unref(reply);
}

void Bluetooth::set_discovery_filter(void)
{
	QList<QString> values = uuids.values();
	QStringListIterator eventIterator(values);

	gchar **uuids_array = (gchar**) g_malloc0((values.count() + 1) * sizeof(gchar*));
	int i = 0;
	while (eventIterator.hasNext()) {
		QByteArray uuid_ba = eventIterator.next().toLocal8Bit();
		gchar *uuid_cstr = g_strdup(uuid_ba.data());
		uuids_array[i++] = uuid_cstr;
	}

	gchar *transport = g_strdup("bredr");
	bluez_adapter_set_discovery_filter(NULL, uuids_array, transport);

	for (i = 0; i < values.count(); i++)
		g_free(uuids_array[i]);
	g_free(uuids_array);
	g_free(transport);
}

void Bluetooth::update_adapter_power(const bool powered)
{
	if (!powered)
		m_bluetooth->removeAllDevices();

	if (m_power != powered) {
		m_power = powered;
		emit powerChanged(powered);

		if (powered)
			refresh_device_list();
	}


	if (!m_power) {
		bool discoverable = m_discoverable;
		m_discoverable = false;
		if (discoverable != m_discoverable)
			emit discoverableChanged(false);
	}
}

void Bluetooth::update_connected_state(const QString &device, const bool connected)
{
#ifdef BLUETOOTH_EVENT_DEBUG
	qDebug() << "Bluetooth::update_connected_state: device = " << device
		 << ", connected = " << connected;
#endif
	if (m_connected != connected) {
		if (!m_connected) {
			// Connecting
			m_connected = true;
			m_connected_device = device;
			emit connectedChanged(true);
		} else if (m_connected_device == device) {
			// Disconnecting
			m_connected = false;
			m_connected_device = "";
			emit connectedChanged(false);
		} else {
			qDebug() << "Bluetooth::update_connected_state: ignoring " << device;
		}
	}
}

void Bluetooth::update_media_connected_state(const bool connected)
{
#ifdef BLUETOOTH_EVENT_DEBUG
	qDebug() << "Bluetooth::update_media_connected_state: connected = " << connected;
#endif
	if (m_media_connected != connected) {
		m_media_connected = connected;
		emit mediaConnectedChanged(connected);
	}
}

void Bluetooth::update_media_properties(const QVariantMap &metadata)
{
	emit mediaPropertiesChanged(metadata);
}

void Bluetooth::request_confirmation(const int pincode)
{
	QString pincode_str;
	pincode_str.setNum(pincode);
	emit requestConfirmationEvent(pincode_str);
}
