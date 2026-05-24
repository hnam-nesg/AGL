#ifndef FUELARC_H
#define FUELARC_H

#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QImage>

class FuelArcImageOverlay : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(int fuelLevel READ fuelLevel WRITE setFuelLevel NOTIFY fuelLevelChanged)
    Q_PROPERTY(qreal time READ time WRITE setTime NOTIFY timeChanged)
    qreal m_time = 0.0;
    qreal time() const { return m_time; }
    void setTime(qreal t) { m_time = t; update(); emit timeChanged(); }

signals:
    void timeChanged();

public:
    FuelArcImageOverlay(QQuickItem *parent = nullptr);

    int fuelLevel() const { return m_fuelLevel; }
    void setFuelLevel(int level);

    void paint(QPainter *painter) override;

signals:
    void fuelLevelChanged();

private:
    int m_fuelLevel = 0;
    QImage maskImage;
};

class CoolantArcImageOverlay : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(int fuelLevel READ fuelLevel WRITE setFuelLevel NOTIFY fuelLevelChanged)
    Q_PROPERTY(qreal time READ time WRITE setTime NOTIFY timeChanged)
    qreal m_time = 0.0;
    qreal time() const { return m_time; }
    void setTime(qreal t) { m_time = t; update(); emit timeChanged(); }

signals:
    void timeChanged();

public:
    CoolantArcImageOverlay(QQuickItem *parent = nullptr);

    int fuelLevel() const { return m_fuelLevel; }
    void setFuelLevel(int level);

    void paint(QPainter *painter) override;

signals:
    void fuelLevelChanged();

private:
    int m_fuelLevel = 0;
    QImage maskImage;
};

#endif // FUELARC_H
