#include "abstractnetworkmodel.h"
#include "connectionprofile.h"
#include <QDebug>


AbstractNetworkModel::AbstractNetworkModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void AbstractNetworkModel::addNetwork(ConnectionProfile *network)
{
	int start_rows = rowCount();
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_networks.insert(rowCount(), network);
	endInsertRows();
}

void AbstractNetworkModel::removeNetwork(ConnectionProfile *network)
{
	if (m_networks.isEmpty() || (network == nullptr))
		return;

	int row = m_networks.indexOf(network);
	beginRemoveRows(QModelIndex(), row, row);
	m_networks.removeAt(row);
	endRemoveRows();
	delete network;
}

void AbstractNetworkModel::removeAllNetworks()
{
	if (m_networks.isEmpty())
		return;

	beginRemoveRows(QModelIndex(), 0, m_networks.count() - 1);
	qDeleteAll(m_networks.begin(), m_networks.end());
	endRemoveRows();
	m_networks.clear();
}

ConnectionProfile *AbstractNetworkModel::getNetwork(QString service)
{
	if (m_networks.isEmpty())
		return nullptr;

	for (auto network : m_networks) {
		if (network->service() == service)
			return network;
	}

	return nullptr;
}

int AbstractNetworkModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_networks.count();
}

QModelIndex AbstractNetworkModel::indexOf(ConnectionProfile *network)
{
	int row = m_networks.indexOf(network);

	return index(row);
}
