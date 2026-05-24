#include "header_file/fuelarc.h"
#include <QPainter>
#include <QtMath>
#include <QPainterPath>
#include <QDebug>

FuelArcImageOverlay::FuelArcImageOverlay(QQuickItem *parent)
    : QQuickPaintedItem(parent) {
    setAntialiasing(true);
    setRenderTarget(QQuickPaintedItem::Image);


    maskImage.load(":/asset/icons/Vector 1.png");
    if (maskImage.isNull())
        qWarning() << "Failed to load mask image";
}

void FuelArcImageOverlay::setFuelLevel(int level) {
    level = qBound(0, level, 100);
    if (m_fuelLevel != level) {
        m_fuelLevel = level;
        qDebug() << "FuelArc paint item level=" << m_fuelLevel;
        update();
        emit fuelLevelChanged();
    }
}

void FuelArcImageOverlay::paint(QPainter *painter) {
    if (maskImage.isNull()) return;

    QSize size = maskImage.size();
    QRectF targetRect = boundingRect();

    QImage greenLayer(size, QImage::Format_ARGB32_Premultiplied);
    greenLayer.fill(Qt::transparent);

    QPainter greenPainter(&greenLayer);
    QLinearGradient greenGrad(0, size.height(), 0, 150);
    greenGrad.setColorAt(0.0, QColor("#D910DE"));
    greenGrad.setColorAt(1.0 - (m_fuelLevel / 100.0), QColor(0, 255, 0, 0));
    greenPainter.fillRect(greenLayer.rect(), greenGrad);
    greenPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    greenPainter.drawImage(0, 0, maskImage);
    greenPainter.end();
    QImage redLayer(size, QImage::Format_ARGB32_Premultiplied);
    redLayer.fill(Qt::transparent);

    QPainter redPainter(&redLayer);
    QLinearGradient redGrad(0.0, size.height(), 0, 0);
    redGrad.setColorAt(0.0, QColor("#2C10DE"));
    redGrad.setColorAt((1.0 - (m_fuelLevel / 100.0)), QColor(255, 0, 0, 0));
    redPainter.fillRect(redLayer.rect(), redGrad);
    redPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    redPainter.drawImage(0, 0, maskImage);
    redPainter.end();

    painter->drawImage(targetRect, maskImage);
    painter->drawImage(targetRect, redLayer);
    painter->drawImage(targetRect, greenLayer);
}

////////////////////////////////////////////////////////////
CoolantArcImageOverlay::CoolantArcImageOverlay(QQuickItem *parent)
    : QQuickPaintedItem(parent) {
    setAntialiasing(true);
    setRenderTarget(QQuickPaintedItem::Image);


    maskImage.load(":/asset/icons/Vector 2.png");
    if (maskImage.isNull())
        qWarning() << "Failed to load mask image";
}

void CoolantArcImageOverlay::setFuelLevel(int level) {
    level = qBound(0, level, 100);
    if (m_fuelLevel != level) {
        m_fuelLevel = level;
        qDebug() << "CoolantArc paint item level=" << m_fuelLevel;
        update();
        emit fuelLevelChanged();
    }
}

void CoolantArcImageOverlay::paint(QPainter *painter) {
    if (maskImage.isNull()) return;

    QSize size = maskImage.size();
    QRectF targetRect = boundingRect();

    QImage greenLayer(size, QImage::Format_ARGB32_Premultiplied);
    greenLayer.fill(Qt::transparent);

    QPainter greenPainter(&greenLayer);
    QLinearGradient greenGrad(0, size.height(), 0, 150);
    greenGrad.setColorAt(0.0, QColor("#D910DE"));
    greenGrad.setColorAt(1.0 - (m_fuelLevel / 100.0), QColor(0, 255, 0, 0));
    greenPainter.fillRect(greenLayer.rect(), greenGrad);
    greenPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    greenPainter.drawImage(0, 0, maskImage);
    greenPainter.end();
    QImage redLayer(size, QImage::Format_ARGB32_Premultiplied);
    redLayer.fill(Qt::transparent);

    QPainter redPainter(&redLayer);
    QLinearGradient redGrad(0.0, size.height(), 0, 0);
    redGrad.setColorAt(0.0, QColor("#2C10DE"));
    redGrad.setColorAt((1.0 - (m_fuelLevel / 100.0)), QColor(255, 0, 0, 0));
    redPainter.fillRect(redLayer.rect(), redGrad);
    redPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    redPainter.drawImage(0, 0, maskImage);
    redPainter.end();

    painter->drawImage(targetRect, maskImage);
    painter->drawImage(targetRect, redLayer);
    painter->drawImage(targetRect, greenLayer);
}
