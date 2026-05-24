#pragma once

#include <onnxruntime_cxx_api.h>

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

class SileroVadEngine {
public:
    struct Config {
        std::string model_path = "/usr/share/models/silero_vad.onnx";
        int sample_rate = 16000;

        // Silero ONNX v5/v6 thực tế nên dùng 512 samples cho 16 kHz
        int window_size_samples = 512;
        int context_size_samples = 64;

        float speech_threshold = 0.5f;
        float silence_threshold = 0.35f;

        int start_trigger_chunks = 3;
        int end_trigger_chunks = 6;

        int intra_threads = 1;
        bool debug = false;
    };

    struct Result {
        bool ok = false;
        float speech_prob = 0.0f;
        bool is_speech = false;
    };

public:
    explicit SileroVadEngine(const Config& cfg);
    ~SileroVadEngine() = default;

    bool initialize(std::string* error = nullptr);
    void reset();

    bool isInitialized() const { return session_ != nullptr; }

    int windowSizeSamples() const { return cfg_.window_size_samples; }
    int sampleRate() const { return cfg_.sample_rate; }

    Result processPcm16(const std::vector<int16_t>& pcm16);
    Result processFloat(const std::vector<float>& audioFloat);

private:
    bool inspectModelIO(std::string* error);
    static std::string toLower(std::string s);
    static bool containsInsensitive(const std::string& s, const std::string& needle);
    static std::vector<float> pcm16ToFloat(const std::vector<int16_t>& in);

private:
    Config cfg_;

    Ort::Env env_{ORT_LOGGING_LEVEL_WARNING, "silero_vad"};
    std::unique_ptr<Ort::Session> session_;
    Ort::SessionOptions session_options_;

    std::string input_audio_name_;
    std::string input_sr_name_;
    std::string input_h_name_;
    std::string input_c_name_;

    std::string output_prob_name_;
    std::string output_h_name_;
    std::string output_c_name_;

    bool has_sr_input_ = false;
    bool has_hc_inputs_ = false;
    bool has_hc_outputs_ = false;

    std::vector<float> h_state_;
    std::vector<float> c_state_;
    std::vector<float> context_;
};