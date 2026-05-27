#pragma once

#include <QObject>

class QQuickItem;

class InputInjector : public QObject
{
    Q_OBJECT

public:
    explicit InputInjector(QObject *parent = nullptr);

    Q_INVOKABLE bool clickItem(QQuickItem *item, qreal x, qreal y);
};
