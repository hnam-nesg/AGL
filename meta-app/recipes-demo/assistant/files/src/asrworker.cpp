#include "asrworker.h"

AsrWorker::AsrWorker(QObject *parent)
    : QObject(parent)
{
}

void AsrWorker::initialize(const QString &modelPath)
{
    QString errorMessage;
    m_ready = m_engine.initialize(modelPath, &errorMessage);
    emit initializedChanged(m_ready);

    if (!m_ready) {
        emit errorOccurred(errorMessage.isEmpty() ? QStringLiteral("Failed to initialize ASR engine") : errorMessage);
    }
}

void AsrWorker::processAudioChunk(const QByteArray &pcmChunk)
{
    if (!m_ready) {
        emit errorOccurred(QStringLiteral("ASR engine is not initialized"));
        return;
    }

    const WhisperEngine::Result result = m_engine.processPcmChunk(pcmChunk);

    if (result.hasPartial && !result.partialText.isEmpty()) {
        emit partialResultReady(result.partialText);
    }

    if (result.hasFinal && !result.finalText.isEmpty()) {
        emit finalResultReady(result.finalText);
    }
}

void AsrWorker::reset()
{
    if (!m_ready) {
        return;
    }
    m_engine.reset();
}

void AsrWorker::stop()
{
    if (!m_ready) {
        return;
    }
    m_engine.stop();
}
