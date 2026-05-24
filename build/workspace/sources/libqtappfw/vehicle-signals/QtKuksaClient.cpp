/*
 * Copyright (C) 2023 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <QDebug>
#include <QSettings>
#include <QUrl>
#include <QFile>
#include <QtConcurrent>
#include <mutex>
#include <chrono>

#include "QtKuksaClient.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

class QtKuksaClient::SubscribeReader : public grpc::ClientReadReactor<SubscribeResponse> {
public:
	SubscribeReader(VAL::Stub *stub,
			QtKuksaClient *client,
			VehicleSignalsConfig &config,
			const QMap<QString, bool> &s,
			const SubscribeRequest *request):
		client_(client),
		config_(config),
		signals_(s) {
		QString token = config_.authToken();
		if (!token.isEmpty()) {
			token.prepend("Bearer ");
			context_.AddMetadata(std::string("authorization"), token.toStdString());
		}
		stub->async()->Subscribe(&context_, request, this);
		StartRead(&response_);
		StartCall();
	}
	void OnReadDone(bool ok) override {
		std::unique_lock<std::mutex> lock(mutex_);
		if (ok) {
			if (client_)
				client_->handleSubscribeResponse(&response_);
			StartRead(&response_);
		}
	}
	void OnDone(const Status& s) override {
		status_ = s;
		if (client_) {
			if (config_.verbose() > 1)
				qDebug() << "QtKuksaClient::subscribe::Reader done";
			client_->handleSubscribeDone(signals_, status_);
		}

		// gRPC engine is done with us, safe to self-delete
		delete this;
	}

private:
	QtKuksaClient *client_;
	VehicleSignalsConfig config_;
	QMap<QString, bool> signals_;

	ClientContext context_;
	SubscribeResponse response_;
	std::mutex mutex_;
	Status status_;
};

QtKuksaClient::QtKuksaClient(const std::shared_ptr< ::grpc::ChannelInterface>& channel,
			     const VehicleSignalsConfig &config,
			     QObject *parent) :
	QObject(parent),
	m_channel(channel),
	m_config(config),
	m_connected(false)
{
        m_stub = VAL::NewStub(channel);

}

void QtKuksaClient::connect()
{
	// Check for connection in another thread
	QFuture<void> future = QtConcurrent::run(&QtKuksaClient::waitForConnected, this);
}

void QtKuksaClient::get(const QString &path, const bool actuator)
{
	m_connected_mutex.lock();
	if (!m_connected) {
		m_connected_mutex.unlock();
		return;
	}
	m_connected_mutex.unlock();
	
	ClientContext *context = new ClientContext();
	if (!context) {
		handleCriticalFailure("Could not create ClientContext");
		return;
	}
	QString token = m_config.authToken();
	if (!token.isEmpty()) {
		token.prepend("Bearer ");
		context->AddMetadata(std::string("authorization"), token.toStdString());
	}

	GetRequest request;
	auto entry = request.add_entries();
	entry->set_path(path.toStdString());
	entry->add_fields(Field::FIELD_PATH);	
	if (actuator)
		entry->add_fields(Field::FIELD_ACTUATOR_TARGET);
	else
		entry->add_fields(Field::FIELD_VALUE);

	GetResponse *response = new GetResponse();
	if (!response) {
		handleCriticalFailure("Could not create GetResponse");
		return;
	}

	// NOTE: Using ClientUnaryReactor instead of the shortcut method
	//       would allow getting detailed errors.
	m_stub->async()->Get(context, &request, response,
			     [this, context, response](Status s) {
				     if (s.ok())
					     handleGetResponse(response);
				     delete response;
				     delete context;
			     });
}

// Since a set request needs a Datapoint with the appropriate type value,
// checking the signal metadata to get the type would be a requirement for
// a generic set call that takes a string as argument.  For now, assume
// that set with a string is specifically for a signal of string type.

void QtKuksaClient::set(const QString &path, const bool value, const bool actuator)
{
	Datapoint dp;
	dp.set_bool_(value);
	set(path, dp, actuator);
}

void QtKuksaClient::set(const QString &path, const QString &value, const bool actuator)
{
	Datapoint dp;
	dp.set_string(value.toStdString());
	set(path, dp, actuator);
}

void QtKuksaClient::set(const QString &path, const qint32 value, const bool actuator)
{
	Datapoint dp;
	dp.set_int32(value);
	set(path, dp, actuator);
}

void QtKuksaClient::set(const QString &path, const qint64 value, const bool actuator)
{
	Datapoint dp;
	dp.set_int64(value);
	set(path, dp, actuator);
}

void QtKuksaClient::set(const QString &path, const quint32 value, const bool actuator)
{
	Datapoint dp;
	dp.set_uint32(value);
	set(path, dp, actuator);
}

void QtKuksaClient::set(const QString &path, const quint64 value, const bool actuator)
{
	Datapoint dp;
	dp.set_uint64(value);
	set(path, dp, actuator);
}

void QtKuksaClient::set(const QString &path, const float value, const bool actuator)
{
	Datapoint dp;
	dp.set_float_(value);
	set(path, dp, actuator);
}

void QtKuksaClient::set(const QString &path, const double value, const bool actuator)
{
	Datapoint dp;
	dp.set_double_(value);
	set(path, dp, actuator);
}

void QtKuksaClient::subscribe(const QString &path, const bool actuator)
{
	m_connected_mutex.lock();
	if (!m_connected) {
		m_connected_mutex.unlock();
		return;
	}
	m_connected_mutex.unlock();

	QMap<QString, bool> s;
	s[path] = actuator;
	subscribe(s);
}

void QtKuksaClient::subscribe(const QMap<QString, bool> &signals_)
{
	m_connected_mutex.lock();
	if (!m_connected) {
		m_connected_mutex.unlock();
		return;
	}
	m_connected_mutex.unlock();

	SubscribeRequest *request = new SubscribeRequest();
	if (!request) {
		handleCriticalFailure("Could not create SubscribeRequest");
		return;
	}

	auto it = signals_.constBegin();
	while (it != signals_.constEnd()) {
		if (m_config.verbose() > 1)
			qDebug() << "QtKuksaClient::subscribe: adding " << it.key() << ", actuator " << it.value();
		auto entry = request->add_entries();
		entry->set_path(it.key().toStdString());
		entry->add_fields(Field::FIELD_PATH);	
		if (it.value())
			entry->add_fields(Field::FIELD_ACTUATOR_TARGET);
		else
			entry->add_fields(Field::FIELD_VALUE);
		++it;
	}

	SubscribeReader *reader = new SubscribeReader(m_stub.get(), this, m_config, signals_, request);
	if (!reader)
		handleCriticalFailure("Could not create SubscribeReader");

	delete request;
}

// Private

void QtKuksaClient::waitForConnected()
{
	while (!m_channel->WaitForConnected(std::chrono::system_clock::now() +
					  std::chrono::milliseconds(500)));

	m_connected_mutex.lock();
	m_connected = true;
	m_connected_mutex.unlock();

	qInfo() << "Databroker gRPC channel ready";
	emit connected();
}

void QtKuksaClient::set(const QString &path, const Datapoint &dp, const bool actuator)
{
	m_connected_mutex.lock();
	if (!m_connected) {
		m_connected_mutex.unlock();
		return;
	}
	m_connected_mutex.unlock();

	ClientContext *context = new ClientContext();
	if (!context) {
		handleCriticalFailure("Could not create ClientContext");
		return;
	}
	QString token = m_config.authToken();
	if (!token.isEmpty()) {
		token.prepend("Bearer ");
		context->AddMetadata(std::string("authorization"), token.toStdString());
	}

	SetRequest request;
	auto update = request.add_updates();
	auto entry = update->mutable_entry();
	entry->set_path(path.toStdString());
	if (actuator) {
		auto target = entry->mutable_actuator_target();
		*target = dp;
		update->add_fields(Field::FIELD_ACTUATOR_TARGET);
	} else {
		auto value = entry->mutable_value();
		*value = dp;
		update->add_fields(Field::FIELD_VALUE);		
	}

	SetResponse *response = new SetResponse();
	if (!response) {
		handleCriticalFailure("Could not create SetResponse");
		delete context;
		return;
	}

	// NOTE: Using ClientUnaryReactor instead of the shortcut method
	//       would allow getting detailed errors.
	m_stub->async()->Set(context, &request, response,
				       [this, context, response](Status s) {
					       if (s.ok())
						       handleSetResponse(response);
					       delete response;
					       delete context;
				       });
}

void QtKuksaClient::handleGetResponse(const GetResponse *response)
{
	if (!(response && response->entries_size()))
		return;

	for (auto it = response->entries().begin(); it != response->entries().end(); ++it) {
		// We expect paths in the response entries
		if (!it->path().size())
			continue;
		Datapoint dp;
		if (it->has_actuator_target())
			dp = it->actuator_target();
		else
			dp = it->value();

		QString path = QString::fromStdString(it->path());
		QString value;
		QString timestamp;		
		if (convertDatapoint(dp, value, timestamp)) {
			if (m_config.verbose() > 1)
				qDebug() << "QtKuksaClient::handleGetResponse: path = " << path << ", value = " << value;

			emit getResponse(path, value, timestamp);
		}
	}
}

void QtKuksaClient::handleSetResponse(const SetResponse *response)
{
	if (!(response && response->errors_size()))
		return;

	for (auto it = response->errors().begin(); it != response->errors().end(); ++it) {
		QString path = QString::fromStdString(it->path());
		QString error = QString::fromStdString(it->error().reason());
		emit setResponse(path, error);
	}

}

void QtKuksaClient::handleSubscribeResponse(const SubscribeResponse *response)
{
	if (!(response && response->updates_size()))
		return;

	for (auto it = response->updates().begin(); it != response->updates().end(); ++it) {
		// We expect entries that have paths in the response
		if (!(it->has_entry() && it->entry().path().size()))
			continue;

		auto entry = it->entry();
		QString path = QString::fromStdString(entry.path());

		Datapoint dp;
		if (entry.has_actuator_target())
			dp = entry.actuator_target();
		else
			dp = entry.value();

		QString value;
		QString timestamp;		
		if (convertDatapoint(dp, value, timestamp)) {
			if (m_config.verbose() > 1)
				qDebug() << "QtKuksaClient::handleSubscribeResponse: path = " << path << ", value = " << value;
	
			emit subscribeResponse(path, value, timestamp);
		}
	}
}

void QtKuksaClient::handleSubscribeDone(const QMap<QString, bool> &signals_, const Status &status)
{
	qInfo() << "QtKuksaClient::handleSubscribeDone: enter";
	if (m_config.verbose() > 1)
		qDebug() << "Subscribe status = " << status.error_code() <<
			" (" << QString::fromStdString(status.error_message()) << ")";

	emit subscribeDone(signals_, status.error_code() == grpc::CANCELLED);
}

bool QtKuksaClient::convertDatapoint(const Datapoint &dp, QString &value, QString &timestamp)
{
	// NOTE: Currently no array type support

	if (dp.has_string())
		value = QString::fromStdString(dp.string());
	else if (dp.has_int32())
		value = QString::number(dp.int32());
	else if (dp.has_int64())
		value = QString::number(dp.int64());
	else if (dp.has_uint32())
		value = QString::number(dp.uint32());
	else if (dp.has_uint64())
		value = QString::number(dp.uint64());
	else if (dp.has_float_())
		value = QString::number(dp.float_(), 'f', 6);
	else if (dp.has_double_())
		value = QString::number(dp.double_(), 'f', 6);
	else if (dp.has_bool_())
		value = dp.bool_() ? "true" : "false";
	else {
		if (m_config.verbose())
			qWarning() << "Malformed response (unsupported value type)";
		return false;
	}

	timestamp = QString::number(dp.timestamp().seconds()) + "." + QString::number(dp.timestamp().nanos());
	return true;
}

void QtKuksaClient::handleCriticalFailure(const QString &error)
{
	if (error.size())
		qCritical() << error;
}
