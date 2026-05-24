#pragma once

#include <QElapsedTimer>
#include <QImage>
#include <QQuickPaintedItem>
#include <QTimer>
#include <QUrl>

class ShimmerLogoItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged)
    Q_PROPERTY(qreal logoOpacity READ logoOpacity WRITE setLogoOpacity NOTIFY logoOpacityChanged)
    Q_PROPERTY(qreal shimmerSpeed READ shimmerSpeed WRITE setShimmerSpeed NOTIFY shimmerSpeedChanged)

public:
    explicit ShimmerLogoItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;
    void componentComplete() override;

    QUrl source() const;
    void setSource(const QUrl &source);

    bool running() const;
    void setRunning(bool running);

    int fps() const;
    void setFps(int fps);

    qreal logoOpacity() const;
    void setLogoOpacity(qreal opacity);

    qreal shimmerSpeed() const;
    void setShimmerSpeed(qreal speed);

signals:
    void sourceChanged();
    void runningChanged();
    void fpsChanged();
    void logoOpacityChanged();
    void shimmerSpeedChanged();

private:
    QString imagePathFromUrl(const QUrl &url) const;
    void reloadImage();
    void syncTimer();
    QImage scaledLogoFor(const QSize &maxSize);

private:
    QUrl m_source;
    QImage m_logo;
    QImage m_scaledLogo;
    QSize m_scaledMaxSize;

    QTimer m_timer;
    QElapsedTimer m_elapsed;

    bool m_completed = false;
    bool m_running = true;
    int m_fps = 30;
    qreal m_logoOpacity = 0.96;
    qreal m_shimmerSpeed = 0.42;
};
