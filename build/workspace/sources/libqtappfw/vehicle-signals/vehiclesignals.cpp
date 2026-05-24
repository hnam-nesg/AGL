/*
 * Copyright (C) 2022,2023 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <QDebug>
#include <QtConcurrent>

#include "vehiclesignals.h"
#include "QtKuksaClient.h"

VehicleSignals::VehicleSignals(const VehicleSignalsConfig &config, QObject *parent) :
	QObject(parent),
	m_config(config)
{
	// Create gRPC channel
	// NOTE: channel creation and waiting for connected state could be put into
	//       a thread that is spawned here.

  	QString host = m_config.hostname();
	host += ":";
	host += QString::number(m_config.port());

	std::shared_ptr<grpc::Channel> channel;
	if (m_config.useTls()) {
		qInfo() << "Using TLS";
		if (!m_config.caCert().isEmpty()) {
			qInfo() << "Using CA certificate " << m_config.caCertFileName();
			grpc::SslCredentialsOptions options;
			options.pem_root_certs = m_config.caCert().toStdString();
			if (!m_config.tlsServerName().isEmpty()) {
				grpc::ChannelArguments args;
				auto target = m_config.tlsServerName();
				qInfo() << "Overriding TLS server name with " << target;
				args.SetString(GRPC_SSL_TARGET_NAME_OVERRIDE_ARG, target.toStdString());
				channel = grpc::CreateCustomChannel(host.toStdString(), grpc::SslCredentials(options), args);
			} else {
				channel = grpc::CreateChannel(host.toStdString(), grpc::SslCredentials(options));
			}
		} else {
			channel = grpc::CreateChannel(host.toStdString(), grpc::SslCredentials(grpc::SslCredentialsOptions()));
		}
	} else {
		channel = grpc::CreateChannel(host.toStdString(), grpc::InsecureChannelCredentials());
	}

	m_broker = new QtKuksaClient(channel, config);
	if (!m_broker)
		qCritical() << "gRPC client initialization failed";

	QObject::connect(m_broker, &QtKuksaClient::connected, this, &VehicleSignals::onConnected);
}

VehicleSignals::~VehicleSignals()
{
	delete m_broker;
}

void VehicleSignals::connect()
{
	// QtKuksaClient will call our onConnected slot when the channel
	// is connected, and then we pass that along via our connected
	// signal.
	qInfo() << "[TRAP]" << QCoreApplication::applicationPid() << this;
	qInfo() << "[TRAP] VehicleSignalsConfig appname =" << qAppName();
	QSettings *pSettings = new QSettings("AGL", qAppName());
	qInfo() << "[TRAP] settings fileName =" << pSettings->fileName();
	qInfo() << "[TRAP] allKeys =" << pSettings->allKeys();
	qInfo() << "[TRAP] hostname raw =" << pSettings->value("kuksa-client/hostname");
	qInfo() << "[TRAP] use-tls raw =" << pSettings->value("kuksa-client/use-tls");
	qInfo() << "[TRAP] auth raw =" << pSettings->value("kuksa-client/authorization").toString();
	qInfo() << "[TRAP] auth exists =" << QFile::exists(pSettings->value("kuksa-client/authorization").toString());
	if (m_broker)
		m_broker->connect();
}

void VehicleSignals::authorize()
{
	// Databroker has no separate authorize call, so this is a no-op
	emit authorized();
}

void VehicleSignals::set(const QString &path, const bool value, const bool actuator)
{
    if (m_broker)
		m_broker->set(path, value, actuator);
}

void VehicleSignals::get(const QString &path, const bool actuator)
{
	if (m_broker)
		m_broker->get(path, actuator);
}

void VehicleSignals::set(const QString &path, const QString &value, const bool actuator)
{
	if (m_broker)
		m_broker->set(path, value, actuator);
}

void VehicleSignals::set(const QString &path, const qint32 value, const bool actuator)
{
	if (m_broker)
		m_broker->set(path, value, actuator);
}

void VehicleSignals::set(const QString &path, const qint64 value, const bool actuator)
{
	if (m_broker)
		m_broker->set(path, value, actuator);
}

void VehicleSignals::set(const QString &path, const quint32 value, const bool actuator)
{
	if (m_broker)
		m_broker->set(path, value, actuator);
}

void VehicleSignals::set(const QString &path, const quint64 value, const bool actuator)
{
	if (m_broker)
		m_broker->set(path, value, actuator);
}

void VehicleSignals::set(const QString &path, const float value, const bool actuator)
{
	if (m_broker)
		m_broker->set(path, value, actuator);
}

void VehicleSignals::set(const QString &path, const double value, const bool actuator)
{
	if (m_broker)
		m_broker->set(path, value, actuator);
}

void VehicleSignals::subscribe(const QString &path, bool actuator)
{
	if (m_broker)
		m_broker->subscribe(path, actuator);
}

void VehicleSignals::subscribe(const QMap<QString, bool> &signals_)
{
	if (m_broker)
		m_broker->subscribe(signals_);
}

// Slots

void VehicleSignals::onConnected()
{
	QObject::connect(m_broker, &QtKuksaClient::getResponse, this, &VehicleSignals::onGetResponse);
	QObject::connect(m_broker, &QtKuksaClient::setResponse, this, &VehicleSignals::onSetResponse);
	QObject::connect(m_broker, &QtKuksaClient::subscribeResponse, this, &VehicleSignals::onSubscribeResponse);
	//QObject::connect(m_broker, &QtKuksaClient::subscribeDone, this, &VehicleSignals::onSubscribeDone);

	emit connected();
}

void VehicleSignals::onGetResponse(QString path, QString value, QString timestamp)
{
	emit getSuccessResponse(path, value, timestamp);
}

void VehicleSignals::onSetResponse(QString path, QString error)
{
	emit setErrorResponse(path, error);
}

void VehicleSignals::onSubscribeResponse(QString path, QString value, QString timestamp)
{
	if (m_config.verbose() > 1)
		qDebug() << "VehicleSignals::onSubscribeResponse: got " << path << " = " << value;
	emit signalNotification(path, value, timestamp);
}

void VehicleSignals::onSubscribeDone(const QMap<QString, bool> &signals_, bool canceled)
{
	if (!canceled) {
		// queue up a resubscribe attempt
		QFuture<void> future = QtConcurrent::run(&VehicleSignals::resubscribe, this, signals_);
	}
}

// Private

void VehicleSignals::resubscribe(const QMap<QString, bool> &signals_)
{
	// Delay 100 milliseconds between subscribe attempts
	QThread::msleep(100);

	if (m_broker)
		m_broker->subscribe(signals_);
}
