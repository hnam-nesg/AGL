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

#ifndef WIREDADAPTER_H
#define WIREDADAPTER_H

#include <QDebug>
#include <QObject>
#include <QtQml/QQmlContext>
#include "networkadapter.h"

class Network;
class WiredNetworkModel;

class WiredAdapter : public QObject, public AdapterIf
{
	Q_OBJECT
	Q_INTERFACES(AdapterIf)
	public:
        explicit WiredAdapter(Network *network, QQmlContext *context, QObject *parent);
        virtual ~WiredAdapter();

        bool wiredConnected() const { return m_wiredConnected; }
        bool wiredEnabled() const { return m_wiredEnabled; }

        bool addService(const QString &id, const QVariantMap &properties) override;
        void removeService(const QString &id) override;
        void updateProperties(const QString &service, const QVariantMap &properties) override;

        QString getType() override { return "ethernet"; }
        void updateStatus(const QVariantMap &properties) override;

signals:
        void wiredConnectedChanged(bool connected);
        void wiredEnabledChanged(bool enabled);

private:
        bool m_wiredConnected;
        bool m_wiredEnabled;
        WiredNetworkModel *m_model;
        Network *nw;
};

#endif // WIREDADAPTER_H
