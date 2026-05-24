/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2017 Konsulko Group
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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

#include "audiocontroller.h"
#include "bluetoothmediaplayer.h"
#include "mediadbusservice.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    BluetoothMediaPlayer mediaPlayer;

    AudioController audioController;
    audioController.setAppNameHint("mediaplayer");
    audioController.setDeviceAddress(mediaPlayer.deviceAddress());

    QObject::connect(&mediaPlayer, &BluetoothMediaPlayer::deviceAddressChanged,
                     &audioController, [&audioController, &mediaPlayer]() {
        audioController.setDeviceAddress(mediaPlayer.deviceAddress());
    });
    QObject::connect(&mediaPlayer, &BluetoothMediaPlayer::connectedChanged,
                     &audioController, [&audioController]() {
        audioController.refreshTarget();
    });

    MediaDbusService mediaDbusService(&mediaPlayer, &audioController);
    mediaDbusService.registerService();

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();

    app.setDesktopFileName("mediaplayer");

    context->setContextProperty("mediaplayer", &mediaPlayer);
    context->setContextProperty("audioController", &audioController);

    engine.load(QUrl(QStringLiteral("qrc:/MediaPlayer.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
