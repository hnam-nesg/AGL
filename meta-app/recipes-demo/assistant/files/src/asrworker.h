#ifndef ASRWORKER_H
#define ASRWORKER_H

#include <QObject>
#include <QByteArray>
#include <QString>

#include "whisperengine.h"

class AsrWorker : public QObject
{
    Q_OBJECT
public:
    explicit AsrWorker(QObject *parent = nullptr);

public slots:
    void initialize(const QString &modelPath);
    void processAudioChunk(const QByteArray &pcmChunk);
    void reset();
    void stop();

signals:
    void initializedChanged(bool ready);
    void partialResultReady(const QString &text);
    void finalResultReady(const QString &text);
    void errorOccurred(const QString &message);

private:
    WhisperEngine m_engine;
    bool m_ready = false;
};

#endif // ASRWORKER_H
