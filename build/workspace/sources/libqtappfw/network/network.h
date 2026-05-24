 /*
 * Copyright (C) 2018-2021 Konsulko Group
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

#ifndef NETWORK_H
#define NETWORK_H

#include <memory>
#include <QObject>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlListProperty>

class NetworkEventHandler;
class AdapterIf;

class Network : public QObject
{
	Q_OBJECT

public:
	explicit Network(bool register_agent, QQmlContext *context, QObject * parent = Q_NULLPTR);
	virtual ~Network();

	Q_INVOKABLE void connect(QString service);
	Q_INVOKABLE void disconnect(QString service);
	Q_INVOKABLE void remove(QString service);
	Q_INVOKABLE void power(bool on, QString type = "wifi");
	Q_INVOKABLE void input(int id, QString passphrase);
	Q_INVOKABLE void configureAddress(QString service, QVariantList paramlist);
	Q_INVOKABLE void configureNameServer(QString service, QVariantList paramlist);

	AdapterIf* findAdapter(const QString &technology);
	void getServices();

signals:
	void inputRequest(int id);
	void invalidPassphrase(const QString &service);
	void searchResults(const QString &name);

private:
	NetworkEventHandler *m_event_handler;
	bool m_agent;
	QList<AdapterIf*> m_adapters;

	void scanServices(const QString &technology);
	void disableTechnology(const QString &technology);
	void enableTechnology(const QString &technology);
	void getTechnologies();

	friend class NetworkEventHandler;

private slots:
	// Invoked by NetworkEventHandler signals to drive model updates on UI thread
	void updateAdapterStatus(const QString &technology, const QVariantMap &properties);
	void updateServiceProperties(const QString &service, const QVariantMap &properties);
	void addService(const QString &service, const QVariantMap &properties);
	void removeService(const QString &service);
	void requestInput(int id, const QVariantMap &properties);
	void handleConnectResponse(const QString &service, bool status, const QString &error);
};

#endif // NETWORK_H
