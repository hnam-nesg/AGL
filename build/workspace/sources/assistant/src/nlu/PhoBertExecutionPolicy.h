#pragma once

#include "NluTypes.h"

#include <QString>

namespace nlu {

class PhoBertExecutionPolicy {
public:
    struct Input {
        QString modelAction;
        QString normalizedText;
        QString domain;
        QString op;
        QString target;
        QString execute;
        QString slotType;
        double confidence = 0.0;
        QString action;
        SlotMap slotValues;
    };

    ResolvedAction apply(const Input& input) const;

    static QString normalizeModelAction(const QString& action);
    static QString normalizeModelAction(const QString& action,
                                        const QString& target,
                                        const QString& slotType);

private:
    static bool hasUsableSlotValue(const SlotMap& slotValues, const QString& slotName);
    static bool isKnownExecutableServiceAction(const QString& action);
    static bool isServiceQueryAction(const QString& action);
    static bool looksLikeGeneralQuestion(const QString& text);
    static bool startsWithExplicitCommand(const QString& text);
    static QString requiredSlotForAction(const QString& action);
    static QString missingSlotReply(const QString& slotName);
    static QString replyForAction(const QString& action, const SlotMap& slotValues);
};

} // namespace nlu
