/*
 * Copyright (C) 2020-2021 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QDebug>
#include "hvac.h"
#include "vehiclesignals.h"


// TODO: don't duplicate defaults from HVAC service here
HVAC::HVAC(VehicleSignals *vs, QObject * parent) :
	QObject(parent),
	m_vs(vs),
	m_connected(false),
	m_fanspeed(0),
	m_temp_left_zone(21),
	m_temp_right_zone(21)
{
	QObject::connect(m_vs, &VehicleSignals::connected, this, &HVAC::onConnected);

	if (m_vs)
		m_vs->connect();
}

HVAC::~HVAC()
{
}

void HVAC::set_fanspeed(int speed)
{
	if (!(m_vs && m_connected))
		return;

	// Scale incoming 0-255 speed to 0-100 to match VSS signal
	double value = (speed % 256) * 100.0 / 255.0;
	m_vs->set("Vehicle.Cabin.HVAC.Station.Row1.Driver.FanSpeed", (unsigned int) (speed), true);
	emit fanSpeedChanged(speed);
}

void HVAC::set_temp_left_zone(int temp)
{
	if (!(m_vs && m_connected))
		return;

	// Make sure value is within VSS signal range
	int value = temp;
	if (value > 50)
		value = 50;
	else if (value < -50)
		value = -50;
	m_vs->set("Vehicle.Cabin.HVAC.Station.Row1.Driver.Temperature", value, true);
	emit leftTemperatureChanged(temp);
}

void HVAC::set_temp_right_zone(int temp)
{
	if (!(m_vs && m_connected))
		return;

	// Make sure value is within VSS signal range
	int value = temp;
	if (value > 50)
		value = 50;
	else if (value < -50)
		value = -50;
	m_vs->set("Vehicle.Cabin.HVAC.Station.Row1.Passenger.Temperature", value, true);
	emit rightTemperatureChanged(temp);
}

void HVAC::setSyncTemp(bool sync)
{
    qDebug() << "[HVAC] setSyncTemp ENTER sync =" << sync
             << "m_vs =" << m_vs
             << "m_connected =" << m_connected;

    if (!m_vs) {
        qDebug() << "[HVAC] setSyncTemp FAILED: m_vs is null";
        return;
    }

    if (!m_connected) {
        qDebug() << "[HVAC] setSyncTemp FAILED: not connected";
        return;
    }

    m_synctemp = sync;

    qDebug() << "[HVAC] before m_vs->set IsAirConditioningActive =" << sync;

    m_vs->set("Vehicle.Cabin.HVAC.IsAirConditioningActive", sync, true);

    qDebug() << "[HVAC] after m_vs->set IsAirConditioningActive =" << sync;

    emit syncTempChanged(sync);
}

void HVAC::setRearDefrost(bool active){
	if (!(m_vs && m_connected))
		return;

	m_reardefrost = active;	
	m_vs->set("Vehicle.Cabin.HVAC.IsRearDefrosterActive", active, true);
	emit rearDefrostChanged(active);
}

void HVAC::setFrontDefrost(bool active){
	if (!(m_vs && m_connected))
		return;

	m_frontdefrost = active;	
	m_vs->set("Vehicle.Cabin.HVAC.IsFrontDefrosterActive", active, true);
	emit frontDefrostChanged(active);
}

void HVAC::setAirDistribution(QString mode)
{
    if (!(m_vs && m_connected))
        return;

    mode = mode.trimmed().toUpper();

    if (mode != "UP" && mode != "MIDDLE" && mode != "DOWN") {
        qWarning() << "[HVAC] Invalid AirDistribution:" << mode;
        return;
    }

    if (m_airDistribution == mode)
        return;

	qDebug() << "[HVAC] after m_vs->set IsAirConditioningActive =" << mode;

    m_vs->set("Vehicle.Cabin.HVAC.Station.Row1.Driver.AirDistribution", mode, false);
}

void HVAC::setAirRecir(bool active){
	if (!(m_vs && m_connected))
		return;

	m_airrecir = active;	
	m_vs->set("Vehicle.Cabin.HVAC.IsRecirculationActive", active, true);
	emit airRecirChanged(active);
}

void HVAC::setAirAuto(bool active){
	if (!(m_vs && m_connected))
		return;

	m_airauto = active;	
	m_vs->set("Vehicle.Cabin.HVAC.IsAutoPowerOptimize", active, true);
	emit airAutoChanged(active);
}

void HVAC::setAirCondition(bool active)
{
    if (!(m_vs && m_connected))
		return;

    m_aircondition = active;

    m_vs->set("Vehicle.Cabin.HVAC.IsAirConditioningActive", active, true);

    emit airConditionChanged(active);
}

void HVAC::setSeatDriver(int level)
{
    if (!(m_vs && m_connected))
		return;

    m_seatdriver = level;

    m_vs->set("Vehicle.Cabin.Seat.Row1.DriverSide.HeatingCooling", level, true);

    emit seatDriverChanged(level);
}

void HVAC::setSeatPassenger(int level)
{
    if (!(m_vs && m_connected))
		return;

    m_seatpassenger = level;

    m_vs->set("Vehicle.Cabin.Seat.Row1.PassengerSide.HeatingCooling", level, true);

    emit seatPassengerChanged(level);
}

void HVAC::setSeatBehindDriver(int level)
{
    if (!(m_vs && m_connected))
		return;

    m_seatbehinddriver = level;

    m_vs->set("Vehicle.Cabin.Seat.Row2.DriverSide.HeatingCooling", level, true);

    emit seatBehindDriverChanged(level);
}

void HVAC::setSeatBehindPassenger(int level)
{
    if (!(m_vs && m_connected))
		return;

    m_seatbehindpassenger = level;

    m_vs->set("Vehicle.Cabin.Seat.Row2.PassengerSide.HeatingCooling", level, true);

    emit seatBehindPassengerChanged(level);
}



void HVAC::onConnected()
{
	if (!m_vs)
		return;

	// Could subscribe and connect notification signal here to monitor
	// external updates...

	m_connected = true;
}
