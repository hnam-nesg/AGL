#pragma once

#include <QJsonObject>
#include <QString>

#include "../nlu/NluTypes.h"

class RuntimeContext {
public:
    bool hvacAcOn = false;
    int cabinTemp = -1;
    int targetTemp = -1;
    int fanLevel = -1;

    bool mediaPlaying = false;
    bool phoneIncomingCall = false;
    bool phoneActiveCall = false;
    bool navigationActive = false;

    QString pendingAction;
    QJsonObject pendingSlots;
    QString lastDomain;
    QString lastFeature;

    nlu::VehicleContext toVehicleContext() const;
};
