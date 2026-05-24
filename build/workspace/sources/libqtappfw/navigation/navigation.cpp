/*
 * Copyright (C) 2019-2023 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <QDebug>
#include <math.h>

#include "navigation.h"
#include "vehiclesignals.h"

#define ROUND9(x) (round((x) * 1000000000) / 1000000000)

Navigation::Navigation(VehicleSignals *vs, bool router, QObject * parent) :
	m_vs(vs),
	m_router(router),
	QObject(parent)
{
	QObject::connect(m_vs, &VehicleSignals::connected, this, &Navigation::onConnected);

	if (m_vs)
		m_vs->connect();
}

Navigation::~Navigation()
{
	delete m_vs;
}

void Navigation::sendWaypoint(double lat, double lon)
{
	if (!(m_vs && m_connected && m_router))
		return;

	// The original implementation resulted in at least 9 decimal places
	// making it through to the clients, so explicitly format for 9.  In
	// practice going from the QString default 6 to 9 does make a difference
	// with respect to smoothness in the position-based map rotations done
	// in tbtnavi.
	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude", lat);
	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude", lon);
}

void Navigation::broadcastPosition(double lat, double lon, double drc, double dst)
{
	if (!(m_vs && m_connected && m_router))
		return;

	m_vs->set("Vehicle.CurrentLocation.Latitude", lat);
	m_vs->set("Vehicle.CurrentLocation.Longitude", lon);
	m_vs->set("Vehicle.CurrentLocation.Heading", drc);

	// NOTES:
	// - This signal is an AGL addition, it may make sense to engage with the
	//   VSS specification upstream to discuss an addition along these lines.
	// - The signal makes more sense in kilometers wrt VSS expectations, so
	//   conversion from meters happens here for now to avoid changing the
	//   existing clients.  This may be worth revisiting down the road.
	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.ElapsedDistance", (float) (dst / 1000));
}

void Navigation::broadcastRouteInfo(double lat, double lon, double route_lat, double route_lon)
{
	if (!(m_vs && m_connected && m_router))
		return;

	m_vs->set("Vehicle.CurrentLocation.Latitude", lat);
	m_vs->set("Vehicle.CurrentLocation.Longitude", lon);

	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude", route_lat, true);
	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude", route_lon, true);
}

void Navigation::broadcastStatus(QString state)
{
	if (!(m_vs && m_connected && m_router))
		return;

	// The signal states have been changed to all uppercase to match
	// VSS 3.x expectations, enforce that.
	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.State", state.toUpper());
}

void Navigation::onConnected()
{
	if (!m_vs)
		return;

	m_connected = true;

	if (m_router) {
		qInfo() << "Connected as router";
		QObject::connect(m_vs, &VehicleSignals::signalNotification, this, &Navigation::onSignalNotificationRouter);
	} else {
		QObject::connect(m_vs, &VehicleSignals::signalNotification, this, &Navigation::onSignalNotification);
	}

	// NOTE: This signal is another AGL addition where it is possible
	//       upstream may be open to adding it to VSS.
	QMap<QString, bool> s;
	s["Vehicle.Cabin.Infotainment.Navigation.State"] = false;
	if (!m_router) {
		// Router broadcasts these signals and does not need to listen to them
		s["Vehicle.CurrentLocation.Latitude"] = false;
		s["Vehicle.CurrentLocation.Longitude"] = false;
		s["Vehicle.CurrentLocation.Heading"] = false;
		s["Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude"] = false;
		s["Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude"] = false;
		s["Vehicle.Cabin.Infotainment.Navigation.ElapsedDistance"] = false;
	} else {
		// Router acts as actuator for these signals
		// This is a bit convoluted, but would allow wiring up external destination
		// setting with e.g. Alexa as has been done in the past.
		s["Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude"] = true;
		s["Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude"] = true;
	}
	m_vs->subscribe(s);
}

void Navigation::onSignalNotification(QString path, QString value, QString timestamp)
{
	if (path == "Vehicle.Cabin.Infotainment.Navigation.State") {
		QVariantMap event;
		event["state"] = value;
		emit statusEvent(event);
	} else if (path == "Vehicle.CurrentLocation.Latitude") {
		m_position_mutex.lock();
		m_latitude = value.toDouble();
		m_latitude_set = true;
		handlePositionUpdate();
		m_position_mutex.unlock();
	} else if (path == "Vehicle.CurrentLocation.Longitude") {
		m_position_mutex.lock();
		m_longitude = value.toDouble();
		m_longitude_set = true;
		handlePositionUpdate();
		m_position_mutex.unlock();
	} else if (path == "Vehicle.CurrentLocation.Heading") {
		m_position_mutex.lock();
		m_heading = value.toDouble();
		m_heading_set = true;
		handlePositionUpdate();
		m_position_mutex.unlock();
	} else if (path == "Vehicle.Cabin.Infotainment.Navigation.ElapsedDistance") {
		m_position_mutex.lock();
		m_distance = value.toDouble();
		m_distance_set = true;
		handlePositionUpdate();
		m_position_mutex.unlock();
	} else if (path == "Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude") {
		m_position_mutex.lock();
		m_dest_latitude = value.toDouble();
		m_dest_latitude_set = true;
		handlePositionUpdate();
		m_position_mutex.unlock();
	} else if (path == "Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude") {
		m_position_mutex.lock();
		m_dest_longitude = value.toDouble();
		m_dest_longitude_set = true;
		handlePositionUpdate();
		m_position_mutex.unlock();

		// NOTE: Potentially could emit a waypointsEvent here, but
		//       nothing in the demo currently requires it, so do
		//       not bother for now.  If something like Alexa is
		//       added it or some other replacement / rework will
		//       be required.
	}
}

void Navigation::onSignalNotificationRouter(QString path, QString value, QString timestamp)
{
	if (path == "Vehicle.Cabin.Infotainment.Navigation.State") {
		QVariantMap event;
		event["state"] = value;
		emit statusEvent(event);
	} else if (path == "Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude") {
		m_vs->set(path, value.toDouble());
	} else if (path == "Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude") {
		m_vs->set(path, value.toDouble());
	}
}

// Private

void Navigation::handlePositionUpdate()
{
	// NOTE: Since all the known AGL users of the VSS signals are users of
	//       this API, we know that updates occur in certain sequences and
	//       can leverage this to roll up for emitting the existing events.
	//
	//       The switch to the gRPC API for KUKSA.val complicates this as
	//       we now have to live with the combination of only getting
	//       notified when a signal changes to a new value on top of
	//       notifications not necessarily happening in order by the time
	//       they get through gRPC client/server/client/Qt stacks.  The "set"
	//       flags give us something that mostly works for now, but further
	//       investigation is required.  It is possible that either a switch
	//       to a custom gRPC API (i.e. standalone nav service daemon) or
	//       pushing for a rolled up position signal in a struct in upstream
	//       VSS might be needed to do this better.

	if (m_latitude_set && m_longitude_set && m_heading_set && m_distance_set) {
		QVariantMap event;
		event["position"] = "car";
		event["latitude"] = m_latitude;
		event["longitude"] = m_longitude;
		event["direction"] = m_heading;
		event["distance"] = m_distance * 1000;
		emit positionEvent(event);
		// The distance being updated is our trigger, so clear its flag.
		m_distance_set = false;
	} else if (m_latitude_set && m_longitude_set && m_dest_latitude_set && m_dest_longitude_set) {
		QVariantMap event;
		event["position"] = "route";
		event["latitude"] = m_latitude;
		event["longitude"] = m_longitude;
		event["route_latitude"] = m_dest_latitude;
		event["route_longitude"] = m_dest_longitude;
		emit positionEvent(event);
		// The destination values are the triggers, so clear their flags.
		m_dest_latitude_set = m_dest_longitude_set = false;
	}
}
