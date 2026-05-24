/*
 * Copyright (C) 2022 Konsulko Group
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
#include "MediaplayerBluezBackend.h"
#include "mediaplayer.h"


MediaplayerBluezBackend::MediaplayerBluezBackend(Mediaplayer *player, QQmlContext *context, QObject *parent) :
	MediaplayerBackend(player, parent)
{
	m_bluetooth = new Bluetooth(false, context, true);
	if (!m_bluetooth)
		qFatal("Could not create Bluetooth");

	// Connect up metadata updates
	connect(m_bluetooth,
		&Bluetooth::mediaConnectedChanged,
		player,
		&Mediaplayer::updateBluetoothMediaConnected);
#if 0
	connect(m_bluetooth,
		&Bluetooth::mediaPropertiesChanged,
		player,
		&Mediaplayer::updateBluetoothMetadata);
#else
	// Proxy Bluetooth metadata updates so we can cache the
	// latest update locally to allow for quick refreshes.
	connect(m_bluetooth,
		&Bluetooth::mediaPropertiesChanged,
		this,
		&MediaplayerBluezBackend::updateMetadata);
	connect(this,
		&MediaplayerBluezBackend::metadataUpdate,
		player,
		&Mediaplayer::updateBluetoothMetadata);
#endif
}

MediaplayerBluezBackend::~MediaplayerBluezBackend()
{
	delete m_bluetooth;
}

void MediaplayerBluezBackend::start()
{
	m_bluetooth->start();
}

void MediaplayerBluezBackend::refresh_metadata()
{
	// Try to avoid driving a D-Bus request if we have a cached update
	if (m_cached_metadata.isEmpty())
		m_bluetooth->refresh_media_state();
	else
		emit metadataUpdate(m_cached_metadata);
}

// Slots

void MediaplayerBluezBackend::updateMetadata(QVariantMap metadata)
{
	m_cached_metadata = metadata;
	emit metadataUpdate(metadata);
}

// Control methods

void MediaplayerBluezBackend::play()
{
	m_bluetooth->media_control(Bluetooth::MediaAction::Play);
}

void MediaplayerBluezBackend::pause()
{
	m_bluetooth->media_control(Bluetooth::MediaAction::Pause);
}

void MediaplayerBluezBackend::previous()
{
	m_bluetooth->media_control(Bluetooth::MediaAction::Previous);
}

void MediaplayerBluezBackend::next()
{
	m_bluetooth->media_control(Bluetooth::MediaAction::Next);
}

void MediaplayerBluezBackend::seek(int milliseconds)
{
	// Not implemented, currently not needed by demo app
	// It is not quite obvious how this is implemented with the AVRCP
	// commands not taking a position/offset.
}

// Relative to current position
void MediaplayerBluezBackend::fastforward(int milliseconds)
{
	// Not implemented, currently not needed by demo app
	// It is not quite obvious how this is implemented with the AVRCP
	// commands not taking a position/offset.
}

// Relative to current position
void MediaplayerBluezBackend::rewind(int milliseconds)
{
	// Not implemented, currently not needed by demo app
	// It is not quite obvious how this is implemented with the AVRCP
	// commands not taking a position/offset.
}

void MediaplayerBluezBackend::picktrack(int track)
{
	// Not implemented
}

void MediaplayerBluezBackend::volume(int volume)
{
	// Not implemented
}

void MediaplayerBluezBackend::loop(QString state)
{
	// Not implemented, but potentially possible by setting player property
	// with bluez-glib addition
	m_bluetooth->media_control(Bluetooth::MediaAction::Loop);
}

void MediaplayerBluezBackend::connect_media()
{
	m_bluetooth->media_control(Bluetooth::MediaAction::Connect);
}

void MediaplayerBluezBackend::disconnect_media()
{
	m_bluetooth->media_control(Bluetooth::MediaAction::Disconnect);
}
