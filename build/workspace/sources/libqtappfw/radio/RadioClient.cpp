/*
 * Copyright (C) 2018-2020,2022 Konsulko Group
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
#include "RadioClient.h"
#include "RadioGrpcClient.h"

RadioClient::RadioClient(QQmlContext *context, QObject * parent) :
	QObject(parent),
	m_band(1),
	m_frequency(0),
	m_minFrequency(0),
	m_maxFrequency(0),
	m_playing(false),
	m_scanning(false)
{
	m_radio = new RadioGrpcClient(this);

	if (m_radio) {
		m_radio->GetBandParameters(m_band, m_minFrequency, m_maxFrequency, m_frequencyStep);
		emit minFrequencyChanged(m_minFrequency);
		emit maxFrequencyChanged(m_maxFrequency);
		emit frequencyStepChanged(m_frequencyStep);

		// Handle start up
		if (!m_frequency) {
			m_frequency = m_minFrequency;
			emit frequencyChanged(m_frequency);
		}
	}
}

RadioClient::~RadioClient()
{
	delete m_radio;
}

void RadioClient::setBand(int band)
{
	if (m_radio)
		m_radio->SetBand(band);
}

void RadioClient::setFrequency(int frequency)
{
	if (!m_radio)
		return;

	m_radio->SetFrequency(frequency);

	// To improve UI responsiveness, signal the change here immediately
	// This fixes visual glitchiness in the slider caused by the frequency
	// update event taking long enough that the QML engine gets a chance
	// to update the slider with the current value before the event with
	// the new value comes.
	m_frequency = frequency;
	emit frequencyChanged(m_frequency);
}

// control related methods

void RadioClient::start()
{
	if (m_radio)
		m_radio->Start();
}

void RadioClient::stop()
{
	if (m_radio)
		m_radio->Stop();
}

void RadioClient::scanForward()
{
	if (!m_radio || m_scanning)
		return;

    	m_radio->ScanForward();

	m_scanning = true;
	emit scanningChanged(m_scanning);
}

void RadioClient::scanBackward()
{
	if (!m_radio || m_scanning)
		return;

    	m_radio->ScanBackward();

	m_scanning = true;
	emit scanningChanged(m_scanning);
}

void RadioClient::scanStop()
{
	if (m_radio)
		m_radio->ScanStop();

	m_scanning = false;
	emit scanningChanged(m_scanning);
}

void RadioClient::updateBand(int band)
{
	m_band = band;
	emit bandChanged(m_band);
}

void RadioClient::updateFrequency(int frequency)
{
	m_frequency = frequency;
	emit frequencyChanged(m_frequency);
}

void RadioClient::updatePlaying(bool status)
{
	m_playing = status;
	emit playingChanged(m_playing);
}

void RadioClient::updateScanning(int station_found)
{
	if (station_found && m_scanning) {
		m_scanning = false;
		emit scanningChanged(m_scanning);
	}
}
