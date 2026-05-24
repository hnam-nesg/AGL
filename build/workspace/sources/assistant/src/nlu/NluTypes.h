#pragma once

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace nlu {

using SlotMap = QMap<QString, QVariant>;

struct VehicleContext {
    bool acOn = false;
    int cabinTemp = -1;
    int targetTemp = -1;
    int fanLevel = -1;

    bool mediaPlaying = false;
    bool phoneIncomingCall = false;
    bool phoneActiveCall = false;
    bool navigationActive = false;
};

struct ResolvedAction {
    bool matched = false;
    bool shouldExecute = false;
    QString decision = "ASK_CLARIFY";
    QString domain = "UNKNOWN";
    QString action;
    QString reply;
    QString normalizedText;
    QString op;
    QString target;
    QString slotType;
    QStringList evidence;
    SlotMap slotValues;
    double confidence = 0.0;
};

} // namespace nlu
