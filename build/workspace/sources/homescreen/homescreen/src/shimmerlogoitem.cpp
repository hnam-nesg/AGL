#include "shimmerlogoitem.h"

#include <QDebug>
#include <QPainter>
#include <QtGlobal>

#include <algorithm>
#include <cmath>

namespace {

constexpr int ShimmerR = 235;
constexpr int ShimmerG = 248;
constexpr int ShimmerB = 255;

} // namespace

ShimmerLogoItem::ShimmerLogoItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    setOpaquePainting(false);

    m_timer.setTimerType(Qt::PreciseTimer);

    connect(&m_timer, &QTimer::timeout, this, [this]() {
        update();
    });
}

void ShimmerLogoItem::componentComplete()
{
    QQuickPaintedItem::componentComplete();

    m_completed = true;

    if (!m_elapsed.isValid())
        m_elapsed.start();

    reloadImage();
    syncTimer();
}

QUrl ShimmerLogoItem::source() const
{
    return m_source;
}

void ShimmerLogoItem::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;
    m_scaledLogo = QImage();
    m_scaledMaxSize = QSize();

    reloadImage();
    update();

    emit sourceChanged();
}

bool ShimmerLogoItem::running() const
{
    return m_running;
}

void ShimmerLogoItem::setRunning(bool running)
{
    if (m_running == running)
        return;

    m_running = running;
    if (m_running)
        m_elapsed.restart();
    syncTimer();
    update();

    emit runningChanged();
}

int ShimmerLogoItem::fps() const
{
    return m_fps;
}

void ShimmerLogoItem::setFps(int fps)
{
    fps = std::max(5, std::min(60, fps));

    if (m_fps == fps)
        return;

    m_fps = fps;
    syncTimer();

    emit fpsChanged();
}

qreal ShimmerLogoItem::logoOpacity() const
{
    return m_logoOpacity;
}

void ShimmerLogoItem::setLogoOpacity(qreal opacity)
{
    opacity = qBound<qreal>(0.0, opacity, 1.0);

    if (qFuzzyCompare(m_logoOpacity, opacity))
        return;

    m_logoOpacity = opacity;
    update();

    emit logoOpacityChanged();
}

qreal ShimmerLogoItem::shimmerSpeed() const
{
    return m_shimmerSpeed;
}

void ShimmerLogoItem::setShimmerSpeed(qreal speed)
{
    speed = std::max<qreal>(0.02, speed);

    if (qFuzzyCompare(m_shimmerSpeed, speed))
        return;

    m_shimmerSpeed = speed;
    update();

    emit shimmerSpeedChanged();
}

QString ShimmerLogoItem::imagePathFromUrl(const QUrl &url) const
{
    if (!url.isValid())
        return QString();

    if (url.scheme() == QStringLiteral("qrc"))
        return QStringLiteral(":") + url.path();

    if (url.scheme() == QStringLiteral("file"))
        return url.toLocalFile();

    if (url.scheme().isEmpty())
        return url.toString();

    return url.toString();
}

void ShimmerLogoItem::reloadImage()
{
    if (!m_completed)
        return;

    const QString path = imagePathFromUrl(m_source);

    if (path.isEmpty()) {
        m_logo = QImage();
        setImplicitWidth(0);
        setImplicitHeight(0);
        return;
    }

    QImage image;
    if (!image.load(path)) {
        qWarning() << "ShimmerLogoItem: cannot load image:" << m_source << "resolved:" << path;
        m_logo = QImage();
        setImplicitWidth(0);
        setImplicitHeight(0);
        return;
    }

    m_logo = image.convertToFormat(QImage::Format_ARGB32);
    m_scaledLogo = QImage();
    m_scaledMaxSize = QSize();
    setImplicitWidth(m_logo.width());
    setImplicitHeight(m_logo.height());
}

void ShimmerLogoItem::syncTimer()
{
    if (!m_completed)
        return;

    if (m_running) {
        if (!m_elapsed.isValid())
            m_elapsed.start();

        const int intervalMs = std::max(1, 1000 / std::max(1, m_fps));
        m_timer.start(intervalMs);
    } else {
        m_timer.stop();
    }
}

QImage ShimmerLogoItem::scaledLogoFor(const QSize &maxSize)
{
    if (m_logo.isNull() || maxSize.isEmpty())
        return QImage();

    if (!m_scaledLogo.isNull() && m_scaledMaxSize == maxSize)
        return m_scaledLogo;

    m_scaledMaxSize = maxSize;
    m_scaledLogo = m_logo.scaled(maxSize,
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation)
                           .convertToFormat(QImage::Format_ARGB32);

    return m_scaledLogo;
}

void ShimmerLogoItem::paint(QPainter *painter)
{
    if (!painter)
        return;

    const int itemW = std::max(1, static_cast<int>(width()));
    const int itemH = std::max(1, static_cast<int>(height()));

    QImage logo = scaledLogoFor(QSize(itemW, itemH));
    if (logo.isNull())
        return;

    const int logoW = logo.width();
    const int logoH = logo.height();

    const QPointF logoPos((itemW - logoW) * 0.5,
                          (itemH - logoH) * 0.5);

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    painter->save();
    painter->setOpacity(m_logoOpacity);
    painter->drawImage(logoPos, logo);
    painter->restore();

    if (!m_running)
        return;

    const qreal elapsedSec = m_elapsed.isValid()
            ? static_cast<qreal>(m_elapsed.elapsed()) / 1000.0
            : 0.0;

    const qreal phase = std::fmod(elapsedSec * m_shimmerSpeed, 1.0);
    const qreal travel = static_cast<qreal>(logoW + logoH) * 0.95;
    const qreal bandCenter = -static_cast<qreal>(logoH) * 0.32 + phase * travel;
    const qreal bandWidth = std::max<qreal>(18.0, static_cast<qreal>(logoW) * 0.075);

    QImage shimmer(logoW, logoH, QImage::Format_ARGB32);
    shimmer.fill(Qt::transparent);

    for (int y = 0; y < logoH; ++y) {
        const QRgb *srcLine = reinterpret_cast<const QRgb *>(logo.constScanLine(y));
        QRgb *dstLine = reinterpret_cast<QRgb *>(shimmer.scanLine(y));

        for (int x = 0; x < logoW; ++x) {
            const int srcAlpha = qAlpha(srcLine[x]);
            if (srcAlpha <= 0)
                continue;

            const qreal linePos = static_cast<qreal>(x) + static_cast<qreal>(y) * 0.42;
            const qreal distance = std::abs(linePos - bandCenter);

            if (distance > bandWidth)
                continue;

            const qreal strength = 1.0 - distance / bandWidth;
            int alpha = static_cast<int>((static_cast<qreal>(srcAlpha) *
                                          (44.0 + strength * 150.0)) / 255.0);
            alpha = std::max(0, std::min(255, alpha));

            dstLine[x] = qRgba(ShimmerR, ShimmerG, ShimmerB, alpha);
        }
    }

    painter->drawImage(logoPos, shimmer);
}
