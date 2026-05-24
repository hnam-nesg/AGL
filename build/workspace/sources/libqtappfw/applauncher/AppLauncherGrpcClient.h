// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2022 Konsulko Group
 */

#ifndef APPLAUNCHER_GRPC_CLIENT_H
#define APPLAUNCHER_GRPC_CLIENT_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QThread>
#include <grpcpp/grpcpp.h>

#include "applauncher.grpc.pb.h"

using grpc::Channel;

class AppStatusEventReader : public QObject
{
	Q_OBJECT
public:
	AppStatusEventReader(std::shared_ptr<automotivegradelinux::AppLauncher::Stub> &stub,
			     QObject *parent = Q_NULLPTR) : QObject(parent), stub_(stub) {}

public slots:
	void GetStatusEvents();

signals:
	void statusUpdate(const QString &id, const QString &status);
	void finished();

private:
	std::shared_ptr<automotivegradelinux::AppLauncher::Stub> stub_;
};

class AppLauncherGrpcClient : public QObject
{
	Q_OBJECT

public:
	AppLauncherGrpcClient(QObject *parent = Q_NULLPTR);

	bool StartApplication(const QString &id);

	bool ListApplications(QList<QMap<QString, QString>> &list);

private:
	std::shared_ptr<automotivegradelinux::AppLauncher::Stub> stub_;

	QThread m_event_thread;

};

#endif // APPLAUNCHER_GRPC_CLIENT_H
