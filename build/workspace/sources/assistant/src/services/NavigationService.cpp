#include "NavigationService.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QStringList>
#include <QVariantList>

namespace {

struct DbusEndpoint {
    const char* service;
    const char* path;
    const char* interface;
};

constexpr DbusEndpoint kNavigationEndpoint = {
    "org.agl.navigation",
    "/org/agl/navigation",
    "org.agl.navigation"
};

bool callBoolMethod(const QString& method, const QVariantList& args = {})
{
    struct CandidateBus {
        const char* name;
        QDBusConnection connection;
    };

    const CandidateBus buses[] = {
        {"system", QDBusConnection::systemBus()},
        {"session", QDBusConnection::sessionBus()}
    };

    QStringList errors;

    for (const CandidateBus& bus : buses) {
        if (!bus.connection.isConnected()) {
            errors.push_back(QStringLiteral("%1 bus unavailable: %2")
                                 .arg(QString::fromLatin1(bus.name),
                                      bus.connection.lastError().message()));
            continue;
        }

        QDBusInterface iface(
            QString::fromLatin1(kNavigationEndpoint.service),
            QString::fromLatin1(kNavigationEndpoint.path),
            QString::fromLatin1(kNavigationEndpoint.interface),
            bus.connection);

        if (!iface.isValid()) {
            errors.push_back(QStringLiteral("%1 bus invalid interface: %2")
                                 .arg(QString::fromLatin1(bus.name),
                                      iface.lastError().message()));
            continue;
        }

        const QDBusMessage reply = iface.callWithArgumentList(QDBus::Block, method, args);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            errors.push_back(QStringLiteral("%1 bus %2 failed: %3")
                                 .arg(QString::fromLatin1(bus.name),
                                      method,
                                      reply.errorMessage()));
            continue;
        }

        if (reply.arguments().isEmpty() || !reply.arguments().first().canConvert<bool>()) {
            qWarning() << "[NavigationService]" << method
                       << "returned unexpected D-Bus reply:"
                       << reply.arguments();
            return false;
        }

        const bool ok = reply.arguments().first().toBool();
        qInfo() << "[NavigationService]" << method
                << "via" << bus.name << "bus result =" << ok;
        return ok;
    }

    qWarning() << "[NavigationService]" << method
               << "D-Bus call failed for" << kNavigationEndpoint.service
               << errors;
    return false;
}

} // namespace

bool NavigationService::executeAction(const QString& action, const nlu::SlotMap& slotValues)
{
    qInfo() << "[NavigationService] EXECUTE action=" << action << "slotValues=" << slotValues;
    if (action == "NAVIGATION_START") {
        const QString destination = slotValues.value("destination").toString().trimmed();
        if (destination.isEmpty()) {
            qWarning() << "[NavigationService] missing destination slot";
            return false;
        }

        return callBoolMethod(QStringLiteral("NavigateTo"), {destination});
    }
    if (action == "NAVIGATION_SEARCH_POI") {
        const QString poi = slotValues.value("poi").toString().trimmed();
        if (poi.isEmpty()) {
            qWarning() << "[NavigationService] missing poi slot";
            return false;
        }

        return callBoolMethod(QStringLiteral("SearchDestination"), {poi});
    }
    if (action == "NAVIGATION_HOME") {
        return callBoolMethod(QStringLiteral("NavigateTo"), {QStringLiteral("home")});
    }
    if (action == "NAVIGATION_WORK") {
        return callBoolMethod(QStringLiteral("NavigateTo"), {QStringLiteral("work")});
    }
    if (action == "NAVIGATION_STOP") {
        qWarning() << "[NavigationService] org.agl.navigation has no stop method";
        return false;
    }
    qWarning() << "[NavigationService] unknown action:" << action;
    return false;
}
