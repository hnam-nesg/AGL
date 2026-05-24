#pragma once

#include "silero_vad_engine.h"

#include <QString>
#include <cstdint>
#include <string>
#include <vector>

#ifdef USE_RNNOISE
#include <rnnoise.h>
#endif

class CommandSegmenter {
public:
    struct Config {
        unsigned int sample_rate = 16000;

        bool enable_high_pass = false;
        float high_pass_alpha = 0.995f;
        bool enable_rnnoise = false;

        int preroll_chunks = 2;

        int no_speech_timeout_ms = 3000;
        int force_final_after_ms = 15000;
        int min_final_audio_ms = 1800;

        int partial_window_ms = 3000;
        int partial_update_interval_ms = 1200;

        float fallback_start_speech_rms_threshold = 0.05f;

        bool enable_silero_vad = true;
        SileroVadEngine::Config silero_vad_cfg;

        bool debug = false;
    };

    struct Output {
        bool speech_started = false;
        bool partial_ready = false;
        bool utterance_finalized = false;

        float speech_prob = 0.0f;
        double rms16 = 0.0;

        QString finalize_reason;

        std::vector<float> partial_window_f32;
        std::vector<float> final_utterance_f32;
        std::vector<int16_t> final_raw_pcm16;
        std::vector<int16_t> final_clean_pcm16;
    };

public:
    explicit CommandSegmenter(const Config& cfg);
    ~CommandSegmenter();

    bool initialize(QString* errorMessage = nullptr);
    void resetForNewCommand();

    void pushChunk(const std::vector<int16_t>& rawChunk, Output* out);

private:
    enum class State {
        Idle,
        ArmedAfterWake,
        InSpeech,
        Finalized
    };

    struct HighPassFilter {
        explicit HighPassFilter(float a = 0.995f) : alpha(a) {}

        void reset() {
            prevX = 0.0f;
            prevY = 0.0f;
        }

        int16_t process(int16_t x) {
            const float xf = static_cast<float>(x);
            const float y = alpha * (prevY + xf - prevX);
            prevX = xf;
            prevY = y;

            if (y > 32767.0f) return 32767;
            if (y < -32768.0f) return -32768;
            return static_cast<int16_t>(y);
        }

        float alpha = 0.995f;
        float prevX = 0.0f;
        float prevY = 0.0f;
    };

    struct VadDecision {
        bool valid = false;
        bool isSpeech = false;
        float maxSpeechProb = 0.0f;
        int processedFrames = 0;
    };

private:
    int samplesFromMs(int ms) const;
    static double calcRms16(const std::vector<int16_t>& chunk);
    std::vector<float> pcm16ToFloat(const std::vector<int16_t>& pcm16) const;

    std::vector<int16_t> denoiseChunk(const std::vector<int16_t>& in16k);
    std::vector<int16_t> gateChunk(const std::vector<int16_t>& in16k);

    VadDecision detectSpeechBySileroChunked(const std::vector<int16_t>& gateAudio);
    bool fallbackSpeechByRms(const std::vector<int16_t>& gateAudio, double* rms16Out = nullptr) const;

    void finalize(Output* out, const QString& reason);

#ifdef USE_RNNOISE
    bool initRnnoise(QString* errorMessage = nullptr);
    void destroyRnnoise();
    std::vector<int16_t> resample16kTo48k(const std::vector<int16_t>& in) const;
    std::vector<int16_t> resample48kTo16k(const std::vector<int16_t>& in) const;
    std::vector<int16_t> applyRnnoise48k(const std::vector<int16_t>& pcm48k);
#endif

private:
    Config cfg_;
    SileroVadEngine silero_;
    HighPassFilter hpf_;

    State state_ = State::Idle;

    std::vector<int16_t> vad_pending_samples_;

    std::vector<std::vector<int16_t>> preroll_raw_chunks_;
    std::vector<std::vector<int16_t>> preroll_clean_chunks_;

    std::vector<int16_t> utterance_raw_pcm16_;
    std::vector<int16_t> utterance_clean_pcm16_;
    std::vector<float> utterance_f32_;
    std::vector<float> partial_window_f32_;

    int total_received_samples_ = 0;
    int in_speech_samples_ = 0;
    int partial_samples_since_last_run_ = 0;

    int consecutive_speech_chunks_ = 0;
    int consecutive_silent_chunks_ = 0;

    bool vad_in_speech_ = false;

#ifdef USE_RNNOISE
    DenoiseState* rnnoise_state_ = nullptr;
#endif
};