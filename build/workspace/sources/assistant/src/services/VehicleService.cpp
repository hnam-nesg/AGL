#include "VehicleService.h"

#include <QDebug>

bool VehicleService::executeAction(const QString& action, const nlu::SlotMap& slotValues)
{
    qInfo() << "[VehicleService] EXECUTE action=" << action << "slotValues=" << slotValues;

    if (action == "VEHICLE_STATUS_QUERY") {
        const QString metric = slotValues.value("vehicle_metric").toString().trimmed();
        if (metric.isEmpty()) {
            qWarning() << "[VehicleService] missing vehicle_metric slot";
            return false;
        }

        qInfo() << "[VehicleService] TODO: query vehicle metric =" << metric;
        return true;
    }

    qWarning() << "[VehicleService] unknown action:" << action;
    return false;
}
