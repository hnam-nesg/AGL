#include "HvacUiDbus.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

namespace {

constexpr auto kServiceName = "org.agl.hvac";
constexpr auto kObjectPath = "/org/agl/hvac";

bool registerOnConnection(QDBusConnection &connection)
{
    if (!connection.isConnected()) {
        qWarning() << "[HvacUiDbus] D-Bus connection unavailable:"
                   << connection.lastError().message();
        return false;
    }

    if (!connection.registerService(QString::fromLatin1(kServiceName))) {
        qWarning() << "[HvacUiDbus] Cannot register D-Bus service"
                   << kServiceName
                   << connection.lastError().message();
        return false;
    }

    return true;
}

int pageIndexForName(QString page)
{
    page = page.trimmed().toLower();

    if (page == QStringLiteral("0") ||
        page == QStringLiteral("seat") ||
        page == QStringLiteral("seats") ||
        page == QStringLiteral("seat_heat") ||
        page == QStringLiteral("seat-heating")) {
        return 0;
    }

    if (page == QStringLiteral("1") ||
        page == QStringLiteral("climate") ||
        page == QStringLiteral("air") ||
        page == QStringLiteral("fan") ||
        page == QStringLiteral("hvac")) {
        return 1;
    }

    return -1;
}

} // namespace

HvacUiDbus::HvacUiDbus(QObject *parent)
    : QObject(parent)
{
}

bool HvacUiDbus::registerService()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!registerOnConnection(connection)) {
        qWarning() << "[HvacUiDbus] Falling back to session bus";
        connection = QDBusConnection::sessionBus();

        if (!registerOnConnection(connection)) {
            return false;
        }
    }

    const bool ok = connection.registerObject(
        QString::fromLatin1(kObjectPath),
        this,
        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);

    if (!ok) {
        qWarning() << "[HvacUiDbus] Cannot register D-Bus object"
                   << kObjectPath
                   << connection.lastError().message();
        return false;
    }

    qInfo() << "[HvacUiDbus] Ready on" << kServiceName << kObjectPath;
    return true;
}

QString HvacUiDbus::Ping() const
{
    return QString::fromLatin1(kServiceName);
}

bool HvacUiDbus::ShowPage(const QString &page)
{
    const int pageIndex = pageIndexForName(page);
    if (pageIndex < 0) {
        qWarning() << "[HvacUiDbus] Unknown HVAC page:" << page;
        return false;
    }

    emit pageRequested(pageIndex);
    return true;
}

bool HvacUiDbus::ShowSeatPage()
{
    emit pageRequested(0);
    return true;
}

bool HvacUiDbus::ShowClimatePage()
{
    emit pageRequested(1);
    return true;
}
