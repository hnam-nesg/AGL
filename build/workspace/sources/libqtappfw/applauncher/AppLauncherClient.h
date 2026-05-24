// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2022 Konsulko Group
 */

#ifndef APPLAUNCHER_CLIENT_H
#define APPLAUNCHER_CLIENT_H

#include <QObject>
#include <QList>
#include <QMap>

class AppLauncherGrpcClient;

class AppLauncherClient : public QObject
{
	Q_OBJECT

public:
	explicit AppLauncherClient(QObject *parent = Q_NULLPTR);
	virtual ~AppLauncherClient();

	Q_INVOKABLE bool startApplication(const QString &id);
	Q_INVOKABLE bool listApplications(QList<QMap<QString, QString>> &list);

public slots:
	void sendStatusEvent(const QString &id, const QString &status);

signals:
	void appStatusEvent(const QString &id, const QString &status);

private:
	AppLauncherGrpcClient *m_launcher;
};

#endif // APPLAUNCHER_CLIENT_H
