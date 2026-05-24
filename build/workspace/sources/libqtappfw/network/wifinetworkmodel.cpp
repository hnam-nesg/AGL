#include "wifinetworkmodel.h"
#include "connectionprofile.h"
#include <QVector>
#include <QDebug>

WifiNetworkModel::WifiNetworkModel(QObject *parent)
    : AbstractNetworkModel(parent)
{
}

QVariant WifiNetworkModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_networks.count())
		return QVariant();

	ConnectionProfile *network = m_networks[index.row()];

	switch (role) {
        case AddressRole:
		return network->address();
        case SecurityRole:
		return network->security();
        case ServiceRole:
		return network->service();
        case SsidRole:
		return network->ssid();
        case StateRole:
		return network->state();
        case StrengthRole:
		return network->strength();
	}

	return QVariant();
}

QHash<int, QByteArray> WifiNetworkModel::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[AddressRole] = "address";
	roles[SecurityRole] = "security";
	roles[ServiceRole] = "service";
	roles[SsidRole] = "ssid";
	roles[StateRole] = "sstate";
	roles[StrengthRole] = "strength";

	return roles;
}

void WifiNetworkModel::updateProperties(const QString &service, const QVariantMap &properties)
{
	ConnectionProfile *network;
	QVector<int> vroles;
	bool sbcast = false;

	network = getNetwork(service);
	if (!network)
		return;

	QString key = "IPv4";
        if (properties.contains(key)) {
		QVariantMap ip4_map = properties.value(key).toMap();

		key = "Address";
		if (ip4_map.contains(key)) {
			QString address = ip4_map.value(key).toString();
			network->setAddress(address);
			vroles.push_back(AddressRole);
		}
	}

	key = "State";
        if (properties.contains(key)) {
		QString state = properties.value(key).toString();
		network->setState(state);
		vroles.push_back(StateRole);
		if ((network->state() == "ready") ||
		    (network->state() == "online"))
			sbcast = true;
	}

	key = "Strength";
        if (properties.contains(key)) {
		int strength =  properties.value(key).toInt();
		network->setStrength(strength);
		vroles.push_back(StrengthRole);
		if ((network->state() == "ready") ||
		    (network->state() == "online"))
			sbcast = true;
	}

	if (!vroles.isEmpty()) {
		emit dataChanged(indexOf(network), indexOf(network), vroles);
		if (sbcast)
			emit strengthChanged(network->strength());
	}
}
