#ifndef WHISPERENGINE_H
#define WHISPERENGINE_H

#include <QString>
#include <QByteArray>
#include <vector>

struct whisper_context;
struct whisper_full_params;

class WhisperEngine
{
public:
    struct Result {
        QString partialText;
        QString finalText;
        bool hasPartial = false;
        bool hasFinal = false;
    };

    WhisperEngine();
    ~WhisperEngine();

    bool initialize(const QString &modelPath, QString *errorMessage = nullptr);
    void reset();
    void stop();
    Result processPcmChunk(const QByteArray &pcmChunk);

private:
    bool runDecode(QString *decodedText, QString *errorMessage = nullptr);
    static QString normalizeSegmentText(const QString &text);
    static float computeChunkRms(const std::vector<float> &samples);

    whisper_context *m_ctx = nullptr;
    bool m_initialized = false;
    bool m_cancelRequested = false;

    std::vector<float> m_audioWindow;
    std::vector<float> m_currentUtterance;

    int m_stepSamplesAccumulator = 0;
    int m_silenceMs = 0;
    QString m_lastDecodedText;
    QString m_lastFinalText;

    QString m_modelPath;
    QString m_language = QStringLiteral("vi");

    static constexpr int kSampleRate = 16000;
    static constexpr int kStepMs = 500;
    static constexpr int kDecodeWindowMs = 8000;
    static constexpr int kMinSpeechMsBeforeDecode = 1200;
    static constexpr int kFinalizeSilenceMs = 900;
    static constexpr float kSpeechRmsThreshold = 0.0085f;
};

#endif // WHISPERENGINE_H
