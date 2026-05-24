#pragma once

// CHANGED: file mới thay cho whisperengine.h
#include <QString>
#include <string>
#include <vector>

struct SherpaOnnxOfflineRecognizer;
struct SherpaOnnxOfflineStream;
struct SherpaOnnxOfflineRecognizerResult;

class SherpaOnnxOfflineEngine
{
public:
    struct Config {
        // CHANGED: dùng bộ model sherpa-onnx bạn đã test OK
        std::string encoder_path;
        std::string decoder_path;
        std::string joiner_path;
        std::string tokens_path;

        int num_threads = 4;
        std::string provider = "cpu";
        bool debug = false;
    };

public:
    SherpaOnnxOfflineEngine();
    explicit SherpaOnnxOfflineEngine(const Config& cfg);
    ~SherpaOnnxOfflineEngine();

    bool initialize(const Config& cfg, QString* errorMessage = nullptr);
    void reset();
    bool isInitialized() const;

    // CHANGED: ASR tuần tự, chỉ nhận cả utterance rồi decode 1 lần
    QString transcribeUtterance(const std::vector<float>& samples,
                                int sampleRate,
                                QString* errorMessage = nullptr) const;

private:
    Config cfg_{};
    const SherpaOnnxOfflineRecognizer* recognizer_ = nullptr;
};