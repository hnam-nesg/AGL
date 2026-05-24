#pragma once

#include <alsa/asoundlib.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>

class AudioInputService {
public:
    struct Config {
        std::string alsa_device = "plughw:2,0";
        unsigned int sample_rate = 16000;
        int channels = 1;
        snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
        size_t chunk_samples = 1280;   // 80 ms @ 16 kHz
        int alsa_buffer_periods = 4;
        bool debug = false;
    };

    using ChunkCallback = std::function<void(const std::vector<int16_t>&)>;

public:
    explicit AudioInputService(const Config& cfg);
    ~AudioInputService();

    bool start(ChunkCallback cb);
    void stop();

    bool isRunning() const { return running_.load(); }

private:
    bool openCapture();
    void closeCapture();
    bool readChunk(std::vector<int16_t>& out_samples);
    void threadMain();

private:
    Config cfg_;
    ChunkCallback callback_;

    snd_pcm_t* pcm_handle_ = nullptr;
    std::thread thread_;
    std::atomic<bool> running_{false};
};