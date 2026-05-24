#include "HvacService.h"

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

constexpr DbusEndpoint kHvacUiEndpoint = {
    "org.agl.hvac",
    "/org/agl/hvac",
    "org.agl.hvac"
};

bool isSeatAction(const QString& action)
{
    return action.startsWith(QStringLiteral("HVAC_SEAT_"));
}

QString pageForAction(const QString& action)
{
    if (isSeatAction(action)) {
        return QStringLiteral("seat");
    }

    if (action.startsWith(QStringLiteral("HVAC_"))) {
        return QStringLiteral("climate");
    }

    return {};
}

bool callBoolMethod(const char* logPrefix,
                    const DbusEndpoint& endpoint,
                    const QString& method,
                    const QVariantList& args = {})
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
            QString::fromLatin1(endpoint.service),
            QString::fromLatin1(endpoint.path),
            QString::fromLatin1(endpoint.interface),
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
            qWarning() << logPrefix << method
                       << "returned unexpected D-Bus reply:"
                       << reply.arguments();
            return false;
        }

        const bool ok = reply.arguments().first().toBool();
        qInfo() << logPrefix << method
                << "via" << bus.name << "bus result =" << ok;
        return ok;
    }

    qWarning() << logPrefix << method
               << "D-Bus call failed for" << endpoint.service
               << errors;
    return false;
}

void showHvacPageForAction(const QString& action)
{
    const QString page = pageForAction(action);
    if (page.isEmpty()) {
        return;
    }

    const bool ok = callBoolMethod("[HvacService]",
                                  kHvacUiEndpoint,
                                  QStringLiteral("ShowPage"),
                                  {page});
    if (!ok) {
        qWarning() << "[HvacService] HVAC UI page switch failed for action =" << action
                   << "page =" << page;
    }
}

bool validSeatLevel(const QVariant& seatValue, int* seatLevel)
{
    if (!seatValue.isValid() || seatValue.toString().isEmpty()) {
        qWarning() << "[HvacService] missing seat_level slot";
        return false;
    }

    const int level = seatValue.toInt();
    if (level < -3 || level > 3) {
        qWarning() << "[HvacService] seat_level out of range:" << level;
        return false;
    }

    if (seatLevel) {
        *seatLevel = level;
    }
    return true;
}

} // namespace

HvacService::HvacService(QObject *parent)
    : QObject(parent),
      m_vsConfig(QStringLiteral("homescreen")),
      m_vehicleSignals(new VehicleSignals(m_vsConfig, this)),
      m_hvac(new HVAC(m_vehicleSignals, this))
{
}

bool HvacService::executeAction(const QString& action, const nlu::SlotMap& slotValues)
{
    if (action.isEmpty()) {
        return false;
    }

    qInfo() << "[HvacService] EXECUTE action =" << action
            << "slotValues =" << slotValues;

    showHvacPageForAction(action);

    if (action == "HVAC_TURN_ON") {
        m_hvac->setAirCondition(true);
        return true;
    }

    if (action == "HVAC_TURN_OFF") {
        m_hvac->setAirCondition(false);
        return true;
    }

    if (action == "HVAC_AUTO_ON") {
        m_hvac->setAirAuto(true);
        return true;
    }

    if (action == "HVAC_AUTO_OFF") {
        m_hvac->setAirAuto(false);
        return true;
    }

    if (action == "HVAC_SET_TEMPERATURE") {
        const QVariant tempValue = slotValues.value("temperature");

        if (!tempValue.isValid() || tempValue.toString().isEmpty()) {
            qWarning() << "[HvacService] missing temperature slot";
            return false;
        }

        const int temp = tempValue.toInt();
        if (temp < 16 || temp > 30) {
            qWarning() << "[HvacService] temperature out of range:" << temp;
            return false;
        }

        const bool leftOk = m_hvac->setProperty("leftTemperature", temp);
        const bool rightOk = m_hvac->setProperty("rightTemperature", temp);
        return leftOk || rightOk;
    }

    if (action == "HVAC_TEMP_DOWN") {
        qInfo() << "[HvacService] TODO: decrease target temperature";
        return true;
    }

    if (action == "HVAC_TEMP_UP") {
        qInfo() << "[HvacService] TODO: increase target temperature";
        return true;
    }

    if (action == "HVAC_FAN_UP") {
        qInfo() << "[HvacService] TODO: fan up";
        return true;
    }

    if (action == "HVAC_FAN_DOWN") {
        qInfo() << "[HvacService] TODO: fan down";
        return true;
    }

    if (action == "HVAC_SET_FAN_LEVEL") {
        const QVariant fanValue = slotValues.value("fan_level");

        if (!fanValue.isValid() || fanValue.toString().isEmpty()) {
            qWarning() << "[HvacService] missing fan_level slot";
            return false;
        }

        const int fan = fanValue.toInt();
        if (fan < 1 || fan > 6) {
            qWarning() << "[HvacService] fan_level out of range:" << fan;
            return false;
        }

        return m_hvac->setProperty("fanSpeed", fan);
    }

    if (action == "HVAC_SEAT_DRIVER_SET_LEVEL") {
        int seatLevel = 0;
        if (!validSeatLevel(slotValues.value("seat_level"), &seatLevel)) {
            return false;
        }

        m_hvac->setSeatDriver(seatLevel);
        return true;
    }

    if (action == "HVAC_SEAT_PASSENGER_SET_LEVEL") {
        int seatLevel = 0;
        if (!validSeatLevel(slotValues.value("seat_level"), &seatLevel)) {
            return false;
        }

        m_hvac->setSeatPassenger(seatLevel);
        return true;
    }

    if (action == "HVAC_SEAT_REAR_LEFT_SET_LEVEL") {
        int seatLevel = 0;
        if (!validSeatLevel(slotValues.value("seat_level"), &seatLevel)) {
            return false;
        }

        m_hvac->setSeatBehindDriver(seatLevel);
        return true;
    }

    if (action == "HVAC_SEAT_REAR_RIGHT_SET_LEVEL") {
        int seatLevel = 0;
        if (!validSeatLevel(slotValues.value("seat_level"), &seatLevel)) {
            return false;
        }

        m_hvac->setSeatBehindPassenger(seatLevel);
        return true;
    }

    if (action == "HVAC_SET_AIR_SOURCE_RECIRCULATION" ||
        action == "HVAC_RECIRCULATION_ON" ||
        action == "HVAC_FRESH_AIR_OFF") {
        m_hvac->setAirRecir(true);
        return true;
    }

    if (action == "HVAC_SET_AIR_SOURCE_FRESH_AIR" ||
        action == "HVAC_FRESH_AIR_ON" ||
        action == "HVAC_RECIRCULATION_OFF") {
        m_hvac->setAirRecir(false);
        return true;
    }

    if (action == "HVAC_DEFROST_FRONT_ON") {
        m_hvac->setFrontDefrost(true);
        return true;
    }

    if (action == "HVAC_DEFROST_FRONT_OFF") {
        m_hvac->setFrontDefrost(false);
        return true;
    }

    if (action == "HVAC_DEFROST_REAR_ON") {
        m_hvac->setRearDefrost(true);
        return true;
    }

    if (action == "HVAC_DEFROST_REAR_OFF") {
        m_hvac->setRearDefrost(false);
        return true;
    }

    if (action == "HVAC_SYNC_ON") {
        return m_hvac->setProperty("syncTemp", true);
    }

    if (action == "HVAC_SYNC_OFF") {
        return m_hvac->setProperty("syncTemp", false);
    }

    if (action == "HVAC_MODE_FACE") {
        m_hvac->setAirDistribution(QStringLiteral("UP"));
        return true;
    }

    if (action == "HVAC_MODE_FEET") {
        m_hvac->setAirDistribution(QStringLiteral("DOWN"));
        return true;
    }

    if (action == "HVAC_MODE_WINDSHIELD") {
        m_hvac->setAirDistribution(QStringLiteral("UP"));
        return true;
    }

    if (action == "HVAC_MODE_FACE_FEET") {
        m_hvac->setAirDistribution(QStringLiteral("MIDDLE"));
        return true;
    }

    if (action == "HVAC_MODE_FEET_WINDSHIELD") {
        m_hvac->setAirDistribution(QStringLiteral("MIDDLE"));
        return true;
    }

    if (action == "HVAC_STATUS_QUERY") {
        qInfo() << "[HvacService] TODO: query HVAC status";
        return true;
    }

    qWarning() << "[HvacService] unknown HVAC action:" << action;
    return false;
}
