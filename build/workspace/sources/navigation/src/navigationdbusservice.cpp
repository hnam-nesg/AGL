#include "navigationdbusservice.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QString>

namespace {

constexpr auto kServiceName = "org.agl.navigation";
constexpr auto kObjectPath = "/org/agl/navigation";

bool registerOnConnection(QDBusConnection &connection)
{
    if (!connection.isConnected())
        return false;

    if (!connection.registerService(QString::fromLatin1(kServiceName))) {
        qWarning() << "Không đăng ký được tên D-Bus" << kServiceName
                   << connection.lastError().message();
        return false;
    }

    return true;
}

} // namespace

NavigationDBusService::NavigationDBusService(QObject *parent)
    : QObject(parent)
{
}

bool NavigationDBusService::registerService()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!registerOnConnection(connection)) {
        qWarning() << "Thử đăng ký D-Bus trên session bus";

        connection = QDBusConnection::sessionBus();

        if (!registerOnConnection(connection))
            return false;
    }

    const bool ok = connection.registerObject(
        QString::fromLatin1(kObjectPath),
        this,
        QDBusConnection::ExportAllSlots |
        QDBusConnection::ExportAllSignals
    );

    if (!ok) {
        qWarning() << "Không đăng ký được object D-Bus" << kObjectPath
                   << connection.lastError().message();
        return false;
    }

    qInfo() << "D-Bus navigation service đã sẵn sàng:"
            << kServiceName << kObjectPath;

    return true;
}

bool NavigationDBusService::NavigateTo(const QString &destination)
{
    return requestDestination(destination);
}

bool NavigationDBusService::SearchDestination(const QString &destination)
{
    return requestDestination(destination);
}

bool NavigationDBusService::navigateTo(const QString &destination)
{
    return requestDestination(destination);
}

bool NavigationDBusService::searchDestination(const QString &destination)
{
    return requestDestination(destination);
}

QString NavigationDBusService::Ping() const
{
    return QString::fromLatin1(kServiceName);
}

void NavigationDBusService::publishRouteState(bool active,
                                              double currentLatitude,
                                              double currentLongitude,
                                              double startLatitude,
                                              double startLongitude,
                                              double destinationLatitude,
                                              double destinationLongitude,
                                              double heading,
                                              const QString &instruction,
                                              const QString &nextInstruction,
                                              const QString &distanceText,
                                              const QString &timeText,
                                              const QString &routePathJson,
                                              const QString &destinationName)
{
    emit routeStateChanged(active,
                           currentLatitude,
                           currentLongitude,
                           startLatitude,
                           startLongitude,
                           destinationLatitude,
                           destinationLongitude,
                           heading,
                           instruction,
                           nextInstruction,
                           distanceText,
                           timeText,
                           routePathJson,
                           destinationName);
}

bool NavigationDBusService::requestDestination(const QString &destination)
{
    const QString trimmed = destination.trimmed();

    if (trimmed.isEmpty())
        return false;

    emit destinationRequested(trimmed);
    return true;
}
