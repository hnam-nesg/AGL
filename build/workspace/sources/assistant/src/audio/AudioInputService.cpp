#include "AudioInputService.h"

#include <algorithm>
#include <iostream>

AudioInputService::AudioInputService(const Config& cfg)
    : cfg_(cfg)
{
}

AudioInputService::~AudioInputService()
{
    stop();
}

bool AudioInputService::start(ChunkCallback cb)
{
    stop();

    callback_ = std::move(cb);
    if (!callback_) {
        std::cerr << "[AudioInputService] callback is empty\n";
        return false;
    }

    if (!openCapture()) {
        return false;
    }

    running_.store(true);
    thread_ = std::thread([this]() {
        this->threadMain();
    });

    return true;
}

void AudioInputService::stop()
{
    running_.store(false);

    if (thread_.joinable()) {
        thread_.join();
    }

    closeCapture();
}

void AudioInputService::threadMain()
{
    std::vector<int16_t> chunk;
    while (running_.load()) {
        if (!readChunk(chunk)) {
            continue;
        }
        if (callback_) {
            callback_(chunk);
        }
    }
}

bool AudioInputService::openCapture()
{
    closeCapture();

    int rc = snd_pcm_open(&pcm_handle_, cfg_.alsa_device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        std::cerr << "[ALSA] snd_pcm_open failed: " << snd_strerror(rc) << "\n";
        return false;
    }

    snd_pcm_hw_params_t* params = nullptr;
    snd_pcm_hw_params_malloc(&params);

    rc = snd_pcm_hw_params_any(pcm_handle_, params);
    if (rc < 0) {
        std::cerr << "[ALSA] snd_pcm_hw_params_any failed: " << snd_strerror(rc) << "\n";
        snd_pcm_hw_params_free(params);
        closeCapture();
        return false;
    }

    rc = snd_pcm_hw_params_set_access(pcm_handle_, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        std::cerr << "[ALSA] set_access failed: " << snd_strerror(rc) << "\n";
        snd_pcm_hw_params_free(params);
        closeCapture();
        return false;
    }

    rc = snd_pcm_hw_params_set_format(pcm_handle_, params, cfg_.format);
    if (rc < 0) {
        std::cerr << "[ALSA] set_format failed: " << snd_strerror(rc) << "\n";
        snd_pcm_hw_params_free(params);
        closeCapture();
        return false;
    }

    rc = snd_pcm_hw_params_set_channels(pcm_handle_, params, cfg_.channels);
    if (rc < 0) {
        std::cerr << "[ALSA] set_channels failed: " << snd_strerror(rc) << "\n";
        snd_pcm_hw_params_free(params);
        closeCapture();
        return false;
    }

    unsigned int rate = cfg_.sample_rate;
    int dir = 0;
    rc = snd_pcm_hw_params_set_rate_near(pcm_handle_, params, &rate, &dir);
    if (rc < 0) {
        std::cerr << "[ALSA] set_rate_near failed: " << snd_strerror(rc) << "\n";
        snd_pcm_hw_params_free(params);
        closeCapture();
        return false;
    }

    snd_pcm_uframes_t period = static_cast<snd_pcm_uframes_t>(cfg_.chunk_samples);
    rc = snd_pcm_hw_params_set_period_size_near(pcm_handle_, params, &period, &dir);
    if (rc < 0) {
        std::cerr << "[ALSA] set_period_size_near failed: " << snd_strerror(rc) << "\n";
        snd_pcm_hw_params_free(params);
        closeCapture();
        return false;
    }

    snd_pcm_uframes_t bufferSize =
        period * static_cast<snd_pcm_uframes_t>(std::max(2, cfg_.alsa_buffer_periods));
    rc = snd_pcm_hw_params_set_buffer_size_near(pcm_handle_, params, &bufferSize);
    if (rc < 0) {
        std::cerr << "[ALSA] set_buffer_size_near failed: " << snd_strerror(rc) << "\n";
        snd_pcm_hw_params_free(params);
        closeCapture();
        return false;
    }

    rc = snd_pcm_hw_params(pcm_handle_, params);
    if (rc < 0) {
        std::cerr << "[ALSA] snd_pcm_hw_params failed: " << snd_strerror(rc) << "\n";
        snd_pcm_hw_params_free(params);
        closeCapture();
        return false;
    }

    snd_pcm_hw_params_free(params);

    rc = snd_pcm_prepare(pcm_handle_);
    if (rc < 0) {
        std::cerr << "[ALSA] snd_pcm_prepare failed: " << snd_strerror(rc) << "\n";
        closeCapture();
        return false;
    }

    rc = snd_pcm_start(pcm_handle_);
    if (rc < 0) {
        std::cerr << "[ALSA] snd_pcm_start failed: " << snd_strerror(rc) << "\n";
    } else {
        std::cerr << "[ALSA] capture started\n";
    }

    return true;
}

void AudioInputService::closeCapture()
{
    if (pcm_handle_) {
        snd_pcm_close(pcm_handle_);
        pcm_handle_ = nullptr;
    }
}

bool AudioInputService::readChunk(std::vector<int16_t>& out_samples)
{
    out_samples.assign(cfg_.chunk_samples, 0);

    int frames = snd_pcm_readi(
        pcm_handle_,
        out_samples.data(),
        static_cast<snd_pcm_uframes_t>(cfg_.chunk_samples)
    );

    if (frames < 0) {
        const int recovered = snd_pcm_recover(pcm_handle_, frames, 1);
        if (recovered < 0) {
            std::cerr << "[ALSA] read failed and recover failed: "
                      << snd_strerror(recovered) << "\n";
        } else if (cfg_.debug) {
            std::cerr << "[ALSA] recovered from read error: "
                      << snd_strerror(frames) << "\n";
        }
        return false;
    }

    if (frames == 0) {
        return false;
    }

    if (frames != static_cast<int>(cfg_.chunk_samples)) {
        out_samples.resize(static_cast<size_t>(frames));
    }

    return true;
}