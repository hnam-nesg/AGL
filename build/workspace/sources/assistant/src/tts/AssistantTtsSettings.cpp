#include "AssistantTtsSettings.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>

#include <algorithm>

namespace {

constexpr float kMinSpeed = 0.75f;
constexpr float kMaxSpeed = 1.50f;
constexpr int kDefaultVolume = 100;

QString settingsDirectory()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (base.isEmpty()) {
        base = QStringLiteral("/tmp");
    }
    return base + QStringLiteral("/assistant");
}

QString normalizedEngine(QString engine)
{
    engine = engine.trimmed().toLower();
    if (engine == QStringLiteral("piper")) {
        return engine;
    }
    return QStringLiteral("vieneu");
}

QStringList preferredVoiceOrder()
{
    return {
        QStringLiteral("Vinh"),
        QStringLiteral("Binh"),
        QStringLiteral("Tuyen"),
        QStringLiteral("Doan"),
        QStringLiteral("Ly"),
        QStringLiteral("Ngoc")
    };
}

float readClampedSpeed(const QJsonObject& object)
{
    return std::clamp(
        static_cast<float>(object.value(QStringLiteral("speed")).toDouble(1.0)),
        kMinSpeed,
        kMaxSpeed);
}

float readClampedVolume(const QJsonObject& object)
{
    const int volume = std::clamp(
        object.value(QStringLiteral("volume")).toInt(kDefaultVolume),
        0,
        100);
    return static_cast<float>(volume) / 100.0f;
}

void appendVoiceKeys(const QJsonObject& object, QStringList* out)
{
    if (!out) {
        return;
    }

    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (it.key() == QStringLiteral("default_voice") ||
            it.key() == QStringLiteral("defaultVoice") ||
            it.key() == QStringLiteral("meta") ||
            it.key() == QStringLiteral("metadata") ||
            it.key() == QStringLiteral("presets") ||
            it.key() == QStringLiteral("voices")) {
            continue;
        }
        if (it.value().isObject()) {
            out->append(it.key());
        }
    }
}

QStringList orderVoices(const QStringList& raw)
{
    QStringList ordered;
    const QStringList preferred = preferredVoiceOrder();

    for (const QString& key : preferred) {
        if (raw.contains(key)) {
            ordered.append(key);
        }
    }

    QStringList extras = raw;
    for (const QString& key : preferred) {
        extras.removeAll(key);
    }
    extras.removeDuplicates();
    extras.sort(Qt::CaseInsensitive);
    ordered.append(extras);
    return ordered;
}

} // namespace

QString AssistantTtsSettings::settingsPath()
{
    return settingsDirectory() + QStringLiteral("/tts-settings.json");
}

AssistantTtsRuntimeSettings AssistantTtsSettings::load()
{
    AssistantTtsRuntimeSettings settings;

    QFile file(settingsPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return settings;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return settings;
    }

    const QJsonObject object = doc.object();
    settings.engine = normalizedEngine(object.value(QStringLiteral("engine"))
                                           .toString(QStringLiteral("vieneu")))
                          .toStdString();
    settings.speed = readClampedSpeed(object);
    settings.volume = readClampedVolume(object);
    settings.voice = object.value(QStringLiteral("voice"))
                         .toString(QStringLiteral("Vinh"))
                         .trimmed()
                         .toStdString();
    if (settings.voice.empty()) {
        settings.voice = "Vinh";
    }

    return settings;
}

QStringList AssistantTtsSettings::readVieNeuVoices(const QString& voicesPath)
{
    QFile file(voicesPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {QStringLiteral("Vinh")};
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return {QStringLiteral("Vinh")};
    }

    QStringList voices;
    const QJsonObject root = doc.object();
    const QJsonValue presets = root.value(QStringLiteral("presets"));
    const QJsonValue nested = root.value(QStringLiteral("voices"));
    if (presets.isObject()) {
        appendVoiceKeys(presets.toObject(), &voices);
    } else if (nested.isObject()) {
        appendVoiceKeys(nested.toObject(), &voices);
    } else {
        appendVoiceKeys(root, &voices);
    }

    voices.removeDuplicates();
    voices = orderVoices(voices);
    if (voices.isEmpty()) {
        voices.append(QStringLiteral("Vinh"));
    }
    return voices;
}
