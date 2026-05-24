/*
 * Copyright (C) 2021,2022 Konsulko Group
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

#ifndef BLUETOOTH_EVENT_HANDLER_H
#define BLUETOOTH_EVENT_HANDLER_H

class Bluetooth;

class BluetoothEventHandler
{
    public:
	explicit BluetoothEventHandler(Bluetooth *parent, bool register_agent, bool handle_media = false);
        virtual ~BluetoothEventHandler();

	static void init_cb(gchar *adapter, gboolean status, gpointer user_data) {
		if (user_data)
			((BluetoothEventHandler*) user_data)->handle_init_event(adapter, status);
	}

	static void device_connect_cb(gchar *device, gboolean status, gpointer user_data) {
		if (user_data)
			((BluetoothEventHandler*) user_data)->handle_connect_event(device, status);
	}
 
	static void device_pair_cb(gchar *device, gboolean status, gpointer user_data) {
		if (user_data)
			((BluetoothEventHandler*) user_data)->handle_pair_event(device, status);
	}

	// Helper function callable by parent (instead of vice versa), since Bluetooth
	// object should not expose glib types.
	void parse_media_player_properties(GVariant *properties, QVariantMap &metadata);

    private:
        Bluetooth *m_parent;
	bool m_agent;

	// Callback functions for bluez-glib hooks
	static void adapter_event_cb(gchar *adapter, bluez_event_t event, GVariant *properties, gpointer user_data) {
		if (user_data)
			((BluetoothEventHandler*) user_data)->handle_adapter_event(adapter, event, properties);
	}
 
	static void device_event_cb(gchar *adapter, gchar *device, bluez_event_t event, GVariant *properties, gpointer user_data) {
		if (user_data)
			((BluetoothEventHandler*) user_data)->handle_device_event(adapter, device, event, properties);
	}

	static void agent_event_cb(gchar *device, bluez_agent_event_t event, GVariant *properties, gpointer user_data) {
		if (user_data)
			((BluetoothEventHandler*) user_data)->handle_agent_event(device, event, properties);
	}

	static void media_control_event_cb(gchar *adapter,
					  gchar *device,
					  bluez_event_t event,
					  GVariant *properties,
					  gpointer user_data) {
		if (user_data)
			((BluetoothEventHandler*) user_data)->handle_media_control_event(adapter,
											 device,
											 event,
											 properties);
	}

	static void media_player_event_cb(gchar *adapter,
					  gchar *device,
					  gchar *player,
					  bluez_event_t event,
					  GVariant *properties,
					  gpointer user_data) {
		if (user_data)
			((BluetoothEventHandler*) user_data)->handle_media_player_event(adapter,
											device,
											player,
											event,
											properties);
	}
 
        void handle_init_event(gchar *adapter, gboolean status);
        void handle_adapter_event(gchar *adapter, bluez_event_t event, GVariant *properties);
        void handle_device_event(gchar *adapter, gchar *device, bluez_event_t event, GVariant *properties);
	void handle_agent_event(gchar *device, bluez_agent_event_t event, GVariant *properties);
	void handle_media_control_event(gchar *adapter,
					gchar *device,
					bluez_event_t event,
					GVariant *properties);
	void handle_media_player_event(gchar *adapter,
				       gchar *device,
				       gchar *player,
				       bluez_event_t event,
				       GVariant *properties);
        void handle_connect_event(gchar *device, gboolean status);
        void handle_pair_event(gchar *device, gboolean status);
};

#endif // BLUETOOTH_EVENT_HANDLER_H
