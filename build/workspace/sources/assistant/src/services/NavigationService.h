#pragma once

#include "../nlu/NluTypes.h"
#include <QString>

class NavigationService {
public:
    bool executeAction(const QString& action, const nlu::SlotMap& slotValues);
};
