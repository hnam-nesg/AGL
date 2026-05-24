// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2022 Konsulko Group
 */

#include <QDebug>

#include "AppLauncherClient.h"
#include "AppLauncherGrpcClient.h" 

AppLauncherClient::AppLauncherClient(QObject *parent) : QObject(parent)
{
	m_launcher = new AppLauncherGrpcClient(this);
}

AppLauncherClient::~AppLauncherClient()
{
	delete m_launcher;
}

bool AppLauncherClient::startApplication(const QString &id)
{
	if (m_launcher)
		return m_launcher->StartApplication(id);

	return false;
}

bool AppLauncherClient::listApplications(QList<QMap<QString, QString>> &list)
{
	if (!m_launcher) {
		return false;
	}

	return m_launcher->ListApplications(list);
}

void AppLauncherClient::sendStatusEvent(const QString &id, const QString &status)
{
	emit appStatusEvent(id, status);
}
