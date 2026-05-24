#pragma once

#include <onnxruntime_cxx_api.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class OpenWakeWordEngine {
public:
    OpenWakeWordEngine(const std::string& mel_model_path,
                       const std::string& embed_model_path,
                       const std::string& ww_model_path);
    ~OpenWakeWordEngine();

    bool initialize();
    bool processAudioChunk(const std::vector<int16_t>& audio_data);
    void reset();

    void setThreshold(float t) { threshold_ = t; }
    float lastScore() const { return last_score_; }

    void setDebugStep(int step, bool enabled);

    void setStepFrames(size_t v) { step_frames_ = (v < 1 ? 1 : v); }
    void setTriggerLevel(int v) { trigger_level_ = (v < 1 ? 1 : v); }
    void setRefractory(int v) { refractory_ = (v < 0 ? 0 : v); }

private:
    std::vector<float> runMelSpectrogram(const std::vector<float>& pcm_frame);
    std::vector<float> runEmbedding(const std::vector<float>& mel_window_76x32);
    float runWakeWord(const std::vector<float>& embed_window_16x96);

    static std::string shapeToString(const std::vector<int64_t>& shape);
    static void printFloatHead(const std::string& tag,
                               const std::vector<float>& data,
                               size_t n = 10);

private:
    Ort::Env env_;
    Ort::MemoryInfo memory_info_;

    std::unique_ptr<Ort::Session> mel_session_;
    std::unique_ptr<Ort::Session> embed_session_;
    std::unique_ptr<Ort::Session> ww_session_;

    std::string mel_model_path_;
    std::string embed_model_path_;
    std::string ww_model_path_;

    // Streaming state
    std::vector<float> sample_buffer_;     // PCM16 as float, NOT normalized
    std::vector<float> mel_buffer_;        // flattened [time, 32]
    std::vector<float> embedding_buffer_;  // flattened [time, 96]

    float threshold_ = 0.5f;
    float last_score_ = 0.0f;

    int activation_ = 0;

    int debug_step_ = -1;
    bool debug_enabled_ = false;

    // Bám theo flow file GitHub
    size_t step_frames_ = 4;     // frameSize = 4 * 1280 samples
    int trigger_level_ = 4;
    int refractory_ = 20;

    static constexpr size_t CHUNK_SAMPLES = 1280;   // 80 ms at 16 kHz
    static constexpr size_t NUM_MELS = 32;
    static constexpr size_t EMB_WINDOW_SIZE = 76;   // mel frames
    static constexpr size_t EMB_STEP_SIZE = 8;      // mel frames
    static constexpr size_t EMB_FEATURES = 96;
    static constexpr size_t WW_FEATURES = 16;       // embeddings
};