#include <QGuiApplication>
#include <QDebug>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <cstdlib>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QtWebEngineQuick/QtWebEngineQuick>

#include "InputInjector.h"
#include "SocialDbus.h"
#include "ThemeSettingsManager.h"

int main(int argc, char *argv[])
{
    setenv("QT_QUICK_CONTROLS_STYLE", "AGL", 1);
    setenv("QTWEBENGINE_DISABLE_SANDBOX", "1", 1);
    setenv("QT_IM_MODULE", "qtvirtualkeyboard", 1);
    setenv("QT_VIRTUALKEYBOARD_LANGUAGES", "vi_VN,en_US", 1);
    unsetenv("QT_VIRTUALKEYBOARD_DESKTOP_DISABLE");
    QLocale::setDefault(QLocale(QLocale::Vietnamese, QLocale::Vietnam));

    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName("AGL");
    QGuiApplication::setApplicationName("social");
    app.setDesktopFileName("social");

    SocialDbus socialDbus;
    InputInjector inputInjector;
    ThemeSettingsManager themeSettings;
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qWarning() << "Failed to connect to system D-Bus:" << bus.lastError().message();
    } else {
        if (!bus.registerService(QStringLiteral("org.agl.social"))) {
            qWarning() << "Failed to register D-Bus service org.agl.social:" << bus.lastError().message();
        }

        if (!bus.registerObject(QStringLiteral("/org/agl/social"), &socialDbus,
                                QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
            qWarning() << "Failed to register D-Bus object /org/agl/social:" << bus.lastError().message();
        }
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("socialDbus"), &socialDbus);
    engine.rootContext()->setContextProperty(QStringLiteral("inputInjector"), &inputInjector);
    engine.rootContext()->setContextProperty(QStringLiteral("themeSettings"), &themeSettings);
    engine.load(QUrl(QStringLiteral("qrc:/Social.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
