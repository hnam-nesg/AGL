#ifndef STARTUPNOTIFIER_H
#define STARTUPNOTIFIER_H

#include <QMetaObject>
#include <QObject>
#include <QPointer>

class QQuickWindow;

class StartupNotifier : public QObject
{
    Q_OBJECT

public:
    explicit StartupNotifier(QObject *parent = nullptr);

    void setWindow(QQuickWindow *window);

    Q_INVOKABLE void markUiReady();
    Q_INVOKABLE void markReady();

private:
    void completeUiReady();
    void completeReady();

    QPointer<QQuickWindow> m_window;
    QMetaObject::Connection m_uiFrameSwappedConnection;
    QMetaObject::Connection m_frameSwappedConnection;
    bool m_uiReady = false;
    bool m_waitingForUiFrame = false;
    bool m_ready = false;
    bool m_waitingForFrame = false;
    int m_uiFramesBeforeReady = 0;
    int m_framesBeforeReady = 0;
};

#endif // STARTUPNOTIFIER_H
