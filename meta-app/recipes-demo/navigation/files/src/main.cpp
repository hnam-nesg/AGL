#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
	setenv("QT_QUICK_CONTROLS_STYLE", "AGL", 1);
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    qputenv("QT_VIRTUALKEYBOARD_LANGUAGES", "vi_VN");
    qputenv("QT_VIRTUALKEYBOARD_INPUTMODE", "telex");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

	app.setDesktopFileName("navigation");

    engine.load(QUrl(QStringLiteral("qrc:/Navigation.qml")));
    if (engine.rootObjects().isEmpty()) return -1;
    return app.exec();
}