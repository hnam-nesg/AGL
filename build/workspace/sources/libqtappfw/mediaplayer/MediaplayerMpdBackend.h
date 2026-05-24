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

#ifndef MEDIAPLAYER_MPD_BACKEND_H
#define MEDIAPLAYER_MPD_BACKEND_H

#include <QObject>
#include <QtQml/QQmlContext>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <mpd/client.h>
#include "MediaplayerBackend.h"
#include "mediaplayer.h"

class MediaplayerMpdBackend : public MediaplayerBackend
{
	Q_OBJECT

public:
	explicit MediaplayerMpdBackend(Mediaplayer *player, QQmlContext *context, QObject * parent = Q_NULLPTR);
	virtual ~MediaplayerMpdBackend();

	void start();
	void refresh_metadata();

	void play();
	void pause();
	void previous();
	void next();
	void seek(int);
	void fastforward(int);
	void rewind(int);
	void picktrack(int);
	void volume(int);
	void loop(QString);

signals:
	void startHandler(void);
        void metadataUpdate(QVariantMap metadata);
	void positionMetadataUpdate(QVariantMap metadata);

private slots:
	void connectionKeepaliveTimeout(void);
	void handleHandlerFinish(void);
	void songPositionTimeout(void);
	void updatePlaybackState(int queue_pos, int song_pos_ms, bool state);
	void updateMetadata(QVariantMap metadata);

private:
	Mediaplayer *m_player;

	// MPD connection for sending commands
	struct mpd_connection *m_mpd_conn;
	QTimer *m_mpd_conn_timer;
	QMutex m_mpd_conn_mutex;

	int m_queue_pos = -1;
	int m_song_pos_ms = 0;
	bool m_playing = false;
	QTimer *m_song_pos_timer;
	QMutex m_state_mutex;

	QThread m_handlerThread;

        // Cached metadata to simplify refresh requests (e.g. on source switch)
        QVariantMap m_cached_metadata;
};

#endif // MEDIAPLAYER_MPD_BACKEND_H
