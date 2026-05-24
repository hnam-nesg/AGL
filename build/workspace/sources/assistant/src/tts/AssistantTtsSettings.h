#pragma once

#include <QString>
#include <QStringList>

#include <string>

struct AssistantTtsRuntimeSettings {
    std::string engine = "vieneu";
    float speed = 1.0f;
    float volume = 1.0f;
    std::string voice = "Vinh";
};

class AssistantTtsSettings {
public:
    static AssistantTtsRuntimeSettings load();
    static QString settingsPath();
    static QStringList readVieNeuVoices(const QString& voicesPath);
};
