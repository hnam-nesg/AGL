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
#include "MediaplayerMpdBackend.h"
#include "MpdEventHandler.h"
#include "mediaplayer.h"

// Use a 30s timeout on our MPD connection
// NOTE: The connection is actively poked at a higher frequency than
//       this to ensure we don't hit the timeout.  The alternatives
//       are to either open and close a connection for every command,
//       or try to keep the connection in idle mode when not using it.
//       The latter is deemed too complicated for our purposes for now,
//       due to it likely requiring another thread.
#define MPD_CONNECTION_TIMEOUT	30000
#define MPD_KEEPALIVE_TIMEOUT	5000

MediaplayerMpdBackend::MediaplayerMpdBackend(Mediaplayer *player, QQmlContext *context, QObject *parent) :
	MediaplayerBackend(player, parent)
{
	struct mpd_connection *conn = mpd_connection_new(NULL, 0, MPD_CONNECTION_TIMEOUT);
	if (!conn) {
		qFatal("Could not create MPD connection");
	}
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		qFatal("%s", mpd_connection_get_error_message(conn));
	}
	m_mpd_conn = conn;

	// Set up connection keepalive timer
	m_mpd_conn_timer = new QTimer(this);
	connect(m_mpd_conn_timer, &QTimer::timeout, this, &MediaplayerMpdBackend::connectionKeepaliveTimeout);
	m_mpd_conn_timer->start(MPD_KEEPALIVE_TIMEOUT);

	m_song_pos_timer = new QTimer(this);
	connect(m_song_pos_timer, &QTimer::timeout, this, &MediaplayerMpdBackend::songPositionTimeout);

	// Connect signal to push the timer-driven position updates to the player
	connect(this,
		&MediaplayerMpdBackend::positionMetadataUpdate,
		player,
		&Mediaplayer::updateLocalMetadata);

	// Create thread to handle polling for MPD events
	MpdEventHandler *handler = new MpdEventHandler();
	handler->moveToThread(&m_handlerThread);
	connect(&m_handlerThread, &QThread::finished, this, &MediaplayerMpdBackend::handleHandlerFinish);
	connect(&m_handlerThread, &QThread::finished, handler, &QObject::deleteLater);
	connect(this,
		&MediaplayerMpdBackend::startHandler,
		handler,
		&MpdEventHandler::handleEvents);

	// Connect playback state updates from the backend handler thread
	// so our view of the state is kept in sync with MPD.
	connect(handler,
		&MpdEventHandler::playbackStateUpdate,
		this,
		&MediaplayerMpdBackend::updatePlaybackState,
		Qt::QueuedConnection);

	// Connect updates from backend handler thread.
	// For now, playlist updates go directly to the parent to keep its
	// model in sync.  This would have to change if there's ever another
	// backend with playlist support, or improving abstraction is desired.
	// Current song metadata is directed to a slot here for caching before
	// sending to the parent, as having the backends be where that is
	// done seems inherently more logical.
	connect(handler,
		&MpdEventHandler::playlistUpdate,
		player,
		&Mediaplayer::updateLocalPlaylist);
	connect(handler,
		&MpdEventHandler::metadataUpdate,
		this,
		&MediaplayerMpdBackend::updateMetadata);
	connect(this,
		&MediaplayerMpdBackend::metadataUpdate,
		player,
		&Mediaplayer::updateLocalMetadata);

	m_handlerThread.start();

	// Start event handler worker loop
	emit startHandler();
}

MediaplayerMpdBackend::~MediaplayerMpdBackend()
{
	m_handlerThread.quit();
	m_handlerThread.wait();

	m_mpd_conn_timer->stop();
	delete m_mpd_conn_timer;

	mpd_connection_free(m_mpd_conn);
}

void MediaplayerMpdBackend::start()
{
	// An explicit refresh of the playlist is likely necessary
	// here to handle starting in situations where MPD has been
	// running before we were (e.g. restarting an app after a
	// crash).  At present the timing seems to be such that we
	// do not have to worry about signals being sent before the
	// app is ready, but this could be an issue down the road.
}


void MediaplayerMpdBackend::refresh_metadata()
{
	if (m_cached_metadata.isEmpty()) {
		// This can happen if the app starts up with Bluetooth
		// connected and then the source is switched.  Provide
		// empty metadata to clear up the app's state.
		//
		// Getting the metadata for the first entry in the playlist
		// to provide it here intersects with how to handle hitting
		// the end of the playlist and MPD stopping, handling that
		// is complicated enough that it is being left as future
		// development.

		QVariantMap track;
		track.insert("title", "");
		track.insert("artist", "");
		track.insert("album", "");
		track.insert("duration", 0);
		m_cached_metadata["track"] = track;
		m_cached_metadata["position"] = 0;
		m_cached_metadata["status"] = "stopped";
	}

	// The demo app currently ignores other information in an update with
	// album art, which is a historical artifact of it arriving in a second
	// update.  Until that assumption is perhaps changed, to avoid having to
	// complicate things wrt caching, we recreate this behavior using the
	// metadata we have if it contains album art.
	if (m_cached_metadata.contains("track")) {
		QVariantMap tmp = m_cached_metadata;
		QVariantMap track = tmp.value("track").toMap();
		if (track.contains("image")) {
			track.remove("image");
			tmp["track"] = track;

			// Send this as the initial no art update
			emit metadataUpdate(tmp);
		}
	}

	emit metadataUpdate(m_cached_metadata);
}

// Slots

void MediaplayerMpdBackend::connectionKeepaliveTimeout(void)
{
	m_mpd_conn_mutex.lock();

	// Clear any lingering non-fatal errors
	if (m_mpd_conn && !mpd_connection_clear_error(m_mpd_conn)) {
		// NOTE: There should likely be an attempt to reconnect here,
		//       but it definitely would complicate things for all the
		//       other users.
		qWarning() << "MPD connection in error state!";
		mpd_connection_free(m_mpd_conn);
		m_mpd_conn = NULL;
		m_mpd_conn_timer->stop();
	}
	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::handleHandlerFinish(void)
{
	m_mpd_conn_mutex.lock();

	// If the event thread finished, our connection is likely dead.
	// Given the behavior seen when mpd is hung is most client API
	// calls hang, trying to determine mpd liveliness here seems
	// problematic. Attempting to reconnect would probably be the
	// only robust solution.
	mpd_connection_free(m_mpd_conn);
	m_mpd_conn = NULL;

	m_mpd_conn_mutex.unlock();

	m_mpd_conn_timer->stop();
	delete m_mpd_conn_timer;
}


void MediaplayerMpdBackend::songPositionTimeout(void)
{
	m_state_mutex.lock();

	if (m_playing) {
		// Instead of the expense of repeatedly calling mpd_run_status,
		// provide our own elapsed time.  In practice this seems
		// sufficient for reasonable behavior in the application UI, and
		// it is what seems recommended for MPD client implementations.
		m_song_pos_ms += 250;
		QVariantMap metadata;
		metadata["position"] = m_song_pos_ms;
		m_cached_metadata["position"] = m_song_pos_ms;
		emit positionMetadataUpdate(metadata);
	}

	m_state_mutex.unlock();
}

void MediaplayerMpdBackend::updatePlaybackState(int queue_pos, int song_pos_ms, bool state)
{
	m_state_mutex.lock();

	m_queue_pos = queue_pos;
	m_song_pos_ms = song_pos_ms;
	if (m_playing != state) {
		if (state) {
			// Start position timer
			m_song_pos_timer->start(250);
		} else {
			// Stop position timer
			m_song_pos_timer->stop();
		}
	}
	m_playing = state;

	m_state_mutex.unlock();
}

void MediaplayerMpdBackend::updateMetadata(QVariantMap metadata)
{
	// Update our cached metadata to allow fast refreshes upon request
	// without having to make an out of band query to MPD.
	//
	// Updates from the event handler thread that contain track
	// information are assumed to be complete with the exception of the
	// album art, which may be included in a follow up update if it is
	// present.

	if (metadata.contains("status")) {
		QString status = metadata.value("status").toString();
		if (status == "stopped") {
			// There's likely no track information as chances are
			// this is from hitting the end of the playlist, so
			// clear things out.
			// If there actually is track metadata, it'll be
			// handled by the logic below.
			m_cached_metadata.clear();
		}
		m_cached_metadata["status"] = metadata["status"];
	}

	if (metadata.contains("track")) {
		QVariantMap track = metadata.value("track").toMap();
		m_cached_metadata["track"] = track;
	}

	// After playback starts, position updates will come from our local
	// timer, but take the initial value if present.
	if (metadata.contains("position"))
		m_cached_metadata["position"] = metadata["position"];

	// Send update up to front end
	emit metadataUpdate(metadata);
}

// Control methods

void MediaplayerMpdBackend::play()
{
	m_mpd_conn_mutex.lock();

	m_state_mutex.lock();
	if (m_mpd_conn && !m_playing) {
		mpd_run_play(m_mpd_conn);
	}
	m_state_mutex.unlock();

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::pause()
{
	m_mpd_conn_mutex.lock();

	m_state_mutex.lock();
	if (m_mpd_conn && m_playing) {
		mpd_run_pause(m_mpd_conn, true);
	}
	m_state_mutex.unlock();

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::previous()
{
	m_mpd_conn_mutex.lock();

	// MPD only allows next/previous if playing
	m_state_mutex.lock();
	if (m_mpd_conn && m_playing) {
		mpd_run_previous(m_mpd_conn);
	}
	m_state_mutex.unlock();

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::next()
{
	m_mpd_conn_mutex.lock();

	// MPD only allows next/previous if playing
	m_state_mutex.lock();
	if (m_mpd_conn && m_playing) {
		mpd_run_next(m_mpd_conn);
	}
	m_state_mutex.unlock();

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::seek(int milliseconds)
{
	m_mpd_conn_mutex.lock();

	if (m_mpd_conn) {
		float t = milliseconds;
		t /= 1000.0;
		mpd_run_seek_current(m_mpd_conn, t, false);
	}

	m_mpd_conn_mutex.unlock();
}

// Relative to current position
void MediaplayerMpdBackend::fastforward(int milliseconds)
{
	m_mpd_conn_mutex.lock();

	if (m_mpd_conn) {
		float t = milliseconds;
		t /= 1000.0;
		mpd_run_seek_current(m_mpd_conn, t, true);
	}

	m_mpd_conn_mutex.unlock();
}

// Relative to current position
void MediaplayerMpdBackend::rewind(int milliseconds)
{
	m_mpd_conn_mutex.lock();

	if (m_mpd_conn) {
		float t = -milliseconds;
		t /= 1000.0;
		mpd_run_seek_current(m_mpd_conn, t, true);
	}

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::picktrack(int track)
{
	m_mpd_conn_mutex.lock();

	if (m_mpd_conn && track >= 0) {
		mpd_run_play_pos(m_mpd_conn, track);
	}

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::volume(int volume)
{
	// Not implemented
}

void MediaplayerMpdBackend::loop(QString state)
{
	m_mpd_conn_mutex.lock();

	// Song:
	// mpd_run_single_state(m_mpd_conn, MPD_SINGLE_ON)
	// mpd_run_repeat(m_mpd_conn, true) to loop
	//
	// Playlist:
	// mpd_run_single_state(m_mpd_conn, MPD_SINGLE_OFF) (default)
	// mpd_run_repeat(m_mpd_conn, true) to loop

	if (m_mpd_conn) {
		if (state == "playlist") {
			mpd_run_repeat(m_mpd_conn, true);
		} else {
			mpd_run_repeat(m_mpd_conn, false);
		}
	}

	m_mpd_conn_mutex.unlock();
}
