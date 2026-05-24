// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2016, 2017 Mentor Graphics Development (Deutschland) GmbH
 * Copyright (c) 2018 TOYOTA MOTOR CORPORATION
 * Copyright (C) 2022 Konsulko Group
 */

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QUrlQuery>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QThread>

#include "AppLauncherClient.h"

int main(int argc, char *argv[])
{
    setenv("QT_QUICK_CONTROLS_STYLE", "AGL", 1);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

	app.setDesktopFileName("launcher");

    AppLauncherClient* applauncher = new AppLauncherClient();
    engine.rootContext()->setContextProperty("applauncher", applauncher);

    engine.load(QUrl(QStringLiteral("qrc:/Launcher.qml")));
    if (engine.rootObjects().isEmpty()) return -1;

    return app.exec();
}
