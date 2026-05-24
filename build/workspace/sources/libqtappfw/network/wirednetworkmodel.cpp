#include "wirednetworkmodel.h"
#include "connectionprofile.h"
#include <QVector>
#include <QDebug>

WiredNetworkModel::WiredNetworkModel(QObject *parent)
    : AbstractNetworkModel(parent)
{
}

QVariantList WiredNetworkModel::readCurrentRouteConfig(const QModelIndex &index) const
{
	QVariantList ret;

	if (!index.isValid())
		return ret;

	if (index.row() < 0 || index.row() >= this->m_networks.count())
		return ret;

	const ConnectionProfile *network = this->m_networks[index.row()];
	ret.append(network->addrmethod());
	ret.append(network->address());
	ret.append(network->netmask());
	ret.append(network->gateway());
	return ret;
}

QVariantList WiredNetworkModel::readCurrentNameServerConfig(const QModelIndex &index) const
{
	QVariantList ret;

	if (!index.isValid())
		return ret;

	if (index.row() < 0 || index.row() >= this->m_networks.count())
		return ret;

	const ConnectionProfile *network = this->m_networks[index.row()];
	ret.append(network->nsmethod());
	ret.append(network->nameservers());
	return ret;
}

QVariant WiredNetworkModel::data(const QModelIndex &index, int role) const
{
	QVariant ret;

	if (!index.isValid())
		return ret;

	if (index.row() < 0 || index.row() >= m_networks.count())
		return ret;

	const ConnectionProfile *network = m_networks[index.row()];

	switch (role) {
        case AddressRole:
		return network->address();
        case SecurityRole:
		return network->security();
        case ServiceRole:
		return network->service();
        case StateRole:
		return network->state();
        case RouteRole:
		return readCurrentRouteConfig(index);
        case NameServerRole:
		return readCurrentNameServerConfig(index);
	}

	return ret;
}

QHash<int, QByteArray> WiredNetworkModel::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[AddressRole] = "address";
	roles[SecurityRole] = "security";
	roles[ServiceRole] = "service";
	roles[StateRole] = "sstate";
	roles[RouteRole] = "sroute";
	roles[NameServerRole] = "nservers";

	return roles;
}

void WiredNetworkModel::updateProperties(const QString &service, const QVariantMap &properties)
{
	ConnectionProfile *network;
	QVector<int> vroles;

	network = getNetwork(service);
	if (!network)
		return;

	QString key = "IPv4";
        if (properties.contains(key)) {
		QVariantMap ip4_map = properties.value(key).toMap();

		QString address;
		key = "Address";
		if (ip4_map.contains(key))
			address = ip4_map.value(key).toString();

		QString netmask;
		key = "Netmask";
		if (ip4_map.contains(key))
			netmask = ip4_map.value(key).toString();

		QString gateway;
		key = "Gateway";
		if (ip4_map.contains(key))
			gateway = ip4_map.value(key).toString();

		QString method;
		key = "Method";
		if (ip4_map.contains(key))
			method = ip4_map.value(key).toString();

		network->setAddress(address);
		network->setNetmask(netmask);
		network->setGateway(gateway);
		network->setAddrMethod(method);
		vroles.push_back(AddressRole);
		vroles.push_back(RouteRole);
	}

	key = "Nameservers";
        if (properties.contains(key)) {
		QString ns = properties.value(key).toString();
		network->setNameservers(ns);
		(network->addrmethod() == "dhcp") ? network->setNSMethod("auto") :
			network->setNSMethod("manual");
		vroles.push_back(NameServerRole);
	}

	key = "State";
        if (properties.contains(key)) {
		QString state = properties.value(key).toString();
		network->setState(state);
		vroles.push_back(StateRole);
	}

	if (!vroles.isEmpty())
		emit dataChanged(indexOf(network), indexOf(network), vroles);
}
