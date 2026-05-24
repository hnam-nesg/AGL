#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QString>
#include <QTimer>

class AssistantUiSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double lineWidth READ lineWidth NOTIFY settingsChanged)
    Q_PROPERTY(double alphaCenter READ alphaCenter NOTIFY settingsChanged)
    Q_PROPERTY(double alphaEdge READ alphaEdge NOTIFY settingsChanged)
    Q_PROPERTY(double fragThickness READ fragThickness NOTIFY settingsChanged)
    Q_PROPERTY(double fragGlow READ fragGlow NOTIFY settingsChanged)
    Q_PROPERTY(double brightness READ brightness NOTIFY settingsChanged)
    Q_PROPERTY(double normalOffset READ normalOffset NOTIFY settingsChanged)
    Q_PROPERTY(QString shaderColor1 READ shaderColor1 NOTIFY settingsChanged)
    Q_PROPERTY(QString shaderColor2 READ shaderColor2 NOTIFY settingsChanged)
    Q_PROPERTY(QString shaderColor3 READ shaderColor3 NOTIFY settingsChanged)

public:
    explicit AssistantUiSettings(QObject* parent = nullptr);

    double lineWidth() const;
    double alphaCenter() const;
    double alphaEdge() const;
    double fragThickness() const;
    double fragGlow() const;
    double brightness() const;
    double normalOffset() const;
    QString shaderColor1() const;
    QString shaderColor2() const;
    QString shaderColor3() const;

    Q_INVOKABLE void reload();

signals:
    void settingsChanged();

private:
    static QString configDirectory();
    static QString settingsPath();
    static QString normalizedColor(const QString& color, const QString& fallback);

    void saveSettings();
    void loadSettings();
    void refreshWatcher();
    void scheduleReload();

private:
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

    QFileSystemWatcher watcher_;
    QTimer reloadTimer_;
};
