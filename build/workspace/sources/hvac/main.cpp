#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <hvac.h>
#include <vehiclesignals.h>

#include "HvacUiDbus.h"

int main(int argc, char *argv[]) {
	setenv("QT_QUICK_CONTROLS_STYLE", "AGL", 1);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

	app.setDesktopFileName("hvac");

    VehicleSignalsConfig vsConfig("homescreen");
    HvacUiDbus hvacUiDbus;

    engine.rootContext()->setContextProperty("VehicleSignals", new VehicleSignals(vsConfig));
	engine.rootContext()->setContextProperty("hvac", new HVAC(new VehicleSignals(vsConfig)));
    engine.rootContext()->setContextProperty("hvacUiDbus", &hvacUiDbus);

    engine.load(QUrl(QStringLiteral("qrc:/HVAC.qml")));
    if (engine.rootObjects().isEmpty()) return -1;
    hvacUiDbus.registerService();
    return app.exec();
}
