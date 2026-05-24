#include "assistantsettingsmanager.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>
#include <QtGlobal>

#include <algorithm>

namespace {

constexpr double kMinSpeed = 0.75;
constexpr double kMaxSpeed = 1.50;
constexpr double kMinLineWidth = 0.1;
constexpr double kMaxLineWidth = 12.0;
constexpr double kMinAlpha = 0.0;
constexpr double kMaxAlpha = 1.0;
constexpr double kMinFragThickness = 0.0;
constexpr double kMaxFragThickness = 20.0;
constexpr double kMinFragGlow = 0.0;
constexpr double kMaxFragGlow = 25.0;
constexpr double kMinBrightness = 0.0;
constexpr double kMaxBrightness = 3.0;
constexpr double kMinNormalOffset = -100.0;
constexpr double kMaxNormalOffset = 100.0;
const QString kVoicesPath =
    QStringLiteral("/opt/vieneu-gguf/models/vieneu-q4/voices.json");

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

QString normalizeColorString(const QString& color, const QString& fallback)
{
    const QColor parsed(color);
    if (!parsed.isValid()) {
        return fallback;
    }
    return parsed.name(QColor::HexRgb);
}

double readDouble(const QJsonObject& object,
                  const QString& key,
                  double fallback,
                  double minValue,
                  double maxValue)
{
    return std::clamp(object.value(key).toDouble(fallback), minValue, maxValue);
}

QString readColor(const QJsonObject& object, const QString& key, const QString& fallback)
{
    return normalizeColorString(object.value(key).toString(fallback), fallback);
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

bool updateDouble(double* storage, double value, double minValue, double maxValue)
{
    if (!storage) {
        return false;
    }

    const double clamped = std::clamp(value, minValue, maxValue);
    if (qAbs(*storage - clamped) < 0.0001) {
        return false;
    }

    *storage = clamped;
    return true;
}

bool updateColor(QString* storage, const QString& value)
{
    if (!storage) {
        return false;
    }

    const QString normalized = normalizeColorString(value, *storage);
    if (normalized == *storage) {
        return false;
    }

    *storage = normalized;
    return true;
}

} // namespace

AssistantSettingsManager::AssistantSettingsManager(QObject* parent)
    : QObject(parent)
{
    loadVoices();
    loadSettings();
}

QString AssistantSettingsManager::configDirectory()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (base.isEmpty()) {
        base = QStringLiteral("/tmp");
    }
    return base + QStringLiteral("/assistant");
}

QString AssistantSettingsManager::settingsPath() const
{
    return configDirectory() + QStringLiteral("/tts-settings.json");
}

QString AssistantSettingsManager::normalizedEngine(QString engine)
{
    engine = engine.trimmed().toLower();
    if (engine == QStringLiteral("piper")) {
        return engine;
    }
    return QStringLiteral("vieneu");
}

QString AssistantSettingsManager::normalizedColor(const QString& color, const QString& fallback)
{
    return normalizeColorString(color, fallback);
}

QString AssistantSettingsManager::voiceLabelForKey(const QString& voice)
{
    if (voice == QStringLiteral("Vinh")) {
        return QStringLiteral("Vĩnh (nam miền Nam)");
    }
    if (voice == QStringLiteral("Binh")) {
        return QStringLiteral("Bình (nam miền Bắc)");
    }
    if (voice == QStringLiteral("Tuyen")) {
        return QStringLiteral("Tuyên (nam miền Bắc)");
    }
    if (voice == QStringLiteral("Doan")) {
        return QStringLiteral("Đoan (nữ miền Nam)");
    }
    if (voice == QStringLiteral("Ly")) {
        return QStringLiteral("Ly (nữ miền Bắc)");
    }
    if (voice == QStringLiteral("Ngoc")) {
        return QStringLiteral("Ngọc (nữ miền Bắc)");
    }
    return voice;
}

QString AssistantSettingsManager::engine() const
{
    return engine_;
}

void AssistantSettingsManager::setEngine(const QString& engine)
{
    const QString normalized = normalizedEngine(engine);
    if (engine_ == normalized) {
        return;
    }
    engine_ = normalized;
    saveSettings();
    emit settingsChanged();
}

double AssistantSettingsManager::speed() const
{
    return speed_;
}

void AssistantSettingsManager::setSpeed(double speed)
{
    const double clamped = std::clamp(speed, kMinSpeed, kMaxSpeed);
    if (qAbs(speed_ - clamped) < 0.001) {
        return;
    }
    speed_ = clamped;
    saveSettings();
    emit settingsChanged();
}

int AssistantSettingsManager::volume() const
{
    return volume_;
}

void AssistantSettingsManager::setVolume(int volume)
{
    const int clamped = std::clamp(volume, 0, 100);
    if (volume_ == clamped) {
        return;
    }
    volume_ = clamped;
    saveSettings();
    emit settingsChanged();
}

QString AssistantSettingsManager::voice() const
{
    return voice_;
}

void AssistantSettingsManager::setVoice(const QString& voice)
{
    const QString trimmed = voice.trimmed();
    if (trimmed.isEmpty() || voice_ == trimmed) {
        return;
    }
    voice_ = trimmed;
    saveSettings();
    emit settingsChanged();
}

double AssistantSettingsManager::lineWidth() const
{
    return lineWidth_;
}

void AssistantSettingsManager::setLineWidth(double lineWidth)
{
    if (!updateDouble(&lineWidth_, lineWidth, kMinLineWidth, kMaxLineWidth)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

double AssistantSettingsManager::alphaCenter() const
{
    return alphaCenter_;
}

void AssistantSettingsManager::setAlphaCenter(double alphaCenter)
{
    if (!updateDouble(&alphaCenter_, alphaCenter, kMinAlpha, kMaxAlpha)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

double AssistantSettingsManager::alphaEdge() const
{
    return alphaEdge_;
}

void AssistantSettingsManager::setAlphaEdge(double alphaEdge)
{
    if (!updateDouble(&alphaEdge_, alphaEdge, kMinAlpha, kMaxAlpha)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

double AssistantSettingsManager::fragThickness() const
{
    return fragThickness_;
}

void AssistantSettingsManager::setFragThickness(double fragThickness)
{
    if (!updateDouble(&fragThickness_, fragThickness, kMinFragThickness, kMaxFragThickness)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

double AssistantSettingsManager::fragGlow() const
{
    return fragGlow_;
}

void AssistantSettingsManager::setFragGlow(double fragGlow)
{
    if (!updateDouble(&fragGlow_, fragGlow, kMinFragGlow, kMaxFragGlow)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

double AssistantSettingsManager::brightness() const
{
    return brightness_;
}

void AssistantSettingsManager::setBrightness(double brightness)
{
    if (!updateDouble(&brightness_, brightness, kMinBrightness, kMaxBrightness)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

double AssistantSettingsManager::normalOffset() const
{
    return normalOffset_;
}

void AssistantSettingsManager::setNormalOffset(double normalOffset)
{
    if (!updateDouble(&normalOffset_, normalOffset, kMinNormalOffset, kMaxNormalOffset)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

QString AssistantSettingsManager::shaderColor1() const
{
    return shaderColor1_;
}

void AssistantSettingsManager::setShaderColor1(const QString& shaderColor)
{
    if (!updateColor(&shaderColor1_, shaderColor)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

QString AssistantSettingsManager::shaderColor2() const
{
    return shaderColor2_;
}

void AssistantSettingsManager::setShaderColor2(const QString& shaderColor)
{
    if (!updateColor(&shaderColor2_, shaderColor)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

QString AssistantSettingsManager::shaderColor3() const
{
    return shaderColor3_;
}

void AssistantSettingsManager::setShaderColor3(const QString& shaderColor)
{
    if (!updateColor(&shaderColor3_, shaderColor)) {
        return;
    }
    saveSettings();
    emit settingsChanged();
}

QStringList AssistantSettingsManager::voices() const
{
    return voices_;
}

QString AssistantSettingsManager::voiceDisplayName(const QString& voice) const
{
    return voiceLabelForKey(voice);
}

void AssistantSettingsManager::reload()
{
    loadVoices();
    loadSettings();
    emit voicesChanged();
    emit settingsChanged();
}

void AssistantSettingsManager::loadSettings()
{
    bool persistDefaults = false;
    {
        QFile file(settingsPath());
        if (!file.open(QIODevice::ReadOnly)) {
            persistDefaults = true;
        } else {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (!doc.isObject()) {
                persistDefaults = true;
            } else {
                const QJsonObject object = doc.object();
                engine_ = normalizedEngine(object.value(QStringLiteral("engine"))
                                               .toString(QStringLiteral("vieneu")));
                speed_ = std::clamp(object.value(QStringLiteral("speed")).toDouble(1.0),
                                    kMinSpeed,
                                    kMaxSpeed);
                volume_ = std::clamp(object.value(QStringLiteral("volume")).toInt(100),
                                     0,
                                     100);
                voice_ = object.value(QStringLiteral("voice"))
                             .toString(QStringLiteral("Vinh"))
                             .trimmed();
                if (voice_.isEmpty()) {
                    voice_ = QStringLiteral("Vinh");
                }
                if (!voices_.isEmpty() && !voices_.contains(voice_)) {
                    voice_ = voices_.first();
                }

                const QJsonObject visual = object.value(QStringLiteral("visual")).toObject();
                lineWidth_ = readDouble(visual, QStringLiteral("lineWidth"), 1.0, kMinLineWidth, kMaxLineWidth);
                alphaCenter_ = readDouble(visual, QStringLiteral("alphaCenter"), 1.0, kMinAlpha, kMaxAlpha);
                alphaEdge_ = readDouble(visual, QStringLiteral("alphaEdge"), 0.0, kMinAlpha, kMaxAlpha);
                fragThickness_ = readDouble(visual, QStringLiteral("fragThickness"), 4.0, kMinFragThickness, kMaxFragThickness);
                fragGlow_ = readDouble(visual, QStringLiteral("fragGlow"), 5.0, kMinFragGlow, kMaxFragGlow);
                brightness_ = readDouble(visual, QStringLiteral("brightness"), 0.7, kMinBrightness, kMaxBrightness);
                normalOffset_ = readDouble(visual, QStringLiteral("normalOffset"), 0.0, kMinNormalOffset, kMaxNormalOffset);
                shaderColor1_ = readColor(visual, QStringLiteral("shaderColor1"), QStringLiteral("#330099"));
                shaderColor2_ = readColor(visual, QStringLiteral("shaderColor2"), QStringLiteral("#004DFF"));
                shaderColor3_ = readColor(visual, QStringLiteral("shaderColor3"), QStringLiteral("#00FFFF"));
            }
        }
    }

    if (persistDefaults) {
        saveSettings();
    }
}

void AssistantSettingsManager::saveSettings() const
{
    QDir().mkpath(configDirectory());

    QJsonObject object;
    object.insert(QStringLiteral("engine"), engine_);
    object.insert(QStringLiteral("speed"), speed_);
    object.insert(QStringLiteral("volume"), volume_);
    object.insert(QStringLiteral("voice"), voice_);

    QJsonObject visual;
    visual.insert(QStringLiteral("lineWidth"), lineWidth_);
    visual.insert(QStringLiteral("alphaCenter"), alphaCenter_);
    visual.insert(QStringLiteral("alphaEdge"), alphaEdge_);
    visual.insert(QStringLiteral("fragThickness"), fragThickness_);
    visual.insert(QStringLiteral("fragGlow"), fragGlow_);
    visual.insert(QStringLiteral("brightness"), brightness_);
    visual.insert(QStringLiteral("normalOffset"), normalOffset_);
    visual.insert(QStringLiteral("shaderColor1"), shaderColor1_);
    visual.insert(QStringLiteral("shaderColor2"), shaderColor2_);
    visual.insert(QStringLiteral("shaderColor3"), shaderColor3_);
    object.insert(QStringLiteral("visual"), visual);

    QFile file(settingsPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning("Cannot write assistant settings");
        return;
    }
    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
}

void AssistantSettingsManager::loadVoices()
{
    QStringList voices;

    QFile file(kVoicesPath);
    if (file.open(QIODevice::ReadOnly)) {
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            const QJsonObject root = doc.object();
            const QJsonValue presets = root.value(QStringLiteral("presets"));
            const QJsonValue nestedVoices = root.value(QStringLiteral("voices"));
            if (presets.isObject()) {
                appendVoiceKeys(presets.toObject(), &voices);
            } else if (nestedVoices.isObject()) {
                appendVoiceKeys(nestedVoices.toObject(), &voices);
            } else {
                appendVoiceKeys(root, &voices);
            }
        }
    }

    voices.removeDuplicates();
    voices = orderVoices(voices);
    if (voices.isEmpty()) {
        voices.append(QStringLiteral("Vinh"));
    }

    voices_ = voices;
    if (!voices_.contains(voice_)) {
        voice_ = voices_.first();
    }
}
