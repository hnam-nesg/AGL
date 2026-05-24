// /*
//  * Copyright (C) 2016 The Qt Company Ltd.
//  * Copyright (C) 2019,2021 Konsulko Group
//  *
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  *      http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  */

// #include <QGuiApplication>
// #include <QQmlApplicationEngine>
// #include <QQmlContext>
// #include <vehiclesignals.h>

// #include "translator.h"

// int main(int argc, char *argv[])
// {
// 	setenv("QT_QUICK_CONTROLS_STYLE", "AGL", 1);

// 	QGuiApplication app(argc, argv);
// 	app.setDesktopFileName("dashboard");

// 	QQmlApplicationEngine engine;
// 	// QQmlContext *context = engine.rootContext();
// 	// VehicleSignalsConfig vsConfig("dashboard");
// 	// context->setContextProperty("VehicleSignals", new VehicleSignals(vsConfig));
// 	// qmlRegisterType<Translator>("Translator", 1, 0, "Translator");
// 	engine.load(QUrl(QStringLiteral("qrc:/Dashboard.qml")));

// 	return app.exec();
// }

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QDebug>
#include <QResource>
#include "shared_protocol.h"
#include "SharedImageProvider.h"

int main(int argc, char *argv[]) {
	setenv("QT_QUICK_CONTROLS_STYLE", "AGL", 1);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Ví dụ: đặt asset trong thư mục: <appDir>/assets/
    const QString assetDir = QCoreApplication::applicationDirPath() + "/images/";
    const QString assetBaseUrl = QUrl::fromLocalFile(assetDir).toString(); // "file:///.../assets/"

    engine.rootContext()->setContextProperty("ASSET_BASE", assetBaseUrl);
    engine.addImageProvider("shared", new SharedImageProvider("SharedImagesKey_v1"));

    engine.load(QUrl(QStringLiteral("qrc:/Dashboard.qml")));
    if (engine.rootObjects().isEmpty()) return -1;
    return app.exec();
}