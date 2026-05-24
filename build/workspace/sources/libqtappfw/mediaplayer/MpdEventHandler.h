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

#ifndef MPD_EVENT_HANDLER_H
#define MPD_EVENT_HANDLER_H

#include <QObject>
#include <QVariant>
#include <mpd/client.h>

// Use a 60s timeout on our MPD connection
#define MPD_CONNECTION_TIMEOUT	60000

class MpdEventHandler : public QObject
{
	Q_OBJECT

public:
	explicit MpdEventHandler(QObject *parent = nullptr);
	virtual ~MpdEventHandler();

public slots:
	void handleEvents(void);

signals:
	void playbackStateUpdate(int queue_pos, int song_pos_ms, bool state);
        void playlistUpdate(QVariantMap playlist);
        void metadataUpdate(QVariantMap metadata);

private:
	void handleDatabaseEvent(void);
	void handleQueueEvent(void);
	void handlePlayerEvent(void);

	bool getSongArt(const QString &path, QByteArray &buffer, QString &type);

	struct mpd_connection *m_mpd_conn;
};

#endif // MPD_EVENT_HANDLER_H
