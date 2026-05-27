#pragma once

// Legacy standalone TTS adapter kept as the selectable "piper" backend.

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

class PiperTTS {
public:
    struct Config {
        std::string piper_path = "/opt/piper/piper";
        std::string model_path = "/usr/share/piper/models/vi_VN-vais1000-medium.onnx";
        std::string config_path;
        std::string espeak_data_path = "/opt/piper/espeak-ng-data";
        std::string tashkeel_model_path = "/opt/piper/libtashkeel_model.ort";
        std::string output_wav_path = "/tmp/piper_tts.wav";

        float noise_scale = 0.667f;
        float noise_w = 0.8f;
        int speaker_id = 0;
        bool quiet = true;

        // ALSA playback
        std::string alsa_playback_device = "default";
        unsigned int playback_chunk_frames = 1024;

        unsigned int output_sample_rate = 24000;
        int output_channels = 1;
        float length_scale = 1.0f;
        float playback_speed = 1.0f;
        float playback_volume = 1.0f;
    };

    struct AudioLevelInfo {
        double rms = 0.0;
        double rmsNormalized = 0.0;
        double dbfs = -100.0;
    };

public:
    explicit PiperTTS(const Config& cfg);
    ~PiperTTS();

    bool isReady() const;
    bool speak(const std::string& text);
    bool synthesizeToFile(const std::string& text, const std::string& wav_path);

    void setAudioLevelCallback(std::function<void(double)> callback);
    void setPlaybackStateCallback(std::function<void(bool)> callback);
    void setPlaybackProgressCallback(std::function<void(double)> callback);
    void setRuntimeOptions(float playbackSpeed,
                           float playbackVolume,
                           const std::string& /*voice*/);

    void stop();
    bool isSpeaking() const;

private:
    struct WavInfo {
        uint32_t sampleRate = 0;
        uint16_t channels = 0;
        uint16_t bitsPerSample = 0;
        std::vector<int16_t> samples;
    };

private:
    static std::string shellQuote(const std::string& input);
    static std::string shellQuoteEnvValue(const std::string& input);

    static bool loadWavPcm16(const std::string& path, WavInfo* out);
    static bool saveWavPcm16(const std::string& path, const WavInfo& wav);
    static AudioLevelInfo analyzeAudioLevel(const int16_t* data, size_t sampleCount);

    bool playWavWithAlsaAndMeter(const std::string& wavPath);
    void clearRuntimeState();

private:
    Config cfg_;

    std::function<void(double)> audioLevelCallback_;
    std::function<void(bool)> playbackStateCallback_;
    std::function<void(double)> playbackProgressCallback_;

    std::atomic<bool> stop_requested_{false};
    std::atomic<bool> speaking_{false};

    mutable std::mutex process_mutex_;
    int piper_pid_ = -1;
};
