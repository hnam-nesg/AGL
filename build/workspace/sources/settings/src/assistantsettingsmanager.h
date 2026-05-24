#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class AssistantSettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString engine READ engine WRITE setEngine NOTIFY settingsChanged)
    Q_PROPERTY(double speed READ speed WRITE setSpeed NOTIFY settingsChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY settingsChanged)
    Q_PROPERTY(QString voice READ voice WRITE setVoice NOTIFY settingsChanged)
    Q_PROPERTY(double lineWidth READ lineWidth WRITE setLineWidth NOTIFY settingsChanged)
    Q_PROPERTY(double alphaCenter READ alphaCenter WRITE setAlphaCenter NOTIFY settingsChanged)
    Q_PROPERTY(double alphaEdge READ alphaEdge WRITE setAlphaEdge NOTIFY settingsChanged)
    Q_PROPERTY(double fragThickness READ fragThickness WRITE setFragThickness NOTIFY settingsChanged)
    Q_PROPERTY(double fragGlow READ fragGlow WRITE setFragGlow NOTIFY settingsChanged)
    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY settingsChanged)
    Q_PROPERTY(double normalOffset READ normalOffset WRITE setNormalOffset NOTIFY settingsChanged)
    Q_PROPERTY(QString shaderColor1 READ shaderColor1 WRITE setShaderColor1 NOTIFY settingsChanged)
    Q_PROPERTY(QString shaderColor2 READ shaderColor2 WRITE setShaderColor2 NOTIFY settingsChanged)
    Q_PROPERTY(QString shaderColor3 READ shaderColor3 WRITE setShaderColor3 NOTIFY settingsChanged)
    Q_PROPERTY(QStringList voices READ voices NOTIFY voicesChanged)
    Q_PROPERTY(QString settingsPath READ settingsPath CONSTANT)

public:
    explicit AssistantSettingsManager(QObject* parent = nullptr);

    QString engine() const;
    void setEngine(const QString& engine);

    double speed() const;
    void setSpeed(double speed);

    int volume() const;
    void setVolume(int volume);

    QString voice() const;
    void setVoice(const QString& voice);

    double lineWidth() const;
    void setLineWidth(double lineWidth);

    double alphaCenter() const;
    void setAlphaCenter(double alphaCenter);

    double alphaEdge() const;
    void setAlphaEdge(double alphaEdge);

    double fragThickness() const;
    void setFragThickness(double fragThickness);

    double fragGlow() const;
    void setFragGlow(double fragGlow);

    double brightness() const;
    void setBrightness(double brightness);

    double normalOffset() const;
    void setNormalOffset(double normalOffset);

    QString shaderColor1() const;
    void setShaderColor1(const QString& shaderColor);

    QString shaderColor2() const;
    void setShaderColor2(const QString& shaderColor);

    QString shaderColor3() const;
    void setShaderColor3(const QString& shaderColor);

    QStringList voices() const;
    QString settingsPath() const;

    Q_INVOKABLE QString voiceDisplayName(const QString& voice) const;
    Q_INVOKABLE void reload();

signals:
    void settingsChanged();
    void voicesChanged();

private:
    void loadSettings();
    void saveSettings() const;
    void loadVoices();

    static QString configDirectory();
    static QString normalizedEngine(QString engine);
    static QString normalizedColor(const QString& color, const QString& fallback);
    static QString voiceLabelForKey(const QString& voice);

private:
    QString engine_ = QStringLiteral("vieneu");
    double speed_ = 1.0;
    int volume_ = 100;
    QString voice_ = QStringLiteral("Vinh");
    double lineWidth_ = 1.0;
    double alphaCenter_ = 1.0;
    double alphaEdge_ = 0.0;
    double fragThickness_ = 4.0;
    double fragGlow_ = 5.0;
    double brightness_ = 0.7;
    double normalOffset_ = 0.0;
    QString shaderColor1_ = QStringLiteral("#330099");
    QString shaderColor2_ = QStringLiteral("#004DFF");
    QString shaderColor3_ = QStringLiteral("#00FFFF");
    QStringList voices_{QStringLiteral("Vinh")};
};
