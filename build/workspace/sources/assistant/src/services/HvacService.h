#pragma once

#include "../nlu/NluTypes.h"
#include <hvac.h>
#include <vehiclesignals.h>

#include <QString>
#include <QObject>
class HvacService : public QObject {
public:
    explicit HvacService(QObject *parent = nullptr);

    bool executeAction(const QString& action, const nlu::SlotMap& slotValues);

    VehicleSignalsConfig m_vsConfig;
    VehicleSignals *m_vehicleSignals = nullptr;
    HVAC *m_hvac = nullptr;
};
