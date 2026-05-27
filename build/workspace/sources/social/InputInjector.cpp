#include "InputInjector.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>

namespace {

void sendMouseEvent(QQuickItem *item,
                    const QPointF &itemPos,
                    QEvent::Type type,
                    Qt::MouseButton button,
                    Qt::MouseButtons buttons)
{
    if (!item || !item->window())
        return;

    QQuickWindow *window = item->window();
    const QPointF windowPos = item->mapToScene(itemPos);
    const QPointF globalPos = window->mapToGlobal(windowPos.toPoint());

    QMouseEvent event(type,
                      windowPos,
                      windowPos,
                      globalPos,
                      button,
                      buttons,
                      Qt::NoModifier,
                      Qt::MouseEventNotSynthesized);
    QCoreApplication::sendEvent(window, &event);
}

} // namespace

InputInjector::InputInjector(QObject *parent)
    : QObject(parent)
{
}

bool InputInjector::clickItem(QQuickItem *item, qreal x, qreal y)
{
    if (!item || !item->window())
        return false;

    const QPointF itemPos(x, y);
    if (x < 0 || y < 0 || x > item->width() || y > item->height())
        return false;

    item->forceActiveFocus(Qt::OtherFocusReason);
    item->window()->requestActivate();

    sendMouseEvent(item, itemPos, QEvent::MouseMove, Qt::NoButton, Qt::NoButton);

    QPointer<QQuickItem> guardedItem(item);
    QTimer::singleShot(80, item, [guardedItem, itemPos]() {
        if (!guardedItem)
            return;

        sendMouseEvent(guardedItem, itemPos, QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton);
        sendMouseEvent(guardedItem, itemPos, QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton);
    });

    return true;
}
