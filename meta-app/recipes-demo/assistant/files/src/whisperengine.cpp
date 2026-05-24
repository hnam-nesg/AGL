#include "whisperengine.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include <whisper.h>

namespace {
QString cleanText(const QString &input)
{
    QString text = input;
    text.replace('\n', ' ');
    text = text.simplified();
    return text;
}
}

WhisperEngine::WhisperEngine() = default;

WhisperEngine::~WhisperEngine()
{
    stop();
    if (m_ctx) {
        whisper_free(m_ctx);
        m_ctx = nullptr;
    }
}

bool WhisperEngine::initialize(const QString &modelPath, QString *errorMessage)
{
    stop();

    if (m_ctx) {
        whisper_free(m_ctx);
        m_ctx = nullptr;
    }

    whisper_context_params cparams = whisper_context_default_params();
    m_ctx = whisper_init_from_file_with_params(modelPath.toUtf8().constData(), cparams);

    if (!m_ctx) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Không thể nạp model whisper.cpp từ: %1").arg(modelPath);
        }
        m_initialized = false;
        return false;
    }

    m_modelPath = modelPath;
    m_initialized = true;
    reset();
    return true;
}

void WhisperEngine::reset()
{
    m_cancelRequested = false;
    m_audioWindow.clear();
    m_currentUtterance.clear();
    m_stepSamplesAccumulator = 0;
    m_silenceMs = 0;
    m_lastDecodedText.clear();
    m_lastFinalText.clear();
}

void WhisperEngine::stop()
{
    m_cancelRequested = true;
}

WhisperEngine::Result WhisperEngine::processPcmChunk(const QByteArray &pcmChunk)
{
    Result result;

    if (!m_initialized || pcmChunk.isEmpty() || !m_ctx) {
        return result;
    }

    const auto *pcm = reinterpret_cast<const int16_t *>(pcmChunk.constData());
    const int sampleCount = pcmChunk.size() / static_cast<int>(sizeof(int16_t));
    if (sampleCount <= 0) {
        return result;
    }

    std::vector<float> chunk;
    chunk.reserve(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        chunk.push_back(static_cast<float>(pcm[i]) / 32768.0f);
    }

    const float rms = computeChunkRms(chunk);
    const bool speechDetected = rms >= kSpeechRmsThreshold;

    m_audioWindow.insert(m_audioWindow.end(), chunk.begin(), chunk.end());
    const int maxWindowSamples = (kDecodeWindowMs * kSampleRate) / 1000;
    if (static_cast<int>(m_audioWindow.size()) > maxWindowSamples) {
        const int extra = static_cast<int>(m_audioWindow.size()) - maxWindowSamples;
        m_audioWindow.erase(m_audioWindow.begin(), m_audioWindow.begin() + extra);
    }

    if (speechDetected) {
        m_currentUtterance.insert(m_currentUtterance.end(), chunk.begin(), chunk.end());
        m_silenceMs = 0;
    } else if (!m_currentUtterance.empty()) {
        m_currentUtterance.insert(m_currentUtterance.end(), chunk.begin(), chunk.end());
        m_silenceMs += (sampleCount * 1000) / kSampleRate;
    }

    m_stepSamplesAccumulator += sampleCount;
    const int stepSamples = (kStepMs * kSampleRate) / 1000;
    const int minDecodeSamples = (kMinSpeechMsBeforeDecode * kSampleRate) / 1000;

    if (m_stepSamplesAccumulator >= stepSamples &&
        static_cast<int>(m_currentUtterance.size()) >= minDecodeSamples &&
        !m_cancelRequested) {

        m_stepSamplesAccumulator = 0;

        QString decodedText;
        if (runDecode(&decodedText) && !decodedText.isEmpty() && decodedText != m_lastDecodedText) {
            m_lastDecodedText = decodedText;
            result.partialText = decodedText;
            result.hasPartial = true;
        }
    }

    if (!m_currentUtterance.empty() && m_silenceMs >= kFinalizeSilenceMs) {
        QString finalText = m_lastDecodedText;

        if (finalText.isEmpty()) {
            runDecode(&finalText);
        }

        if (!finalText.isEmpty() && finalText != m_lastFinalText) {
            result.finalText = finalText;
            result.hasFinal = true;
            m_lastFinalText = finalText;
        }

        m_currentUtterance.clear();
        m_audioWindow.clear();
        m_stepSamplesAccumulator = 0;
        m_silenceMs = 0;
        m_lastDecodedText.clear();
    }

    return result;
}

bool WhisperEngine::runDecode(QString *decodedText, QString *errorMessage)
{
    if (!m_ctx || !decodedText) {
        return false;
    }

    const std::vector<float> *source = !m_currentUtterance.empty() ? &m_currentUtterance : &m_audioWindow;
    if (source->empty()) {
        decodedText->clear();
        return true;
    }

    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_progress = false;
    params.print_realtime = false;
    params.print_timestamps = false;
    params.print_special = false;
    params.translate = false;
    params.no_context = true;
    params.no_timestamps = true;
    params.single_segment = true;
    params.max_tokens = 64;
    params.n_threads = 4;

    const int langId = whisper_lang_id(m_language.toUtf8().constData());
    params.language = langId >= 0 ? m_language.toUtf8().constData() : "auto";

    if (whisper_full(m_ctx, params, source->data(), static_cast<int>(source->size())) != 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("whisper_full() thất bại");
        }
        return false;
    }

    QString text;
    const int segments = whisper_full_n_segments(m_ctx);
    for (int i = 0; i < segments; ++i) {
        const char *segment = whisper_full_get_segment_text(m_ctx, i);
        if (segment) {
            text += QString::fromUtf8(segment);
        }
    }

    *decodedText = normalizeSegmentText(text);
    return true;
}

QString WhisperEngine::normalizeSegmentText(const QString &text)
{
    return cleanText(text);
}

float WhisperEngine::computeChunkRms(const std::vector<float> &samples)
{
    if (samples.empty()) {
        return 0.0f;
    }

    double sumSquares = 0.0;
    for (float sample : samples) {
        sumSquares += static_cast<double>(sample) * static_cast<double>(sample);
    }

    return static_cast<float>(std::sqrt(sumSquares / static_cast<double>(samples.size())));
}
