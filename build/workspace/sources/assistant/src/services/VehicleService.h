#pragma once

#include "../nlu/NluTypes.h"

#include <QString>

class VehicleService {
public:
    bool executeAction(const QString& action, const nlu::SlotMap& slotValues);
};
