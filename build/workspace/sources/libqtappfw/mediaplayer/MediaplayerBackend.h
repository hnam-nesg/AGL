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

#ifndef MEDIAPLAYER_BACKEND_H
#define MEDIAPLAYER_BACKEND_H

#include <QObject>
#include <QtQml/QQmlContext>
#include "mediaplayer.h"

class MediaplayerBackend : public QObject
{
	Q_OBJECT

public:
	explicit MediaplayerBackend(Mediaplayer *player, QObject * parent = Q_NULLPTR);
	virtual ~MediaplayerBackend() {};

	virtual void start() = 0;
	virtual void refresh_metadata() = 0;

	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void previous() = 0;
	virtual void next() = 0;
	virtual void seek(int) = 0;
	virtual void fastforward(int) = 0;
	virtual void rewind(int) = 0;
	virtual void picktrack(int) = 0;
	virtual void volume(int) = 0;
	virtual void loop(QString) = 0;

private:
	Mediaplayer *m_player;
};

#endif // MEDIAPLAYER_MPD_BACKEND_H
