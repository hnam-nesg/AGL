#include "asrmanager.h"
#include "asrworker.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QMetaObject>
#include <QDebug>

AsrManager::AsrManager(QObject *parent)
    : QObject(parent)
{
    setupWorker();
}

AsrManager::~AsrManager()
{
    stopListening();
    m_workerThread.quit();
    m_workerThread.wait();
}

QString AsrManager::state() const
{
    return m_state;
}

QString AsrManager::partialText() const
{
    return m_partialText;
}

QString AsrManager::finalText() const
{
    return m_finalText;
}

QStringList AsrManager::transcriptLines() const
{
    return m_transcriptLines;
}

bool AsrManager::listening() const
{
    return m_listening;
}

bool AsrManager::engineReady() const
{
    return m_engineReady;
}

QString AsrManager::modelPath() const
{
    return m_modelPath;
}

QString AsrManager::audioDeviceName() const
{
    return m_audioDeviceName;
}

void AsrManager::setModelPath(const QString &path)
{
    if (m_modelPath == path) {
        return;
    }
    m_modelPath = path;
    emit modelPathChanged();
}

void AsrManager::initializeEngine()
{
    setState(QStringLiteral("Initializing engine"));
    emit initializeWorker(m_modelPath);
}

void AsrManager::startListening()
{
    if (m_listening) {
        return;
    }

    if (!m_engineReady) {
        m_pendingStartRequested = true;
        initializeEngine();
        return;
    }

    m_pendingAudio.clear();
    m_partialText.clear();
    emit partialTextChanged();

    if (!setupAudioInput()) {
        return;
    }

    m_listening = true;
    emit listeningChanged();
    setState(QStringLiteral("Listening"));
    emit resetWorker();
}

void AsrManager::stopListening()
{
    if (m_audioSource) {
        m_audioSource->stop();
        m_audioSource->deleteLater();
        m_audioSource = nullptr;
        m_audioDevice = nullptr;
    }

    if (m_listening) {
        m_listening = false;
        emit listeningChanged();
    }

    m_pendingStartRequested = false;

    m_pendingAudio.clear();
    emit stopWorker();
    setState(m_engineReady ? QStringLiteral("Idle") : QStringLiteral("Engine not ready"));
}

void AsrManager::clearTranscript()
{
    m_partialText.clear();
    m_finalText.clear();
    m_transcriptLines.clear();
    emit partialTextChanged();
    emit finalTextChanged();
    emit transcriptLinesChanged();
}

void AsrManager::onAudioReadyRead()
{
    if (!m_audioDevice) {
        return;
    }

    const QByteArray data = m_audioDevice->readAll();
    if (data.isEmpty()) {
        return;
    }

    m_pendingAudio.append(data);

    const int bytesPerSecond = kSampleRate * kChannels * kBytesPerSample;
    const int chunkBytes = (bytesPerSecond * kChunkMilliseconds) / 1000;

    while (m_pendingAudio.size() >= chunkBytes) {
        const QByteArray chunk = m_pendingAudio.left(chunkBytes);
        m_pendingAudio.remove(0, chunkBytes);
        emit audioChunkReady(chunk);
    }
}

void AsrManager::onWorkerInitializedChanged(bool ready)
{
    if (m_engineReady == ready) {
        if (ready && !m_listening) {
            setState(QStringLiteral("Idle"));
        }
        return;
    }

    m_engineReady = ready;
    emit engineReadyChanged();
    setState(ready ? QStringLiteral("Idle") : QStringLiteral("Engine init failed"));

    if (ready && m_pendingStartRequested && !m_listening && !m_audioSource) {
        m_pendingStartRequested = false;
        startListening();
    }

    if (!ready) {
        m_pendingStartRequested = false;
    }
}

void AsrManager::onPartialResultReady(const QString &text)
{
    if (m_partialText == text) {
        return;
    }

    m_partialText = text;
    emit partialTextChanged();
    if (m_listening) {
        setState(QStringLiteral("Listening"));
    }
}

void AsrManager::onFinalResultReady(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }

    m_finalText = text;
    m_partialText.clear();
    m_transcriptLines.append(text);

    emit finalTextChanged();
    emit partialTextChanged();
    emit transcriptLinesChanged();

    if (m_listening) {
        setState(QStringLiteral("Listening"));
    }
}

void AsrManager::onWorkerError(const QString &message)
{
    setState(QStringLiteral("Error"));
    emit errorOccurred(message);
    qWarning() << "ASR worker error:" << message;
}

void AsrManager::setupWorker()
{
    m_worker = new AsrWorker;
    m_worker->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    connect(this, &AsrManager::initializeWorker, m_worker, &AsrWorker::initialize, Qt::QueuedConnection);
    connect(this, &AsrManager::audioChunkReady, m_worker, &AsrWorker::processAudioChunk, Qt::QueuedConnection);
    connect(this, &AsrManager::resetWorker, m_worker, &AsrWorker::reset, Qt::QueuedConnection);
    connect(this, &AsrManager::stopWorker, m_worker, &AsrWorker::stop, Qt::QueuedConnection);

    connect(m_worker, &AsrWorker::initializedChanged, this, &AsrManager::onWorkerInitializedChanged);
    connect(m_worker, &AsrWorker::partialResultReady, this, &AsrManager::onPartialResultReady);
    connect(m_worker, &AsrWorker::finalResultReady, this, &AsrManager::onFinalResultReady);
    connect(m_worker, &AsrWorker::errorOccurred, this, &AsrManager::onWorkerError);

    m_workerThread.start();
}

bool AsrManager::setupAudioInput()
{
    QAudioFormat format;
    format.setSampleRate(kSampleRate);
    format.setChannelCount(kChannels);
    format.setSampleFormat(QAudioFormat::Int16);

    const QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (inputDevice.isNull()) {
        onWorkerError(QStringLiteral("No default microphone found"));
        return false;
    }

    if (!inputDevice.isFormatSupported(format)) {
        onWorkerError(QStringLiteral("Microphone does not support 16kHz mono Int16"));
        return false;
    }

    m_audioDeviceName = inputDevice.description();
    emit audioDeviceNameChanged();

    m_audioSource = new QAudioSource(inputDevice, format, this);
    m_audioSource->setBufferSize(16 * 1024);
    m_audioDevice = m_audioSource->start();

    if (!m_audioDevice) {
        onWorkerError(QStringLiteral("Failed to start microphone capture"));
        m_audioSource->deleteLater();
        m_audioSource = nullptr;
        return false;
    }

    connect(m_audioDevice, &QIODevice::readyRead, this, &AsrManager::onAudioReadyRead);
    return true;
}

void AsrManager::setState(const QString &newState)
{
    if (m_state == newState) {
        return;
    }
    m_state = newState;
    emit stateChanged();
}
