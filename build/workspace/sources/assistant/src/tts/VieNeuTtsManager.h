#pragma once

#include "VieNeuTtsConfig.h"
#include "VieNeuTtsEngine.h"
#include "VieNeuTtsWorker.h"

#include <atomic>
#include <functional>
#include <string>
#include <vector>

class VieNeuTtsManager {
public:
    using Config = VieNeuTtsConfig;

public:
    explicit VieNeuTtsManager(const Config& cfg);
    ~VieNeuTtsManager();

    bool initialize();
    bool isReady() const;
    bool speak(const std::string& text);
    bool synthesizeToFile(const std::string& text, const std::string& wav_path);

    void setAudioLevelCallback(std::function<void(double)> callback);
    void setPlaybackStateCallback(std::function<void(bool)> callback);
    void setPlaybackProgressCallback(std::function<void(double)> callback);
    void setRuntimeOptions(float playbackSpeed,
                           float playbackVolume,
                           const std::string& voice);

    void stop();
    bool isSpeaking() const;

private:
    std::vector<std::string> splitTextForSynthesis(const std::string& text) const;
    std::string cacheFileNameForChunk(const std::string& text) const;
    std::string presetPathForChunk(const std::string& text) const;
    std::string cachePathForChunk(const std::string& text) const;
    std::vector<int16_t> prepareForPlayback(const std::vector<int16_t>& pcm,
                                            uint32_t sample_rate) const;
    void applyPlaybackVolume(std::vector<int16_t>* pcm) const;

private:
    Config cfg_;
    VieNeuTtsEngine engine_;
    VieNeuTtsWorker worker_;

    std::function<void(double)> audioLevelCallback_;
    std::function<void(bool)> playbackStateCallback_;

    std::atomic<bool> stop_requested_{false};
    std::atomic<bool> speaking_{false};
};
