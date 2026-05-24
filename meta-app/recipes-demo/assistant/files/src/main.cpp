#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "asrmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Khởi tạo engine backend
    AsrManager asrManager;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("asrManager"), &asrManager);

    app.setDesktopFileName("assistant");

    engine.load(QUrl(QStringLiteral("qrc:/Assistant.qml")));
    if (engine.rootObjects().isEmpty()) return -1;
    return app.exec();
}