#pragma once

#include "VieNeuTtsConfig.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class VieNeuTtsWorker {
public:
    using Config = VieNeuTtsConfig;

    struct AudioLevelInfo {
        double rms = 0.0;
        double rmsNormalized = 0.0;
        double dbfs = -100.0;
    };

public:
    explicit VieNeuTtsWorker(const Config& cfg);
    ~VieNeuTtsWorker();

    void setAudioLevelCallback(std::function<void(double)> callback);
    void setPlaybackProgressCallback(std::function<void(double)> callback);

    void stop();
    void resetStopRequested();
    bool isPlaying() const;

    bool playPcm16WithAlsaAndMeter(const std::vector<int16_t>& samples,
                                   uint32_t sample_rate,
                                   uint16_t channels);
    bool playWavWithAlsaAndMeter(const std::string& wav_path);
    static bool loadWavPcm16File(const std::string& path,
                                 std::vector<int16_t>* samples,
                                 uint32_t* sample_rate,
                                 uint16_t* channels);

private:
    struct WavInfo {
        uint32_t sampleRate = 0;
        uint16_t channels = 0;
        uint16_t bitsPerSample = 0;
        std::vector<int16_t> samples;
    };

private:
    static bool loadWavPcm16(const std::string& path, WavInfo* out);
    static AudioLevelInfo analyzeAudioLevel(const int16_t* data, size_t sampleCount);

private:
    Config cfg_;

    std::function<void(double)> audioLevelCallback_;
    std::function<void(double)> playbackProgressCallback_;

    std::atomic<bool> stop_requested_{false};
    std::atomic<bool> playing_{false};
};
