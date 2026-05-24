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
#include <QMutexLocker>

#include "mediaplayer.h"
#include "MediaplayerMpdBackend.h"
#include "MediaplayerBluezBackend.h"


Playlist::Playlist(QVariantMap &item)
{
	m_index = item["index"].toInt();
	m_duration = item["duration"].toInt();
	m_genre = item["genre"].toString();
	m_path = item["path"].toString();
	m_title = item["title"].toString();
	m_album = item["album"].toString();
	m_artist = item["artist"].toString();
}

Playlist::~Playlist() {}

Mediaplayer::Mediaplayer(QQmlContext *context, QObject * parent) :
	QObject(parent)
{
	m_context = context;
	m_context->setContextProperty("MediaplayerModel", QVariant::fromValue(m_playlist));

	m_mpd_backend = new MediaplayerMpdBackend(this, context);
	if (!m_mpd_backend)
		qFatal("Could not create MediaplayerMpdBackend");
	m_backend = m_mpd_backend;

	m_bluez_backend = new MediaplayerBluezBackend(this, context);
	if (!m_bluez_backend)
		qFatal("Could not create MediaplayerBluezBackend");
}

Mediaplayer::~Mediaplayer()
{
	delete m_mpd_backend;
	delete m_bluez_backend;
}

void Mediaplayer::start()
{
	m_mpd_backend->start();
	m_bluez_backend->start();
}

// Qt UI Context

void Mediaplayer::updateLocalPlaylist(QVariantMap playlist)
{
	QVariantList list = playlist["list"].toList();

	m_playlist.clear();

	for (auto i : list) {
		QVariantMap item = qvariant_cast<QVariantMap>(i);
		m_playlist.append(new Playlist(item));
	}

	if (m_playlist.count() == 0) {
		QVariantMap tmp, track;

		track.insert("title", "");
		track.insert("artist", "");
		track.insert("album", "");
		track.insert("duration", 0);

		tmp.insert("position", 0);
		tmp.insert("track", track);

		// clear metadata in UI
		m_context->setContextProperty("AlbumArt", "");
		emit metadataChanged(tmp);
	}

	// Refresh model
	m_context->setContextProperty("MediaplayerModel", QVariant::fromValue(m_playlist));
}

void Mediaplayer::updateLocalMetadata(QVariantMap metadata)
{
	if (!m_bt_connected)
		updateMetadata(metadata);
}

void Mediaplayer::updateBluetoothMetadata(QVariantMap metadata)
{
	if (m_bt_connected)
		updateMetadata(metadata);
}

void Mediaplayer::updateBluetoothMediaConnected(const bool connected)
{
	if (m_bt_connected != connected) {
		QMutexLocker locker(&m_backend_mutex);
		if (connected) {
			qDebug() << "Mediaplayer::updateBluetoothMediaConnected: switching to BlueZ backend";
			m_backend = m_bluez_backend;
			m_bt_connected = connected;
			m_bluez_backend->refresh_metadata();
		} else {
			qDebug() << "Mediaplayer::updateBluetoothMediaConnected: switching to MPD backend";
			m_backend = m_mpd_backend;
			m_bt_connected = connected;
			m_mpd_backend->refresh_metadata();
		}
	}

	m_bt_operation_mutex.lock();
	if (m_bt_operation) {
		m_bt_operation = false;
	}
	// else externally driven event
	m_bt_operation_mutex.unlock();
}


// Control methods

// For backwards compatibility
void Mediaplayer::disconnect()
{
	disconnectBluetooth();
}

// For backwards compatibility
void Mediaplayer::connect()
{
	connectBluetooth();
}

void Mediaplayer::disconnectBluetooth()
{
	m_bt_operation_mutex.lock();
	if (m_bt_operation) {
		m_bt_operation_mutex.unlock();
		qDebug() << "Bluetooth media connection operation in progress, ignoring";
		return;
	}
	m_bt_operation_mutex.unlock();

	// Disconnect from Bluetooth media
	if (m_bt_connected) {
		// Explicitly pausing before disconnecting does not seem to be required
		m_bluez_backend->disconnect_media();
	}
}

void Mediaplayer::connectBluetooth()
{
	m_bt_operation_mutex.lock();
	if (m_bt_operation) {
		m_bt_operation_mutex.unlock();
		qDebug() << "Bluetooth media connection operation in progress, ignoring";
		return;
	}
	m_bt_operation_mutex.unlock();

	// Connect to Bluetooth media
	if (!m_bt_connected) {
		m_mpd_backend->pause();
		m_bluez_backend->connect_media();
	}
}

void Mediaplayer::play()
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->play();
}

void Mediaplayer::pause()
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->pause();
}

void Mediaplayer::previous()
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->previous();
}

void Mediaplayer::next()
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->next();
}

void Mediaplayer::seek(int milliseconds)
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->seek(milliseconds);
}

void Mediaplayer::fastforward(int milliseconds)
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->fastforward(milliseconds);
}

void Mediaplayer::rewind(int milliseconds)
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->rewind(milliseconds);
}

void Mediaplayer::picktrack(int track)
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->picktrack(track);
}

void Mediaplayer::volume(int volume)
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->volume(volume);
}

void Mediaplayer::loop(QString state)
{
	QMutexLocker locker(&m_backend_mutex);
	m_backend->loop(state);
}

// Private

// Common metadata helper
void Mediaplayer::updateMetadata(QVariantMap &metadata)
{
	if (metadata.contains("track")) {
		QVariantMap track = metadata.value("track").toMap();

		if (track.contains("image")) {
			m_context->setContextProperty("AlbumArt",
						      QVariant::fromValue(track.value("image")));
		}

		if (!track.contains("artist")) {
			track.insert("artist", "");
			metadata["track"] = track;
		}

		if (!track.contains("album")) {
			track.insert("album", "");
			metadata["track"] = track;
		}
	}

	// Insert current Bluetooth status
	metadata["connected"] = m_bt_connected;

	emit metadataChanged(metadata);
}
