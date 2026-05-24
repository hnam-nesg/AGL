 /*
 * Copyright (C) 2019-2023 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QObject>
#include <QVariant>
#include <QMutex>

class VehicleSignals;

class Navigation : public QObject
{
	Q_OBJECT

public:
	explicit Navigation(VehicleSignals *vs, bool router = false, QObject *parent = Q_NULLPTR);
	virtual ~Navigation();

	Q_INVOKABLE void broadcastPosition(double lat, double lon, double drc, double dst);
	Q_INVOKABLE void broadcastRouteInfo(double lat, double lon, double route_lat, double route_lon);
	Q_INVOKABLE void broadcastStatus(QString state);

	// only support one waypoint for now
	Q_INVOKABLE void sendWaypoint(double lat, double lon);

signals:
	void statusEvent(QVariantMap data);
	void positionEvent(QVariantMap data);
	void waypointsEvent(QVariantMap data);

private slots:
	void onConnected();
	void onSignalNotification(QString path, QString value, QString timestamp);
	void onSignalNotificationRouter(QString path, QString value, QString timestamp);

private:
	void handlePositionUpdate();

	VehicleSignals *m_vs;
	bool m_router = false;
	bool m_connected = false;

	QMutex m_position_mutex;
	double m_latitude;
	bool m_latitude_set = false;
	double m_longitude;
	bool m_longitude_set = false;
	double m_heading;
	bool m_heading_set = false;
	double m_distance = 0.0;
	bool m_distance_set = false;
	double m_dest_latitude;
	bool m_dest_latitude_set = false;
	double m_dest_longitude;
	bool m_dest_longitude_set = false;
};

#endif // NAVIGATION_H
