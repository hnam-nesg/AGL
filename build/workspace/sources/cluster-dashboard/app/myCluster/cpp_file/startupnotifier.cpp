#include "header_file/startupnotifier.h"

#include <QDebug>
#include <QQuickWindow>
#include <QTimer>

StartupNotifier::StartupNotifier(QObject *parent)
    : QObject(parent)
{
}

void StartupNotifier::setWindow(QQuickWindow *window)
{
    m_window = window;
}

void StartupNotifier::markUiReady()
{
    if (m_uiReady || m_waitingForUiFrame)
        return;

    qInfo() << "StartupNotifier: static UI ready requested";

    if (m_window) {
        m_waitingForUiFrame = true;
        m_uiFramesBeforeReady = 2;
        m_uiFrameSwappedConnection = QObject::connect(
            m_window,
            &QQuickWindow::frameSwapped,
            this,
            [this]() {
                if (m_uiReady)
                    return;

                --m_uiFramesBeforeReady;
                if (m_uiFramesBeforeReady > 0) {
                    if (m_window)
                        m_window->update();
                    return;
                }

                QObject::disconnect(m_uiFrameSwappedConnection);
                completeUiReady();
            }
        );
        m_window->update();
        return;
    }

    completeUiReady();
}

void StartupNotifier::markReady()
{
    if (m_ready || m_waitingForFrame)
        return;

    qInfo() << "StartupNotifier: final UI ready requested";

    if (m_window) {
        m_waitingForFrame = true;
        m_framesBeforeReady = 4;
        m_frameSwappedConnection = QObject::connect(
            m_window,
            &QQuickWindow::frameSwapped,
            this,
            [this]() {
                if (m_ready)
                    return;

                --m_framesBeforeReady;
                if (m_framesBeforeReady > 0) {
                    if (m_window)
                        m_window->update();
                    return;
                }

                QObject::disconnect(m_frameSwappedConnection);
                m_waitingForFrame = false;

                QTimer::singleShot(120, this, [this]() {
                    completeReady();
                });
            }
        );
        m_window->update();
        return;
    }

    completeReady();
}

void StartupNotifier::completeUiReady()
{
    if (m_uiReady)
        return;

    m_uiReady = true;
    m_waitingForUiFrame = false;
    qInfo() << "StartupNotifier: static UI ready";
}

void StartupNotifier::completeReady()
{
    if (m_ready)
        return;

    m_ready = true;
    m_waitingForFrame = false;
    qInfo() << "StartupNotifier: final UI ready";
}
