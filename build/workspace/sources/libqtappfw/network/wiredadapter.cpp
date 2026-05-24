/*
 * Copyright (C) 2019,2022 Konsulko Group
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

#include <QMetaEnum>
#include <QSortFilterProxyModel>
#include <QtQml/QQmlEngine>

#include "network.h"
#include "wiredadapter.h"
#include "wirednetworkmodel.h"
#include "connectionprofile.h"

WiredAdapter::WiredAdapter(Network *network, QQmlContext *context, QObject *parent) :
	QObject(parent),
	AdapterIf(),
	m_wiredConnected(false),
	m_wiredEnabled(false),
	m_model(nullptr),
	nw(network)
{
	m_model = new WiredNetworkModel();
	QSortFilterProxyModel *model = new QSortFilterProxyModel();
	model->setSourceModel(m_model);
	model->setSortRole(WiredNetworkModel::WiredNetworkRoles::ServiceRole);
	model->setSortCaseSensitivity(Qt::CaseInsensitive);
	model->sort(0);

	context->setContextProperty("WiredNetworkModel", m_model);
	context->setContextProperty("WiredAdapter", this);
}

WiredAdapter::~WiredAdapter()
{
	delete m_model;
}

void WiredAdapter::updateStatus(const QVariantMap &properties)
{
	QString key = "Connected";
        if (properties.contains(key)) {
		m_wiredConnected = properties.value(key).toBool();
		emit wiredConnectedChanged(m_wiredConnected);
	}

	key = "Powered";
        if (properties.contains(key)) {
		m_wiredEnabled = properties.value(key).toBool();
		emit wiredEnabledChanged(m_wiredEnabled);
		if (m_wiredEnabled)
			nw->getServices();
	}
}

void WiredAdapter::updateProperties(const QString &id, const QVariantMap &properties)
{
	if (m_model->getNetwork(id))
		m_model->updateProperties(id, properties);
}

bool WiredAdapter::addService(const QString &id, const QVariantMap &properties)
{
	QString type;
	QString key = "Type";
        if (properties.contains(key)) {
		type = properties.value(key).toString();
		if (type != getType())
			return false;
	}

	// Ignore services already added
	if (m_model->getNetwork(id))
		return false;

	QString state;
	key = "State";
        if (properties.contains(key))
		state = properties.value(key).toString();

	// Initially support only IPv4 and the first security method found
	QString address;
	QString netmask;
	QString gateway;
	QString amethod;
	key = "IPv4";
        if (properties.contains(key)) {
		QVariantMap ip4_map = properties.value(key).toMap();

		key = "Address";
		if (ip4_map.contains(key))
			address = ip4_map.value(key).toString();

		key = "Netmask";
		if (ip4_map.contains(key))
			netmask = ip4_map.value(key).toString();

		key = "Gateway";
		if (ip4_map.contains(key))
			gateway = ip4_map.value(key).toString();

		key = "Method";
		if (ip4_map.contains(key))
			amethod = ip4_map.value(key).toString();
	}

	QString ns;
	key = "Nameservers";
        if (properties.contains(key))
		ns = properties.value(key).toString();
	QString nsmethod = (amethod == "dhcp")? "auto" : "manual";

	QString security;
	key = "Security";
        if (properties.contains(key)) {
		QVariantList security_list = properties.value(key).toList();

		if (!security_list.isEmpty())
			security = security_list[0].toString();
	}

#if LIBQTAPPFW_NETWORK_DEBUG
	qDebug() << "WiredAdapter::addService: address = " << address
		 << ", id = " << id
		 << ", state = " << state
		 << ", netmask = " << netmask
		 << ", gateway = " << gateway
		 << ", address method = " << amethod
		 << ", nameservers = " << ns
		 << ", nameserver method = " << nsmethod
		 << ", security = " << security;
#endif
	ConnectionProfile *network = new ConnectionProfile(address, security, id,
							   state, "", 0, netmask,
							   gateway, amethod, ns,
							   nsmethod);
	m_model->addNetwork(network);

	return true;
}

void WiredAdapter::removeService(const QString &id)
{
	m_model->removeNetwork(m_model->getNetwork(id));
}
