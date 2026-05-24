#ifndef WIRED_NETWORK_MODEL_H
#define WIRED_NETWORK_MODEL_H

#include "abstractnetworkmodel.h"

class WiredNetworkModel : public AbstractNetworkModel
{
	Q_OBJECT

public:
        enum WiredNetworkRoles {
		AddressRole = Qt::UserRole + 1,
		SecurityRole,
		ServiceRole,
		StateRole,
		RouteRole,
		NameServerRole,
        };

        WiredNetworkModel(QObject *parent = Q_NULLPTR);

        Q_INVOKABLE QVariantList readCurrentRouteConfig(const QModelIndex &index) const;
        Q_INVOKABLE QVariantList readCurrentNameServerConfig(const QModelIndex &index) const;
        QString getType() const override { return "wired"; }
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        void updateProperties(const QString &service, const QVariantMap &properties) override;

protected:
        QHash<int, QByteArray> roleNames() const;

};
#endif // WIRED_NETWORK_MODEL_H
