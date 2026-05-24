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
#include "wifiadapter.h"
#include "wifinetworkmodel.h"
#include "connectionprofile.h"

WifiAdapter::WifiAdapter(Network *network, QQmlContext *context, QObject *parent) :
	QObject(parent),
	AdapterIf(),
	m_wifiConnected(false),
	m_wifiEnabled(false),
	m_wifiStrength(0),
	m_model(nullptr),
	nw(network)
{
	m_model = new WifiNetworkModel();
	QSortFilterProxyModel *model = new QSortFilterProxyModel();
	model->setSourceModel(m_model);
	model->setSortRole(WifiNetworkModel::WifiNetworkRoles::SsidRole);
	model->setSortCaseSensitivity(Qt::CaseInsensitive);
	model->sort(0);

	context->setContextProperty("WifiNetworkModel", m_model);
	QObject::connect(m_model, &WifiNetworkModel::strengthChanged, this, &WifiAdapter::updateWifiStrength);
	context->setContextProperty("WifiAdapter", this);
}

WifiAdapter::~WifiAdapter()
{
	delete m_model;
}

void WifiAdapter::updateStatus(const QVariantMap &properties)
{
	QString key = "Connected";
        if (properties.contains(key)) {
		m_wifiConnected = properties.value(key).toBool();
		emit wifiConnectedChanged(m_wifiConnected);
	}

	key = "Powered";
        if (properties.contains(key)) {
		m_wifiEnabled = properties.value(key).toBool();
		emit wifiEnabledChanged(m_wifiEnabled);
		if (m_wifiEnabled)
			nw->getServices();
	}
}

void WifiAdapter::updateWifiStrength(int strength)
{
	m_wifiStrength = strength;
	emit wifiStrengthChanged(m_wifiStrength);
}

void WifiAdapter::updateProperties(const QString &id, const QVariantMap &properties)
{
	if (m_model->getNetwork(id))
		m_model->updateProperties(id, properties);
}

bool WifiAdapter::addService(const QString &id, const QVariantMap &properties)
{
	QString type;
	QString key = "Type";
        if (properties.contains(key)) {
		type = properties.value(key).toString();
		if (type != getType())
			return false;
	}

	QString ssid;
	key = "Name";
        if (properties.contains(key))
		ssid = properties.value(key).toString();

	// Ignore hidden SSIDs or services already added
	if (m_model->getNetwork(id) || (ssid == ""))
		return false;

	QString state;
	key = "State";
        if (properties.contains(key))
		state = properties.value(key).toString();

	int strength = 0;
	key = "Strength";
        if (properties.contains(key))
		strength = properties.value(key).toInt();

	// Initially support only IPv4 and the first security method found
	QString address;
	key = "IPv4";
        if (properties.contains(key)) {
		QVariantMap ip4_map = properties.value(key).toMap();

		key = "Address";
		if (ip4_map.contains(key))
			address = ip4_map.value(key).toString();
	}

	QString security;
	key = "Security";
        if (properties.contains(key)) {
		QVariantList security_list = properties.value(key).toList();

		if (!security_list.isEmpty())
			security = security_list[0].toString();
	}

#if LIBQTAPPFW_NETWORK_DEBUG
	qDebug() << "WifiAdapter::addService: address = " << address
		 << ", id = " << id
		 << ", state = " << state
		 << ", ssid = " << ssid
		 << ", strength = " << strength
		 << ", security = " << security;
#endif
	ConnectionProfile *network = new ConnectionProfile(address, security, id, state, ssid,
							   strength, "", "", "", "", "");
	m_model->addNetwork(network);

	if ((state == "ready") || (state == "online"))
		updateWifiStrength(strength);

	return true;
}

void WifiAdapter::removeService(const QString &id)
{
	m_model->removeNetwork(m_model->getNetwork(id));
}
