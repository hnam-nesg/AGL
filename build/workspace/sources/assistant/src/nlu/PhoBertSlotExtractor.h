#pragma once

#include "NluTypes.h"

#include <QString>

namespace nlu {

class PhoBertSlotExtractor {
public:
    struct Input {
        QString originalText;
        QString normalizedText;
        QString action;
        QString modelAction;
        QString slotType;
        QString target;
    };

    SlotMap extract(const Input& input) const;

private:
    SlotMap extractByAction(const Input& input, const QString& slotText) const;
    SlotMap extractBySlotType(const Input& input, const QString& slotText) const;
};

} // namespace nlu
