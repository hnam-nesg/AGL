#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QTimer>
#include <QDBusConnection>
#include <QDBusError>

#include "telephony.h"
#include "contactsbackend.h"
#include "phonedbusservice.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    Telephony telephony;
    ContactsBackend contactsBackend(&telephony);

    engine.rootContext()->setContextProperty("telephony", &telephony);
    engine.rootContext()->setContextProperty("contactsBackend", &contactsBackend);

    /*
     * Load danh bạ đã pull từ PBAP.
     * Nếu file chưa có, ReloadContacts có thể gọi lại sau.
     */
    //contactsBackend.loadFromVCard("/tmp/phonebook.vcf");

    /*
     * Expose D-Bus API cho assistant service.
     */
    PhoneDbusService phoneDbus(&telephony, &contactsBackend);

    QDBusConnection bus = QDBusConnection::sessionBus();

    if (!bus.registerService("org.agl.Phone")) {
        qWarning() << "[main] Failed to register D-Bus service org.agl.Phone:"
                   << bus.lastError().message();
    } else {
        qDebug() << "[main] Registered D-Bus service org.agl.Phone";
    }

    if (!bus.registerObject("/org/agl/Phone",
                            &phoneDbus,
                            QDBusConnection::ExportAllSlots
                            | QDBusConnection::ExportAllSignals)) {
        qWarning() << "[main] Failed to register D-Bus object /org/agl/Phone:"
                   << bus.lastError().message();
    } else {
        qDebug() << "[main] Registered D-Bus object /org/agl/Phone";
    }

    /*
     * Auto chọn điện thoại HFP đang connect.
     * Nếu oFono chưa sẵn sàng, dial() vẫn sẽ tự autoSelect lại.
     */
    QTimer::singleShot(2000, [&telephony]() {
        qDebug() << "[main] auto select bluetooth telephony device";
        telephony.autoSelectBluetoothDevice();
    });

    const QUrl url(QStringLiteral("qrc:/Phone.qml"));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qCritical() << "Failed to load QML:" << url;
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}