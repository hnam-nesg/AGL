/*
 * Copyright (C) 2023 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef QT_KUKSA_CLIENT_H
#define QT_KUKSA_CLIENT_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <grpcpp/grpcpp.h>
#include "kuksa/val/v1/val.grpc.pb.h"

// Just pull in the whole namespace since Datapoint contains a lot of
// definitions that may potentially be needed.
using namespace kuksa::val::v1;

using grpc::Status;

#include "VehicleSignalsConfig.h"

// KUKSA.val databroker gRPC API Qt client class

class QtKuksaClient : public QObject
{
	Q_OBJECT

public:
	explicit QtKuksaClient(const std::shared_ptr< ::grpc::ChannelInterface>& channel,
			       const VehicleSignalsConfig &config,
			       QObject * parent = Q_NULLPTR);

	Q_INVOKABLE void connect();

	Q_INVOKABLE void get(const QString &path, const bool actuator = false);

	Q_INVOKABLE void set(const QString &path, const QString &value, const bool actuator = false);
	Q_INVOKABLE void set(const QString &path, const qint32 value, const bool actuator = false);
	Q_INVOKABLE void set(const QString &path, const qint64 value, const bool actuator = false);
	Q_INVOKABLE void set(const QString &path, const quint32 value, const bool actuator = false);
	Q_INVOKABLE void set(const QString &path, const quint64 value, const bool actuator = false);
	Q_INVOKABLE void set(const QString &path, const float value, const bool actuator = false);
	Q_INVOKABLE void set(const QString &path, const double value, const bool actuator = false);
  
	Q_INVOKABLE void subscribe(const QString &path, const bool actuator = false);
	Q_INVOKABLE void subscribe(const QMap<QString, bool> &signals_);

	//// add
	Q_INVOKABLE void set(const QString &path, const bool value, const bool actuator = false);

signals:
	void connected();
	void getResponse(QString path, QString value, QString timestamp);
        void setResponse(QString path, QString error);
        void subscribeResponse(QString path, QString value, QString timestamp);
        void subscribeDone(const QMap<QString, bool> &signals_, bool canceled);

private:
	class SubscribeReader;

	std::shared_ptr< ::grpc::ChannelInterface> m_channel;
	VehicleSignalsConfig m_config;
	std::shared_ptr<VAL::Stub> m_stub;
	bool m_connected;
	QMutex m_connected_mutex;

	void waitForConnected();

	void set(const QString &path, const Datapoint &dp, const bool actuator);

	void handleGetResponse(const GetResponse *response);

	void handleSetResponse(const SetResponse *response);

	void handleSubscribeResponse(const SubscribeResponse *response);

	void handleSubscribeDone(const QMap<QString, bool> &signals_, const Status &status);

	void handleCriticalFailure(const QString &error);
	void handleCriticalFailure(const char *error) { handleCriticalFailure(QString(error)); };

	void resubscribe(const QMap<QString, bool> &signals_);

	bool convertDatapoint(const Datapoint &dp, QString &value, QString &timestamp);
};

#endif // QT_KUKSA_CLIENT_H
