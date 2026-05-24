/*
 * Copyright (C) 2022,2023 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VEHICLESIGNALS_H
#define VEHICLESIGNALS_H

#include <QObject>

#include "VehicleSignalsConfig.h"

class QtKuksaClient;

// VSS signal interface class

class VehicleSignals : public QObject
{
	Q_OBJECT

public:
        explicit VehicleSignals(const VehicleSignalsConfig &config, QObject * parent = Q_NULLPTR);
        virtual ~VehicleSignals();

	Q_INVOKABLE void connect();	
	Q_INVOKABLE void authorize();

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

	////////// Add
	Q_INVOKABLE void set(const QString &path, const bool value, const bool actuator = false);
signals:
	void connected();
	void authorized();
	void disconnected();
	void getSuccessResponse(QString path, QString value, QString timestamp);
	void setErrorResponse(QString path, QString error);
	void signalNotification(QString path, QString value, QString timestamp);

private slots:
	void onConnected();
	void onGetResponse(QString path, QString value, QString timestamp);
	void onSetResponse(QString path, QString error);
	void onSubscribeResponse(QString path, QString value, QString timestamp);
	void onSubscribeDone(const QMap<QString, bool> &signals_, bool canceled);

private:
	VehicleSignalsConfig m_config;
	QtKuksaClient *m_broker;

	void resubscribe(const QMap<QString, bool> &signals_);
};

#endif // VEHICLESIGNALS_H
