#ifndef WIFI_NETWORK_MODEL_H
#define WIFI_NETWORK_MODEL_H

#include "abstractnetworkmodel.h"

class WifiNetworkModel : public AbstractNetworkModel
{
	Q_OBJECT

public:
        enum WifiNetworkRoles {
		AddressRole = Qt::UserRole + 1,
		SecurityRole,
		ServiceRole,
		StateRole,
		SsidRole,
		StrengthRole
        };

        WifiNetworkModel(QObject *parent = Q_NULLPTR);

        QString getType() const override { return "wifi"; }
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        void updateProperties(const QString &service, const QVariantMap &properties) override;

signals:
        void strengthChanged(int strength);

protected:
        QHash<int, QByteArray> roleNames() const;

};
#endif // WIFI_NETWORK_MODEL_H
