#ifndef FPSLOGGER_H
#define FPSLOGGER_H

#include <QElapsedTimer>
#include <QFile>
#include <QObject>
#include <QString>

class FpsLogger : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString logPath READ logPath CONSTANT)

public:
    explicit FpsLogger(QObject *parent = nullptr);
    ~FpsLogger() override;

    QString logPath() const;

    Q_INVOKABLE void start(const QString &label);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void frame();
    Q_INVOKABLE void mark(const QString &message);

private:
    bool ensureOpen();
    void writeLine(const QString &line);

    QFile m_file;
    QElapsedTimer m_sessionTimer;
    QElapsedTimer m_bucketTimer;
    QString m_label;
    qint64 m_totalFrames = 0;
    qint64 m_bucketFrames = 0;
    bool m_running = false;
};

#endif // FPSLOGGER_H
