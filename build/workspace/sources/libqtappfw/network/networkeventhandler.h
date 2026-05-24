/*
 * Copyright (C) 2022 Konsulko Group
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

#ifndef NETWORK_EVENT_HANDLER_H
#define NETWORK_EVENT_HANDLER_H

#include <QObject>
#include <QtQml/QQmlContext>
#include <connman-glib.h>

class Network;

class NetworkEventHandler : public QObject
{
	Q_OBJECT

public:
	explicit NetworkEventHandler(Network *network, bool register_agent, QQmlContext *context, QObject * parent = Q_NULLPTR);

        virtual ~NetworkEventHandler();

	static void service_connect_cb(const gchar *service, gboolean status, const gchar *error, gpointer user_data) {
		if (user_data)
			((NetworkEventHandler*) user_data)->handle_connect_event(service, status, error);
	}

	bool populateAdapterProperties(GVariant *in, QVariantMap &out);
	bool populateServiceProperties(GVariant *in, QVariantMap &out);

signals:
	void adapterStatusChanged(const QString &technology, const QVariantMap &properties);
	void servicePropertiesChanged(const QString &service, const QVariantMap &properties);
	bool serviceAdded(const QString &service, const QVariantMap &properties);
	bool serviceRemoved(const QString &service);
	void inputRequested(int id, const QVariantMap &properties);
	void connectResponseReceived(const QString &service, bool status, const QString &error);

private:
        Network *m_network;
	bool m_agent;

	// Callback functions for connman-glib hooks
	static void manager_event_cb(const gchar *path, connman_manager_event_t event, GVariant *properties, gpointer user_data) {
		if (user_data)
			((NetworkEventHandler*) user_data)->handle_manager_event(path, event, properties);
	}
 
	static void technology_event_cb(const gchar *technology, GVariant *properties, gpointer user_data) {
		if (user_data)
			((NetworkEventHandler*) user_data)->handle_technology_event(technology, properties);
	}

	static void service_event_cb(const gchar *service, GVariant *properties, gpointer user_data) {
		if (user_data)
			((NetworkEventHandler*) user_data)->handle_service_event(service, properties);
	}

	static void agent_event_cb(const gchar *service, const int id, GVariant *properties, gpointer user_data) {
		if (user_data)
			((NetworkEventHandler*) user_data)->handle_agent_event(service, id, properties);
	}
 
        void handle_connect_event(const gchar *service, gboolean status, const gchar *error);
        void handle_manager_event(const gchar *path, connman_manager_event_t event, GVariant *properties);
        void handle_technology_event(const gchar *technology, GVariant *properties);
        void handle_service_event(const gchar *service, GVariant *properties);
	void handle_agent_event(const gchar *service, const int id, GVariant *properties);
};

#endif // NETWORK_EVENT_HANDLER_H
