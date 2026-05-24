#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "assistantsettingsmanager.h"
#include "bluetoothmanager.h"

int main(int argc, char *argv[])
{
    setenv("QT_QUICK_CONTROLS_STYLE", "AGL", 1);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();

    /*
     * BluetoothManager nằm ở main thread.
     * Worker nằm trong QThread riêng.
     * QML chỉ nói chuyện với Manager.
     */
    BluetoothManager bluetoothManager(context);
    context->setContextProperty("bluetooth", &bluetoothManager);

    AssistantSettingsManager assistantSettings;
    context->setContextProperty("assistantSettings", &assistantSettings);

    engine.load(QUrl(QStringLiteral("qrc:/Settings.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
