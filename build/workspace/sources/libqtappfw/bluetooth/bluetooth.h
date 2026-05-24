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

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <memory>
#include <QObject>
#include <QtQml/QQmlContext>

class BluetoothModel;
class BluetoothEventHandler;

class Bluetooth : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool power READ power WRITE setPower NOTIFY powerChanged)
	Q_PROPERTY(bool discoverable READ discoverable WRITE setDiscoverable NOTIFY discoverableChanged)
	Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
	Q_PROPERTY(bool mediaConnected READ mediaConnected NOTIFY mediaConnectedChanged)

public:
	explicit Bluetooth(bool register_agent,
			   QQmlContext *context,
			   bool handle_media = false,
			   QObject *parent = Q_NULLPTR);
	virtual ~Bluetooth();

	void setPower(bool);
	void setDiscoverable(bool);

	Q_INVOKABLE void start(void);

	Q_INVOKABLE void start_discovery(void);
	Q_INVOKABLE void stop_discovery(void);

	Q_INVOKABLE void remove_device(QString device);
	Q_INVOKABLE void pair(QString device);
	Q_INVOKABLE void cancel_pair(void);

	Q_INVOKABLE void connect(QString device, QString uuid);
	Q_INVOKABLE void connect(QString device);

	Q_INVOKABLE void disconnect(QString device, QString uuid);
	Q_INVOKABLE void disconnect(QString device);

	Q_INVOKABLE void send_confirmation(int pincode);

	enum MediaAction {
		Connect, Disconnect,
		Play, Pause, Stop, Next, Previous, FastForward, Rewind, Loop
	};
	Q_ENUM(MediaAction)

	Q_INVOKABLE void media_control(MediaAction action);
	Q_INVOKABLE void refresh_media_state();

	bool power() const { return m_power; };
	bool discoverable() const { return m_discoverable; };
	bool connected() const { return m_connected; };
	bool mediaConnected() const { return m_media_connected; };

signals:
	void powerChanged(bool state);
	void discoverableChanged(bool state);
	void connectedChanged(bool state);
	void mediaConnectedChanged(bool state);

	void mediaPropertiesChanged(QVariantMap metadata);
	void requestConfirmationEvent(QString pincode);

private:
	QQmlContext *m_context;
	BluetoothModel *m_bluetooth;
	BluetoothEventHandler *m_event_handler;
	bool m_agent;
	bool m_handle_media;

	void init_adapter_state(const QString &adapter);
	void refresh_device_list(void);
	void set_discovery_filter(void);
	void discovery_command(const bool);
	void update_adapter_power(const bool powered);
	void update_connected_state(const QString &device, const bool connected);
	void update_media_connected_state(const bool connected);
	void update_media_properties(const QVariantMap &metadata);
	void request_confirmation(const int pincode);

	QString process_uuid(QString uuid) { if (uuid.length() == 36) return uuid; return uuids.value(uuid); };

	// values
	bool m_power;
	bool m_discoverable;
	bool m_connected;
	bool m_media_connected;

	QString m_connected_device;

	QMap<QString, QString> uuids;

	friend class BluetoothEventHandler;
};

#endif // BLUETOOTH_H
