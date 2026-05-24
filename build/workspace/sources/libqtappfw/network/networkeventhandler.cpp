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

#include <QDebug>
#include <QThread>

#include "networkeventhandler.h"
#include "network.h"
#include "networkadapter.h"


NetworkEventHandler::NetworkEventHandler(Network *network,
					 bool register_agent,
					 QQmlContext *context,
					 QObject * parent) :
	QObject(parent),
	m_network(network),
	m_agent(register_agent)
{
	connman_add_manager_event_callback(manager_event_cb, this);
	connman_add_technology_property_event_callback(technology_event_cb, this);
	connman_add_service_property_event_callback(service_event_cb, this);
	if (register_agent)
		connman_add_agent_event_callback(agent_event_cb, this);
}

NetworkEventHandler::~NetworkEventHandler()
{
}

bool NetworkEventHandler::populateAdapterProperties(GVariant *in, QVariantMap &out)
{
	GVariantDict *props_dict = g_variant_dict_new(in);
	if (!props_dict)
		return false;

	gchar *p = NULL;
	gboolean value = FALSE;
        if (g_variant_dict_lookup(props_dict, "Connected", "b", &value))
		out.insert(QString("Connected"), QVariant((bool) value));

        if (g_variant_dict_lookup(props_dict, "Powered", "b", &value))
		out.insert(QString("Powered"), QVariant((bool) value));

	g_variant_dict_unref(props_dict);

	return true;
}

bool NetworkEventHandler::populateServiceProperties(GVariant *in, QVariantMap &out)
{
	GVariantDict *props_dict = g_variant_dict_new(in);
	if (!props_dict)
		return false;

	gchar *p = NULL;
        if (g_variant_dict_lookup(props_dict, "Type", "&s", &p))
		out.insert(QString("Type"), QVariant(QString(p)));

        if (g_variant_dict_lookup(props_dict, "Name", "&s", &p))
		out.insert(QString("Name"), QVariant(QString(p)));

        if (g_variant_dict_lookup(props_dict, "State", "&s", &p))
		out.insert(QString("State"), QVariant(QString(p)));

	guchar strength = 0;
        if (g_variant_dict_lookup(props_dict, "Strength", "y", &strength))
		out.insert(QString("Strength"), QVariant((int) strength));

	GVariant *v = NULL;
        if (g_variant_dict_lookup(props_dict, "IPv4", "@a{sv}", &v)) {
		GVariantDict *ip4_dict = g_variant_dict_new(v);
		QVariantMap ip4_map;

		if (g_variant_dict_lookup(ip4_dict, "Address", "&s", &p))
			ip4_map.insert(QString("Address"), QVariant(QString(p)));

		if (g_variant_dict_lookup(ip4_dict, "Netmask", "&s", &p))
			ip4_map.insert(QString("Netmask"), QVariant(QString(p)));

		if (g_variant_dict_lookup(ip4_dict, "Gateway", "&s", &p))
			ip4_map.insert(QString("Gateway"), QVariant(QString(p)));

		if (g_variant_dict_lookup(ip4_dict, "Method", "&s", &p))
			ip4_map.insert(QString("Method"), QVariant(QString(p)));

		g_variant_dict_unref(ip4_dict);
		g_variant_unref(v);

		if (!ip4_map.isEmpty())
			out.insert(QString("IPv4"), QVariant(ip4_map));
	}

        if (g_variant_dict_lookup(props_dict, "Nameservers", "&s", &v))
		out.insert(QString("Nameservers"), QVariant(QString(p)));

	GVariantIter *array = NULL;
        if (g_variant_dict_lookup(props_dict, "Security", "as", &array)) {
		QVariantList security;

		while (g_variant_iter_loop(array, "&s", &p))
			security.push_back(QVariant(QString(p)));

		if (!security.isEmpty())
			out.insert(QString("Security"), QVariant(security));

		g_variant_iter_free(array);
	}

	g_variant_dict_unref(props_dict);

	return !out.isEmpty();
}

void NetworkEventHandler::handle_connect_event(const gchar *service, gboolean status, const gchar *error)
{
	if (!status)
		qDebug() << "connect failed, error " << error;

	emit connectResponseReceived(QString(service), status, QString(error));	
}

void NetworkEventHandler::handle_manager_event(const gchar *path,
					       connman_manager_event_t event,
					       GVariant *properties)
{
#if LIBQTAPPFW_NETWORK_DEBUG
	QString props("NULL");
	QString event_str("");
	switch(event) {
	case CONNMAN_MANAGER_EVENT_TECHNOLOGY_ADD:
		event_str = "TECHNOLOGY_ADD";
		break;
	case CONNMAN_MANAGER_EVENT_TECHNOLOGY_REMOVE:
		event_str = "TECHNOLOGY_REMOVE";
		break;
	case CONNMAN_MANAGER_EVENT_SERVICE_CHANGE:
		event_str = "SERVICE_CHANGE";
		break;
	case CONNMAN_MANAGER_EVENT_SERVICE_REMOVE:
		event_str = "SERVICE_REMOVE";
		break;
	case CONNMAN_MANAGER_EVENT_PROPERTY_CHANGE:
		event_str = "PROPERTY_CHANGE";
		break;
	}
	if (event != CONNMAN_MANAGER_EVENT_TECHNOLOGY_REMOVE &&
	    event != CONNMAN_MANAGER_EVENT_SERVICE_REMOVE) {
		gchar *p = g_variant_print(properties, TRUE);
		props = p;
		g_free(p);
	}

	qDebug() << "NetworkEventHandler::handle_manager_event: received " <<
		event_str << ", path = " << path << ", properties = " << props;
#endif

	switch(event) {
	case CONNMAN_MANAGER_EVENT_TECHNOLOGY_ADD:
	case CONNMAN_MANAGER_EVENT_TECHNOLOGY_REMOVE:
		break;
	case CONNMAN_MANAGER_EVENT_SERVICE_CHANGE:
		{
			QVariantMap props_map;
			if (populateServiceProperties(properties, props_map)) {
				emit serviceAdded(path, props_map);
			}
		}
		break;
	case CONNMAN_MANAGER_EVENT_SERVICE_REMOVE:
		emit serviceRemoved(QString(path));
		break;
	case CONNMAN_MANAGER_EVENT_PROPERTY_CHANGE:
		break;
	default:
		break;
	}

}

void NetworkEventHandler::handle_technology_event(const gchar *technology,
						  GVariant *property)
{
	if (!(technology && property))
		return;

#if LIBQTAPPFW_NETWORK_DEBUG
	gchar *p = g_variant_print(property, TRUE);
	qDebug() << "NetworkEventHandler::handle_technology_event: technology = " <<
		technology << ", property = " << p;
	g_free(p);
#endif

	// Convert (sv) tuple to a a{sv} dict for parsing
	const gchar *key = NULL;
	GVariant *var = NULL;
	g_variant_get(property, "(sv)", &key, &var);
	if (!key)
		return;

	// Build QVariantMap for updateStatus
	QVariantMap props_map;
	if ((g_strcmp0(key, "Connected") == 0 ||
	     g_strcmp0(key, "Powered") == 0) &&
	    g_variant_is_of_type(var, G_VARIANT_TYPE_BOOLEAN)) {
		gboolean value;
		g_variant_get(var, "b", &value);
		props_map.insert(QString(key), QVariant((bool) value));

		// Update properties
		emit adapterStatusChanged(QString(technology), props_map);
	}
	g_variant_unref(var);
}

void NetworkEventHandler::handle_service_event(const gchar *service,
					       GVariant *property)
{
	if (!(service && property))
		return;

#if LIBQTAPPFW_NETWORK_DEBUG

	gchar *p = g_variant_print(property, TRUE);
	qDebug() << "NetworkEventHandler::handle_service_event: service = " <<
		service << ", property = " << p;
	g_free(p);
#endif
	// Convert (sv) tuple to a a{sv} dict for updateProperties
	const gchar *key = NULL;
	GVariant *var = NULL;
	g_variant_get(property, "(sv)", &key, &var);
	if (!key)
		return;
	GVariantDict *props_dict = g_variant_dict_new(NULL);
	if (!props_dict) {
		qWarning() << "g_variant_dict_new failed";
		return;
	}
	g_variant_dict_insert_value(props_dict, key, var);
	GVariant *props_var = g_variant_dict_end(props_dict);
	g_variant_dict_unref(props_dict);
	g_variant_unref(var);

	QVariantMap props_map;
	if (populateServiceProperties(props_var, props_map)) {
		// Update adapter properties
		emit servicePropertiesChanged(QString(service), props_map);
	}
	g_variant_unref(props_var);
}

void NetworkEventHandler::handle_agent_event(const gchar *service,
					     const int id,
					     GVariant *properties)
{
	if (!(service && properties))
		return;

#if LIBQTAPPFW_NETWORK_DEBUG
	gchar *props = g_variant_print(properties, TRUE);
	qDebug() << "Network::handle_agent_event: properties = " << props;
	g_free(props);
#endif

	GVariantDict *props_dict = g_variant_dict_new(properties);
	if (!props_dict)
		return;

	GVariant *v = NULL;
	QVariantMap props_map;
        if (g_variant_dict_lookup(props_dict, "Passphrase", "@a{sv}", &v)) {
		GVariantDict *passphrase_dict = g_variant_dict_new(v);
		gchar *p = NULL;

		if (g_variant_dict_lookup(passphrase_dict, "Type", "&s", &p))
			props_map.insert(QString("Type"), QVariant(QString(p)));

		if (g_variant_dict_lookup(passphrase_dict, "Requirement", "&s", &p))
			props_map.insert(QString("Requirement"), QVariant(QString(p)));

		g_variant_dict_unref(passphrase_dict);
		g_variant_unref(v);
	}
	g_variant_dict_unref(props_dict);

	if (!props_map.isEmpty())
		emit inputRequested(id, props_map);
}
