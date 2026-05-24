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
#include <QFileInfo>
#include <QThread>
#include "MpdEventHandler.h"

MpdEventHandler::MpdEventHandler(QObject *parent) :
	QObject(parent)
{
	// NOTE: Not specifying a timeout here as it is assumed to be long
	//       enough that we won't timeout in the brief intervals when
	//       switching in and out of idle mode.
	struct mpd_connection *conn = mpd_connection_new(NULL, 0, 0);
	if (!conn) {
		qFatal("Could not create MPD connection");
	}
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		qFatal("%s", mpd_connection_get_error_message(conn));
	}
	m_mpd_conn = conn;
}

MpdEventHandler::~MpdEventHandler()
{
	mpd_connection_free(m_mpd_conn);
}

void MpdEventHandler::handleEvents(void)
{
	bool done = false;
	enum mpd_idle mask = (enum mpd_idle)(MPD_IDLE_DATABASE|MPD_IDLE_QUEUE|MPD_IDLE_PLAYER);
	enum mpd_idle events;
	while (!done) {
		// Wait for an event
		events = mpd_run_idle_mask(m_mpd_conn, mask);

		// NOTE: Realistically should be checking the result of
		//       mpd_connection_get_error here, but handling it will
		//       complicate things significantly.

		// handle the events
		if (events & MPD_IDLE_DATABASE) {
			handleDatabaseEvent();
		}
		else if (events & MPD_IDLE_QUEUE) {
			handleQueueEvent();
		}
		else if (events & MPD_IDLE_PLAYER) {
			handlePlayerEvent();
		}
		if (mpd_connection_get_error(m_mpd_conn) != MPD_ERROR_SUCCESS) {
			done = true;
		}
	}

	qDebug() << "MpdEventHandler::handleEvents: exit";
	QThread::currentThread()->quit();
}

void MpdEventHandler::handleDatabaseEvent(void)
{
	// Maybe clear queue here?
	//mpd_run_delete_range(m_mpd_con, 0, UINT_MAX);

	if(!mpd_run_add(m_mpd_conn, "/")) {
		qWarning() << "mpd_run_add failed";
	}
}

void MpdEventHandler::handleQueueEvent(void)
{
	QVariantList playlist;
	bool done = false;
	struct mpd_entity *entity;
	mpd_send_list_queue_meta(m_mpd_conn);
	do {
		entity = mpd_recv_entity(m_mpd_conn);
		if (!entity) {
			enum mpd_error error = mpd_connection_get_error(m_mpd_conn);
			if (error == MPD_ERROR_SUCCESS)
				done = true;
			break;
		}
		if (mpd_entity_get_type(entity) != MPD_ENTITY_TYPE_SONG) {
			mpd_entity_free(entity);
			continue;
		}
		const struct mpd_song *song = mpd_entity_get_song(entity);
		QString title(mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
		QString artist(mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
		QString album(mpd_song_get_tag(song, MPD_TAG_ALBUM, 0));
		QString genre(mpd_song_get_tag(song, MPD_TAG_GENRE, 0));
		QString uri(mpd_song_get_uri(song));
		int pos = mpd_song_get_pos(song);

		if (title.isEmpty()) {
			// If there's no tag, use the filename
			QFileInfo fi(uri);
			title = fi.fileName();
		}

		//qDebug() << "Queue[" << pos << "]: " << artist << " - " << title << " / " << album << ", genre " << genre;

		QVariantMap track;
		track["title"] = title;
		track["artist"] = artist;
		track["album"] = album;
		track["genre"] = genre;
		track["index"] = pos;
		track["duration"] = mpd_song_get_duration_ms(song);
		track["path"] = uri;
		playlist.append(track);

		mpd_entity_free(entity);
	} while(!done);

	if (done) {
		QVariantMap metadata;
		metadata["list"] = playlist;

		emit playlistUpdate(metadata);
	}
}

void MpdEventHandler::handlePlayerEvent(void)
{
	int pos = -1;
	QString uri;
	QVariantMap track;
	QVariantMap metadata;
	struct mpd_song* song = mpd_run_current_song(m_mpd_conn);
	if (song) {
		QString title(mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
		QString artist(mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
		QString album(mpd_song_get_tag(song, MPD_TAG_ALBUM, 0));
		QString genre(mpd_song_get_tag(song, MPD_TAG_GENRE, 0));
		pos = mpd_song_get_pos(song);
		uri = mpd_song_get_uri(song);

		if (title.isEmpty()) {
			// If there's no tag, use the filename
			QFileInfo fi(uri);
			title = fi.fileName();
		}

		//qDebug() << "Current song[" << pos << "]: " << artist << " - " << title << " / " << album << ", genre " << genre;

		track["title"] = title;
		track["artist"] = artist;
		track["album"] = album;
		track["genre"] = genre;
		track["index"] = pos;
		track["duration"] = mpd_song_get_duration_ms(song);
		track["path"] = uri;

		mpd_song_free(song);

		metadata["track"] = track;
	}
	// NOTE:
	// It may make sense to check status first, and then try to handle the
	// no song + stopped case at the end of the queue to trigger a move to
	// the top of the playlist in the UI if that is deemed desirable (the
	// old binding does not seem to).  However, this may prove a bit
	// complicated if triggering stop instead of pause is done to trigger
	// corking in WirePlumber...

	struct mpd_status *status = mpd_run_status(m_mpd_conn);
	if (!status) {
		// mpd has gone away, attempt to get the UI into a good state
		metadata["position"] = 0;
		metadata["status"] = QString("stopped");
		emit metadataUpdate(metadata);
		emit playbackStateUpdate(pos, 0, false);
		return;
	}

	int elapsed_ms = mpd_status_get_elapsed_ms(status);
	metadata["position"] = elapsed_ms;

	int volume = mpd_status_get_volume(status);
	metadata["volume"] = volume == -1 ? 0 : volume;

	// NOTE: current UI client user does not care about paused vs stopped,
	//       and the old binding did not differentiate in its responses,
	//       so do not do so either for now.
	enum mpd_state state = mpd_status_get_state(status);
	QString status_name("stopped");
	if (state ==  MPD_STATE_PLAY)
		status_name = "playing";
	metadata["status"] = status_name;

	mpd_status_free(status);

	// For UI
	emit metadataUpdate(metadata);

	// For backend state tracking
	emit playbackStateUpdate(pos, elapsed_ms, (state == MPD_STATE_PLAY));

	if (uri.size()) {
		// Send album art to UI as a separate update.
		// This avoids things being out of sync than delaying while
		// the art is is read.
		// NOTE: Some form of caching might be desirable here.
		QByteArray buffer;
		QString mime_type;
		if (getSongArt(uri, buffer, mime_type) && mime_type.size()) {
			QString image_base64(buffer.toBase64());
			QString mime_type_header("data:");
			mime_type_header += mime_type;
			mime_type_header += ";base64,";
			image_base64.prepend(mime_type_header);

			// Re-use metadata map...
			track["image"] = image_base64;
			metadata["track"] = track;

			// ...but clear out the ephemeral metadata
			metadata.remove("position");
			metadata.remove("status");
			metadata.remove("volume");

			// Update UI
			emit metadataUpdate(metadata);
		}
	}
}

bool MpdEventHandler::getSongArt(const QString &path, QByteArray &buffer, QString &type)
{
	bool rc = false;
	unsigned pic_offset = 0;
	unsigned pic_size = 0;

	QByteArray path_ba = path.toLocal8Bit();
	const char *path_cstr = path_ba.data();

	bool first = true;
	do {
		QString pic_offset_str = QString::number(pic_offset);
		QByteArray pic_offset_ba = pic_offset_str.toLocal8Bit();
		const char *pic_offset_cstr = pic_offset_ba.data();

		if (!mpd_send_command(m_mpd_conn, "readpicture", path_cstr, pic_offset_cstr, NULL)) {
			if (mpd_connection_get_error(m_mpd_conn) != MPD_ERROR_SUCCESS)
				goto conn_error;
		}
		struct mpd_pair *pair = mpd_recv_pair_named(m_mpd_conn, "size");
		if (!pair) {
			if (first) {
				// No art, exit
				break;
			}

			if (mpd_connection_get_error(m_mpd_conn) != MPD_ERROR_SUCCESS)
				goto conn_error;
		}
		pic_size = QString(pair->value).toInt();
		mpd_return_pair(m_mpd_conn, pair);

		pair = mpd_recv_pair_named(m_mpd_conn, "type");
		if (!pair) {
			// check for error
		}
		QString mime_type(pair->value);
		mpd_return_pair(m_mpd_conn, pair);
		if (first) {
			if (mime_type.size()) {
				type = mime_type;
			} else {
				break;
			}
			first = false;
		}

		pair = mpd_recv_pair_named(m_mpd_conn, "binary");
		if (!pair) {
			if (mpd_connection_get_error(m_mpd_conn) != MPD_ERROR_SUCCESS)
				goto conn_error;
		}

		unsigned chunk_size = QString(pair->value).toInt();
		mpd_return_pair(m_mpd_conn, pair);

		if (!chunk_size)
			break;

		char *buf = new (std::nothrow) char[chunk_size];
		if (!buf)
			goto conn_error;

		if (!mpd_recv_binary(m_mpd_conn, buf, chunk_size)) {
			if (mpd_connection_get_error(m_mpd_conn) != MPD_ERROR_SUCCESS) {
				delete[] buf;
				goto conn_error;
			}
		}

		if (!mpd_response_finish(m_mpd_conn)) {
			if (mpd_connection_get_error(m_mpd_conn) != MPD_ERROR_SUCCESS) {
				delete[] buf;
				goto conn_error;
			}
		}

		buffer.append(buf, chunk_size);
		delete[] buf;

		pic_offset += chunk_size;

	} while (pic_offset < pic_size);

conn_error:
	if (pic_offset == pic_size) {
		rc = true;
	} else {
		// Don't pass garbage to caller
		buffer.clear();
	}

	return rc;
}
