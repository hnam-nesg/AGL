// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (c) 2017 TOYOTA MOTOR CORPORATION
 * Copyright (c) 2022 Konsulko Group
 */

#ifndef HOMESCREENHANDLER_H
#define HOMESCREENHANDLER_H

#include <QObject>
#include <QVariantList>
#include <string>

#include "applicationlauncher.h"
#include "AppLauncherClient.h"
#include "AglShellGrpcClient.h"

using namespace std;

class HomescreenHandler : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.agl.homescreen")
	Q_PROPERTY(bool navigationActive READ navigationActive NOTIFY navigationStateChanged)
	Q_PROPERTY(double navigationCurrentLatitude READ navigationCurrentLatitude NOTIFY navigationStateChanged)
	Q_PROPERTY(double navigationCurrentLongitude READ navigationCurrentLongitude NOTIFY navigationStateChanged)
	Q_PROPERTY(double navigationStartLatitude READ navigationStartLatitude NOTIFY navigationStateChanged)
	Q_PROPERTY(double navigationStartLongitude READ navigationStartLongitude NOTIFY navigationStateChanged)
	Q_PROPERTY(double navigationDestinationLatitude READ navigationDestinationLatitude NOTIFY navigationStateChanged)
	Q_PROPERTY(double navigationDestinationLongitude READ navigationDestinationLongitude NOTIFY navigationStateChanged)
	Q_PROPERTY(double navigationHeading READ navigationHeading NOTIFY navigationStateChanged)
	Q_PROPERTY(QString navigationInstruction READ navigationInstruction NOTIFY navigationStateChanged)
	Q_PROPERTY(QString navigationNextInstruction READ navigationNextInstruction NOTIFY navigationStateChanged)
	Q_PROPERTY(QString navigationDistanceText READ navigationDistanceText NOTIFY navigationStateChanged)
	Q_PROPERTY(QString navigationTimeText READ navigationTimeText NOTIFY navigationStateChanged)
	Q_PROPERTY(QVariantList navigationRoutePath READ navigationRoutePath NOTIFY navigationStateChanged)
	Q_PROPERTY(QString navigationDestinationName READ navigationDestinationName NOTIFY navigationStateChanged)
	Q_PROPERTY(QString goongMapTilesKey READ goongMapTilesKey NOTIFY goongConfigChanged)
	Q_PROPERTY(QString goongStyleUrl READ goongStyleUrl NOTIFY goongConfigChanged)
public:
	explicit HomescreenHandler(ApplicationLauncher *launcher = 0, GrpcClient *_client = nullptr, QObject *parent = 0);
	~HomescreenHandler();

	Q_INVOKABLE void tapShortcut(QString application_id);

	void addAppToStack(const QString& application_id);
	void activateApp(const QString& app_id);
	Q_INVOKABLE void deactivateApp(const QString& app_id);
	Q_INVOKABLE void backToHome();
	void setGrpcClient(GrpcClient *_client) { m_grpc_client = _client; }
	GrpcClient *getGrpcClient(void) { return m_grpc_client; }
	bool navigationActive() const { return m_navigationActive; }
	double navigationCurrentLatitude() const { return m_navigationCurrentLatitude; }
	double navigationCurrentLongitude() const { return m_navigationCurrentLongitude; }
	double navigationStartLatitude() const { return m_navigationStartLatitude; }
	double navigationStartLongitude() const { return m_navigationStartLongitude; }
	double navigationDestinationLatitude() const { return m_navigationDestinationLatitude; }
	double navigationDestinationLongitude() const { return m_navigationDestinationLongitude; }
	double navigationHeading() const { return m_navigationHeading; }
	QString navigationInstruction() const { return m_navigationInstruction; }
	QString navigationNextInstruction() const { return m_navigationNextInstruction; }
	QString navigationDistanceText() const { return m_navigationDistanceText; }
	QString navigationTimeText() const { return m_navigationTimeText; }
	QVariantList navigationRoutePath() const { return m_navigationRoutePath; }
	QString navigationDestinationName() const { return m_navigationDestinationName; }
	QString goongMapTilesKey() const { return m_goongMapTilesKey; }
	QString goongStyleUrl() const { return m_goongStyleUrl; }

	QStringList apps_stack;
	bool m_hvacFloating = true;
	std::list<std::pair<const QString, const QString>> pending_app_list;
signals:
	void showNotification(QString application_id, QString icon_path, QString text);
	void showInformation(QString info);
	void navigationStateChanged();
	void goongConfigChanged();

public slots:
	Q_SCRIPTABLE bool ShowApp(const QString &application_id);
	void processAppStatusEvent(const QString &id, const QString &status);
	void reloadGoongConfig();
	void onNavigationRouteStateChanged(bool active,
					   double currentLatitude,
					   double currentLongitude,
					   double startLatitude,
					   double startLongitude,
					   double destinationLatitude,
					   double destinationLongitude,
					   double heading,
					   const QString &instruction,
					   const QString &nextInstruction,
					   const QString &distanceText,
					   const QString &timeText,
					   const QString &routePathJson,
					   const QString &destinationName);

private:
	void connectNavigationService();
	void registerControlService();
	void scheduleGoongConfigReloads();
	void setGoongConfig(const QString &mapTilesKey, const QString &styleUrl);
	static QVariantList parseRoutePath(const QString &routePathJson);

	ApplicationLauncher *mp_launcher;
	AppLauncherClient *mp_applauncher_client;
	GrpcClient *m_grpc_client;
	bool m_navigationActive = false;
	double m_navigationCurrentLatitude = 0.0;
	double m_navigationCurrentLongitude = 0.0;
	double m_navigationStartLatitude = 0.0;
	double m_navigationStartLongitude = 0.0;
	double m_navigationDestinationLatitude = 0.0;
	double m_navigationDestinationLongitude = 0.0;
	double m_navigationHeading = 0.0;
	QString m_navigationInstruction;
	QString m_navigationNextInstruction;
	QString m_navigationDistanceText;
	QString m_navigationTimeText;
	QVariantList m_navigationRoutePath;
	QString m_navigationDestinationName;
	QString m_goongMapTilesKey;
	QString m_goongStyleUrl;
};

#endif // HOMESCREENHANDLER_H
