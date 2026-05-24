#include "sherpaonnxengine.h"

// CHANGED: dùng sherpa-onnx C API
#include "sherpa-onnx/c-api/c-api.h"

#include <cstring>

SherpaOnnxOfflineEngine::SherpaOnnxOfflineEngine() = default;

SherpaOnnxOfflineEngine::SherpaOnnxOfflineEngine(const Config& cfg)
{
    initialize(cfg);
}

SherpaOnnxOfflineEngine::~SherpaOnnxOfflineEngine()
{
    reset();
}

bool SherpaOnnxOfflineEngine::initialize(const Config& cfg, QString* errorMessage)
{
    reset();
    cfg_ = cfg;

    if (cfg_.encoder_path.empty() ||
        cfg_.decoder_path.empty() ||
        cfg_.joiner_path.empty() ||
        cfg_.tokens_path.empty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Sherpa model paths are empty");
        }
        return false;
    }

    SherpaOnnxOfflineRecognizerConfig config;
    std::memset(&config, 0, sizeof(config));

    // CHANGED: cấu hình transducer offline
    config.model_config.transducer.encoder = cfg_.encoder_path.c_str();
    config.model_config.transducer.decoder = cfg_.decoder_path.c_str();
    config.model_config.transducer.joiner  = cfg_.joiner_path.c_str();
    config.model_config.tokens             = cfg_.tokens_path.c_str();
    config.model_config.num_threads        = cfg_.num_threads > 0 ? cfg_.num_threads : 4;
    config.model_config.provider           = cfg_.provider.c_str();
    config.model_config.debug              = cfg_.debug ? 1 : 0;

    recognizer_ = SherpaOnnxCreateOfflineRecognizer(&config);
    if (!recognizer_) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to create sherpa-onnx offline recognizer");
        }
        return false;
    }

    return true;
}

void SherpaOnnxOfflineEngine::reset()
{
    if (recognizer_) {
        SherpaOnnxDestroyOfflineRecognizer(recognizer_);
        recognizer_ = nullptr;
    }
}

bool SherpaOnnxOfflineEngine::isInitialized() const
{
    return recognizer_ != nullptr;
}

QString SherpaOnnxOfflineEngine::transcribeUtterance(const std::vector<float>& samples,
                                                     int sampleRate,
                                                     QString* errorMessage) const
{
    if (!recognizer_) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Sherpa recognizer is not initialized");
        }
        return {};
    }

    if (samples.empty()) {
        return {};
    }

    const SherpaOnnxOfflineStream* stream = SherpaOnnxCreateOfflineStream(recognizer_);
    if (!stream) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to create sherpa offline stream");
        }
        return {};
    }

    SherpaOnnxAcceptWaveformOffline(
        stream,
        sampleRate,
        samples.data(),
        static_cast<int32_t>(samples.size())
    );

    SherpaOnnxDecodeOfflineStream(recognizer_, stream);

    const SherpaOnnxOfflineRecognizerResult* result =
        SherpaOnnxGetOfflineStreamResult(stream);

    QString text;
    if (result && result->text) {
        text = QString::fromUtf8(result->text).trimmed();
    }

    if (result) {
        SherpaOnnxDestroyOfflineRecognizerResult(result);
    }
    SherpaOnnxDestroyOfflineStream(stream);

    return text;
}