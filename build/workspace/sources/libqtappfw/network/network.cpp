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
#include <QtQml/QQmlEngine>
#include <QThread>
#include <connman-glib.h>

#include "wifiadapter.h"
#include "wiredadapter.h"
#include "network.h"
#include "networkeventhandler.h"

Network::Network(bool register_agent, QQmlContext *context, QObject * parent) :
	QObject(parent),
	m_agent(register_agent)
{
	m_adapters.append(new WifiAdapter(this, context, parent));
	m_adapters.append(new WiredAdapter(this, context, parent));

	m_event_handler = new NetworkEventHandler(this, register_agent, context, parent);

	// Hook up signals so updates can be made to happen from the UI thread as Qt requires
	QObject::connect(m_event_handler, &NetworkEventHandler::adapterStatusChanged, this, &Network::updateAdapterStatus, Qt::QueuedConnection);
	QObject::connect(m_event_handler, &NetworkEventHandler::servicePropertiesChanged, this, &Network::updateServiceProperties, Qt::QueuedConnection);
	QObject::connect(m_event_handler, &NetworkEventHandler::serviceAdded, this, &Network::addService, Qt::QueuedConnection);
	QObject::connect(m_event_handler, &NetworkEventHandler::serviceRemoved, this, &Network::removeService, Qt::QueuedConnection);
	QObject::connect(m_event_handler, &NetworkEventHandler::inputRequested, this, &Network::requestInput, Qt::QueuedConnection);
	QObject::connect(m_event_handler, &NetworkEventHandler::connectResponseReceived, this, &Network::handleConnectResponse, Qt::QueuedConnection);

	connman_init(register_agent);

	getTechnologies();
}

Network::~Network()
{
	while (!m_adapters.isEmpty())
		m_adapters.takeLast();
}

void Network::connect(QString service)
{
	QByteArray service_ba = service.toLocal8Bit();
	const char *service_cstr = service_ba.data();

	connman_service_connect(service_cstr, m_event_handler->service_connect_cb, m_event_handler);
}

void Network::disconnect(QString service)
{
	QByteArray service_ba = service.toLocal8Bit();
	const char *service_cstr = service_ba.data();

	connman_service_disconnect(service_cstr);
}

void Network::remove(QString service)
{
	QByteArray service_ba = service.toLocal8Bit();
	const char *service_cstr = service_ba.data();

	connman_service_remove(service_cstr);
}

void Network::power(bool on, QString type)
{
	if (on)
		enableTechnology(type);
	else
		disableTechnology(type);
}

void Network::input(int id, QString passphrase)
{
	QByteArray passphrase_ba = passphrase.toLocal8Bit();
	const char *passphrase_cstr = passphrase_ba.data();

	// e.g. ({'Passphrase': <'foo'>},)
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_add(&builder,
			      "{sv}",
			      "Passphrase",
			      g_variant_new_string(passphrase_cstr));

	// Wrap in required tuple
	GVariantBuilder builder2;
	g_variant_builder_init(&builder2, G_VARIANT_TYPE_TUPLE);
	g_variant_builder_add_value(&builder2,
				    g_variant_builder_end(&builder));
	GVariant *parameters = g_variant_builder_end(&builder2);

	if (parameters) {
		connman_agent_response(id, parameters);
	} else {
		qWarning() << "Could not build response for input request " << id;
	}
}

void Network::configureAddress(QString service, QVariantList paramlist)
{
	if (service.isEmpty() || paramlist.size() < 4) {
		qWarning("Invalid addressing params");
		return;
	}

	QByteArray path_ba = service.toLocal8Bit();

	GVariantDict *params_dict = g_variant_dict_new(NULL);
	if (!params_dict) {
		qWarning() << "g_variant_dict_new failed";
		return;
	}

	QByteArray param_ba = paramlist[0].toString().toLocal8Bit();
	g_variant_dict_insert(params_dict, "Method", "s", param_ba.data());
	param_ba = paramlist[1].toString().toLocal8Bit();
	g_variant_dict_insert(params_dict, "Address", "s", param_ba.data());
	param_ba = paramlist[2].toString().toLocal8Bit();
	g_variant_dict_insert(params_dict, "Netmask", "s", param_ba.data());
	param_ba = paramlist[3].toString().toLocal8Bit();
	g_variant_dict_insert(params_dict, "Gateway", "s", param_ba.data());
	GVariant *parameters = g_variant_dict_end(params_dict);
	g_variant_dict_unref(params_dict);

	// Need to end up with something like:
	// - path = ethernet_525400123502_cable
	// - name = IPv4.Configuration
	// - parameters = {'Method': <'manual'>, 'Gateway': <'10.0.2.2'>, 'Address': <'10.0.2.15'>, 'Netmask': <'255.255.255'>}
	if (!connman_set_property(CONNMAN_PROPERTY_SERVICE,
				  path_ba.data(),
				  "IPv4.Configuration",
				  parameters)) {
		qWarning() << "Could not configure address";
	}
}

void Network::configureNameServer(QString service, QVariantList paramlist)
{
	// paramlist will be 'manual|auto', 'server1'...
	// NOTE: currently unclear what should happen for 'auto' case
	if (service.isEmpty() || paramlist.size() < 2) {
		qWarning("Invalid nameserver params");
		return;
	}

	QByteArray path_ba = service.toLocal8Bit();

	QStringList nslist = paramlist[1].toString().split(' ');
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));
	QStringList::const_iterator iter;
	for (iter = nslist.constBegin(); iter != nslist.constEnd(); ++iter) {
		g_variant_builder_add(&builder,
				      "s",
				      (*iter).toLocal8Bit().constData());
	}
	GVariant *parameters = g_variant_builder_end(&builder);

	// Need to end up with something like:
	// - path = ethernet_525400123502_cable
	// - name = Nameservers.Configuration
	// - value = ['8.8.8.8.8'] 
	if (!connman_set_property(CONNMAN_PROPERTY_SERVICE,
				  path_ba.data(),
				  "Nameservers.Configuration",
				  parameters)) {
		qWarning() << "Could not configure nameserver";
	}
}

void Network::getServices()
{
	GVariant *reply = NULL;
	if(!connman_get_services(&reply)) {
		qWarning() << "Network::getServices: connman_get_services failed!";
		return;
	}

	GVariantIter *array = NULL;
	g_variant_get(reply, "(a(oa{sv}))", &array);
	const gchar *path = NULL;
	GVariant *var = NULL;
	while (g_variant_iter_next(array, "(o@a{sv})", &path, &var)) {
		QString id = path;
		id = id.mid(id.lastIndexOf("/") + 1);
		QVariantMap props_map;
		if (m_event_handler->populateServiceProperties(var, props_map)) {
			QList<AdapterIf*>::iterator iter;
			for (iter = m_adapters.begin(); iter != m_adapters.end(); ++iter)
				if ((*iter)->addService(id, props_map))
					break;
		}
		g_variant_unref(var);
	}
	g_variant_iter_free(array);
	g_variant_unref(reply);
}

AdapterIf* Network::findAdapter(const QString &type)
{
	QList<AdapterIf*>::iterator iter;
	for (iter = m_adapters.begin(); iter != m_adapters.end(); ++iter)
		if  ((*iter)->getType() == type)
			return (*iter);
	return nullptr;
}

void Network::scanServices(const QString &technology)
{
	QByteArray technology_ba = technology.toLocal8Bit();
	const char *technology_cstr = technology_ba.data();

	connman_technology_scan_services(technology_cstr);
}

void Network::disableTechnology(const QString &technology)
{
	QByteArray technology_ba = technology.toLocal8Bit();
	const char *technology_cstr = technology_ba.data();

	connman_technology_disable(technology_cstr);
}

void Network::enableTechnology(const QString &technology)
{
	QByteArray technology_ba = technology.toLocal8Bit();
	const char *technology_cstr = technology_ba.data();

	connman_technology_enable(technology_cstr);
}

void Network::getTechnologies()
{
	GVariant *reply = NULL;
	if(!connman_get_technologies(&reply))
		return;

	GVariantIter *array = NULL;
	g_variant_get(reply, "(a(oa{sv}))", &array);
	if (!array) {
		g_variant_unref(reply);
		return;
	}
	const gchar *path = NULL;
	GVariant *properties = NULL;
	while (g_variant_iter_next(array, "(o@a{sv})", &path, &properties)) {
		GVariantDict *props_dict = g_variant_dict_new(properties);
		gchar *type_cstr = NULL;
		if (g_variant_dict_lookup(props_dict, "Type", "s", &type_cstr)) {
			QString type(type_cstr);
			AdapterIf* adapter = findAdapter(type);
			if (adapter) {
				QVariantMap props_map;
				if (m_event_handler->populateAdapterProperties(properties, props_map))
					adapter->updateStatus(props_map);
			}
		}
		g_variant_dict_unref(props_dict);
		g_variant_unref(properties);
	}
	g_variant_iter_free(array);
	g_variant_unref(reply);
}

void Network::updateAdapterStatus(const QString &technology, const QVariantMap &properties)
{
        AdapterIf* adapter = findAdapter(technology);
        if (adapter)
		adapter->updateStatus(properties);
}

void Network::updateServiceProperties(const QString &service, const QVariantMap &properties)
{
	QList<AdapterIf*>::iterator iter;
	for (iter = m_adapters.begin(); iter != m_adapters.end(); ++iter)
		(*iter)->updateProperties(service, properties);
}

void Network::addService(const QString &service, const QVariantMap &properties)
{
	QList<AdapterIf*>::iterator iter;
	for (iter = m_adapters.begin(); iter != m_adapters.end(); ++iter)
		if ((*iter)->addService(service, properties))
		  break;
}

void Network::removeService(const QString &service)
{
	QList<AdapterIf*>::iterator iter;
	for (iter = m_adapters.begin(); iter != m_adapters.end(); ++iter)
		(*iter)->removeService(service);
}

void Network::requestInput(int id, const QVariantMap &properties)
{
	QString type;
	QString key = "Type";
        if (properties.contains(key))
		type = properties.value(key).toString();

	QString reqmt;
	key = "Requirement";
        if (properties.contains(key))
		reqmt = properties.value(key).toString();

        if (((type == "psk") || (type == "wep")) && (reqmt == "mandatory")) {
		emit inputRequest(id);
        }
}

void Network::handleConnectResponse(const QString &service, bool status, const QString &error)
{
	if (!status && error.contains("invalid-key")) {
		emit invalidPassphrase(service);
	}
}
