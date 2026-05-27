#include <QGuiApplication>
#include <QDebug>
#include <QLocale>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSGRendererInterface>
#include <QtWebEngineQuick>
#include <QtCore/QCoreApplication>
#include <QtCore/QProcessEnvironment>

#include "mbtilesserver.h"
#include "goongrestclient.h"
#include "navigationdbusservice.h"
#include "searchcontroller.h"
#include "ThemeSettingsManager.h"

int main(int argc, char *argv[]) {
    constexpr auto kChromiumFlags =
        "--disable-vulkan "
        "--disable-features=Vulkan "
        "--use-gl=egl "
        "--ignore-gpu-blocklist "
        "--enable-webgl "
        "--disable-gpu-sandbox";

    setenv("QT_QUICK_CONTROLS_STYLE", "AGL", 1);
    setenv("QTWEBENGINE_DISABLE_SANDBOX", "1", 1);
    qputenv("QSG_RHI_BACKEND", "opengl");
    qputenv("QT_OPENGL", "es2");
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", kChromiumFlags);
    QLocale::setDefault(QLocale(QLocale::Vietnamese, QLocale::Vietnam));

    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);

    qInfo().noquote() << "NAVIGATION_RUNTIME_2026_05_09_EGL"
                      << qEnvironmentVariable("QTWEBENGINE_CHROMIUM_FLAGS");

    MbTilesServer mbTilesServer;
    mbTilesServer.start(8080);

    QQmlApplicationEngine engine;

    app.setDesktopFileName("navigation");

    SearchController searchController;
    GoongRestClient goongRestClient;
    NavigationDBusService navigationDBusService;
    ThemeSettingsManager themeSettings;
    const QString goongRestApiKey =
        QProcessEnvironment::systemEnvironment().value(QStringLiteral("GOONG_REST_API_KEY"));
    searchController.setGoongRestApiKey(goongRestApiKey);
    goongRestClient.setApiKey(goongRestApiKey);

    engine.rootContext()->setContextProperty("SearchController", &searchController);
    engine.rootContext()->setContextProperty("GoongRestClient", &goongRestClient);
    engine.rootContext()->setContextProperty("NavigationDBusService", &navigationDBusService);
    engine.rootContext()->setContextProperty("themeSettings", &themeSettings);
    engine.rootContext()->setContextProperty(QStringLiteral("GoongRestApiKey"), goongRestApiKey);
    engine.rootContext()->setContextProperty(
        QStringLiteral("GoongMapTilesKey"),
        QProcessEnvironment::systemEnvironment().value(QStringLiteral("GOONG_MAPTILES_KEY")));
    engine.rootContext()->setContextProperty(
        QStringLiteral("GoongStyleUrl"),
        QProcessEnvironment::systemEnvironment().value(
            QStringLiteral("GOONG_STYLE_URL"),
            QStringLiteral("https://tiles.goong.io/assets/goong_map_web.json")));

    engine.load(QUrl(QStringLiteral("qrc:/Navigation.qml")));
    if (engine.rootObjects().isEmpty()) return -1;
    navigationDBusService.registerService();
    return app.exec();
}
