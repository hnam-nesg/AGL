#ifndef ABSTRACT_NETWORK_MODEL_H
#define ABSTRACT_NETWORK_MODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QtQml/QQmlContext>

#include "connectionprofile.h"

class AbstractNetworkModel : public QAbstractListModel
{
	Q_OBJECT

public:
        AbstractNetworkModel(QObject *parent = Q_NULLPTR);
        virtual QString getType() const = 0;

        void addNetwork(ConnectionProfile *network);
        void removeNetwork(ConnectionProfile *network);
        void removeAllNetworks();
        ConnectionProfile *getNetwork(QString service);
        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const = 0;
        virtual void updateProperties(const QString &service, const QVariantMap &properties) = 0;

protected:
        QList<ConnectionProfile *> m_networks;
        QModelIndex indexOf(ConnectionProfile *network);
};

#endif // ABSTRACT_NETWORK_MODEL_H
