#pragma once

#include "../nlu/NluTypes.h"
#include "phoneclient.h"
#include <QString>

class PhoneService {
public:
    bool executeAction(const QString& action, const nlu::SlotMap& slotValues);

private:
    PhoneClient phone_;
};
