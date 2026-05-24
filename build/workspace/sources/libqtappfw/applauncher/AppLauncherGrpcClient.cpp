// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2022 Konsulko Group
 */

#include <QDebug>
#include "AppLauncherGrpcClient.h"
#include "AppLauncherClient.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using automotivegradelinux::AppLauncher;
using automotivegradelinux::StartRequest;
using automotivegradelinux::StartResponse;
using automotivegradelinux::ListRequest;
using automotivegradelinux::ListResponse;
using automotivegradelinux::AppInfo;
using automotivegradelinux::StatusRequest;
using automotivegradelinux::StatusResponse;
using automotivegradelinux::AppStatus;


void AppStatusEventReader::GetStatusEvents()
{
	ClientContext context;
	StatusRequest request;
	StatusResponse response;

	std::unique_ptr<ClientReader<StatusResponse> > reader(stub_->GetStatusEvents(&context, request));
	while (reader->Read(&response)) {
		if (response.has_app()) {
			AppStatus app_status = response.app();
			emit statusUpdate(QString::fromStdString(app_status.id()),
					  QString::fromStdString(app_status.status()));
		}
	}
	Status status = reader->Finish();
	if (!status.ok()) {
			qWarning() << "GetStatusEvents RPC failed";
	}

	emit finished();
}

AppLauncherGrpcClient::AppLauncherGrpcClient(QObject *parent) : QObject(parent)
{
	stub_ = AppLauncher::NewStub(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()));

	// Create thread to read status events
	AppStatusEventReader *reader = new AppStatusEventReader(stub_);
	reader->moveToThread(&m_event_thread);
	connect(&m_event_thread, &QThread::started, reader, &AppStatusEventReader::GetStatusEvents);
	connect(reader, &AppStatusEventReader::finished, &m_event_thread, &QThread::quit);
	// FIXME: Normally the thread finishing would be connected per the below
	//        to trigger cleaning up the object.  That seems to trigger a crash
	//        for not entirely obvious reasons.  It seems unrelated to the signal
	//        connection to AppLauncherClient, as not connecting does not prevent
	//        the crash; further investigation is required.
	//connect(reader, &AppStatusEventReader::finished, reader, &AppStatusEventReader::deleteLater);
	//connect(&m_event_thread, &QThread::finished, &m_event_thread, &QThread::deleteLater);

	// To avoid having an intermediary slot+signal in this class, try
	// casting parent to AppLauncherClient and connect directly to its
	// slot. Callers should set parent to ensure this works as required.
	if (parent) {
		AppLauncherClient *launcher = qobject_cast<AppLauncherClient*>(parent);
		if (launcher)
			connect(reader,
				&AppStatusEventReader::statusUpdate,
				launcher,
				&AppLauncherClient::sendStatusEvent);
	}

	// Start status event handling
	m_event_thread.start();
}

bool AppLauncherGrpcClient::StartApplication(const QString &id)
{
	StartRequest request;
	request.set_id(id.toStdString());

	ClientContext context;
	StartResponse response;
	Status status = stub_->StartApplication(&context, request, &response);

	return status.ok();
}

bool AppLauncherGrpcClient::ListApplications(QList<QMap<QString, QString>> &list)
{
	ListRequest request; // empty
	ClientContext context;
	ListResponse response;

	Status status = stub_->ListApplications(&context, request, &response);
	if (!status.ok())
		return false;

	for (int i = 0; i < response.apps_size(); i++) {
		QMap<QString, QString> appinfo_map;
		AppInfo appinfo = response.apps(i);
		appinfo_map["id"] = QString::fromStdString(appinfo.id());
		appinfo_map["name"] = QString::fromStdString(appinfo.name());
		appinfo_map["icon_path"] = QString::fromStdString(appinfo.icon_path());
		list.append(appinfo_map);
	}

	return true;
}
