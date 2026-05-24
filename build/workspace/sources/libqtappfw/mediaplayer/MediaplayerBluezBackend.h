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

#ifndef MEDIAPLAYER_BLUEZ_BACKEND_H
#define MEDIAPLAYER_BLUEZ_BACKEND_H

#include <QObject>
#include <QtQml/QQmlContext>
#include <QThread>
#include <QTimer>
#include <QMutex>

#include "MediaplayerBackend.h"
#include "mediaplayer.h"
#include "bluetooth.h"

class MediaplayerBluezBackend : public MediaplayerBackend
{
	Q_OBJECT

public:
	explicit MediaplayerBluezBackend(Mediaplayer *player, QQmlContext *context, QObject * parent = Q_NULLPTR);
	virtual ~MediaplayerBluezBackend();

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

	// Bluetooth specific
	void connect_media();
	void disconnect_media();

signals:
	void metadataUpdate(QVariantMap metadata);

private slots:
	void updateMetadata(QVariantMap metadata);

private:
	Mediaplayer *m_player;
	Bluetooth *m_bluetooth;

        // Cached metadata to simplify refresh requests (e.g. on source switch)
        QVariantMap m_cached_metadata;
};

#endif // MEDIAPLAYER_BLUEZ_BACKEND_H
