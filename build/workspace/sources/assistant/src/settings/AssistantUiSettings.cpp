#include "AssistantUiSettings.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include <algorithm>
#include <cmath>

namespace {

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

double readDouble(const QJsonObject& object,
                  const QString& key,
                  double fallback,
                  double minValue,
                  double maxValue)
{
    return std::clamp(object.value(key).toDouble(fallback), minValue, maxValue);
}

QString normalizeColorString(const QString& color, const QString& fallback)
{
    const QColor parsed(color);
    if (!parsed.isValid()) {
        return fallback;
    }
    return parsed.name(QColor::HexRgb);
}

QString readColor(const QJsonObject& object, const QString& key, const QString& fallback)
{
    return normalizeColorString(object.value(key).toString(fallback), fallback);
}

bool nearlyEqual(double left, double right)
{
    return std::abs(left - right) < 0.0001;
}

} // namespace

AssistantUiSettings::AssistantUiSettings(QObject* parent)
    : QObject(parent)
{
    reloadTimer_.setSingleShot(true);
    reloadTimer_.setInterval(50);
    connect(&reloadTimer_, &QTimer::timeout, this, &AssistantUiSettings::loadSettings);
    connect(&watcher_, &QFileSystemWatcher::fileChanged, this, [this](const QString&) {
        scheduleReload();
    });
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, [this](const QString&) {
        scheduleReload();
    });

    loadSettings();
}

QString AssistantUiSettings::configDirectory()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (base.isEmpty()) {
        base = QStringLiteral("/tmp");
    }
    return base + QStringLiteral("/assistant");
}

QString AssistantUiSettings::settingsPath()
{
    return configDirectory() + QStringLiteral("/tts-settings.json");
}

QString AssistantUiSettings::normalizedColor(const QString& color, const QString& fallback)
{
    return normalizeColorString(color, fallback);
}

double AssistantUiSettings::lineWidth() const
{
    return lineWidth_;
}

double AssistantUiSettings::alphaCenter() const
{
    return alphaCenter_;
}

double AssistantUiSettings::alphaEdge() const
{
    return alphaEdge_;
}

double AssistantUiSettings::fragThickness() const
{
    return fragThickness_;
}

double AssistantUiSettings::fragGlow() const
{
    return fragGlow_;
}

double AssistantUiSettings::brightness() const
{
    return brightness_;
}

double AssistantUiSettings::normalOffset() const
{
    return normalOffset_;
}

QString AssistantUiSettings::shaderColor1() const
{
    return shaderColor1_;
}

QString AssistantUiSettings::shaderColor2() const
{
    return shaderColor2_;
}

QString AssistantUiSettings::shaderColor3() const
{
    return shaderColor3_;
}

void AssistantUiSettings::reload()
{
    loadSettings();
}

void AssistantUiSettings::loadSettings()
{
    QDir().mkpath(configDirectory());

    QJsonObject visual;
    bool persistDefaults = false;
    {
        QFile file(settingsPath());
        if (file.open(QIODevice::ReadOnly)) {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isObject()) {
                const QJsonObject root = doc.object();
                visual = root.value(QStringLiteral("visual")).toObject();
            } else {
                persistDefaults = true;
            }
        } else {
            persistDefaults = true;
        }
    }

    const double nextLineWidth =
        readDouble(visual, QStringLiteral("lineWidth"), 1.0, kMinLineWidth, kMaxLineWidth);
    const double nextAlphaCenter =
        readDouble(visual, QStringLiteral("alphaCenter"), 1.0, kMinAlpha, kMaxAlpha);
    const double nextAlphaEdge =
        readDouble(visual, QStringLiteral("alphaEdge"), 0.0, kMinAlpha, kMaxAlpha);
    const double nextFragThickness =
        readDouble(visual, QStringLiteral("fragThickness"), 4.0, kMinFragThickness, kMaxFragThickness);
    const double nextFragGlow =
        readDouble(visual, QStringLiteral("fragGlow"), 5.0, kMinFragGlow, kMaxFragGlow);
    const double nextBrightness =
        readDouble(visual, QStringLiteral("brightness"), 0.7, kMinBrightness, kMaxBrightness);
    const double nextNormalOffset =
        readDouble(visual, QStringLiteral("normalOffset"), 0.0, kMinNormalOffset, kMaxNormalOffset);
    const QString nextShaderColor1 =
        readColor(visual, QStringLiteral("shaderColor1"), QStringLiteral("#330099"));
    const QString nextShaderColor2 =
        readColor(visual, QStringLiteral("shaderColor2"), QStringLiteral("#004DFF"));
    const QString nextShaderColor3 =
        readColor(visual, QStringLiteral("shaderColor3"), QStringLiteral("#00FFFF"));

    const bool changed =
        !nearlyEqual(lineWidth_, nextLineWidth) ||
        !nearlyEqual(alphaCenter_, nextAlphaCenter) ||
        !nearlyEqual(alphaEdge_, nextAlphaEdge) ||
        !nearlyEqual(fragThickness_, nextFragThickness) ||
        !nearlyEqual(fragGlow_, nextFragGlow) ||
        !nearlyEqual(brightness_, nextBrightness) ||
        !nearlyEqual(normalOffset_, nextNormalOffset) ||
        shaderColor1_ != nextShaderColor1 ||
        shaderColor2_ != nextShaderColor2 ||
        shaderColor3_ != nextShaderColor3;

    lineWidth_ = nextLineWidth;
    alphaCenter_ = nextAlphaCenter;
    alphaEdge_ = nextAlphaEdge;
    fragThickness_ = nextFragThickness;
    fragGlow_ = nextFragGlow;
    brightness_ = nextBrightness;
    normalOffset_ = nextNormalOffset;
    shaderColor1_ = nextShaderColor1;
    shaderColor2_ = nextShaderColor2;
    shaderColor3_ = nextShaderColor3;

    if (persistDefaults) {
        saveSettings();
    }
    refreshWatcher();
    if (changed) {
        emit settingsChanged();
    }
}

void AssistantUiSettings::saveSettings()
{
    QDir().mkpath(configDirectory());

    QJsonObject root;

    {
        QFile readFile(settingsPath());
        if (readFile.open(QIODevice::ReadOnly)) {
            const QJsonDocument doc = QJsonDocument::fromJson(readFile.readAll());
            if (doc.isObject()) {
                root = doc.object();
            }
        }
    }

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

    root.insert(QStringLiteral("visual"), visual);

    QFile writeFile(settingsPath());
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    const QJsonDocument doc(root);
    writeFile.write(doc.toJson(QJsonDocument::Indented));
}

void AssistantUiSettings::refreshWatcher()
{
    const QString dirPath = configDirectory();
    if (!watcher_.directories().contains(dirPath)) {
        watcher_.addPath(dirPath);
    }

    const QString filePath = settingsPath();
    if (QFile::exists(filePath) && !watcher_.files().contains(filePath)) {
        watcher_.addPath(filePath);
    }
}

void AssistantUiSettings::scheduleReload()
{
    if (!reloadTimer_.isActive()) {
        reloadTimer_.start();
    }
}
