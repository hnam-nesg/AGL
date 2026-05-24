#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>


int main(int argc, char *argv[]) {
	setenv("QT_QUICK_CONTROLS_STYLE", "AGL", 1);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    app.setDesktopFileName("drivemode");

    engine.load(QUrl(QStringLiteral("qrc:/DriveMode.qml")));
    if (engine.rootObjects().isEmpty()) return -1;
    return app.exec();
}