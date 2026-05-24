#include "VieNeuTtsWorker.h"

#include <alsa/asoundlib.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <utility>

namespace {
#pragma pack(push, 1)
struct BasicWavHeader {
    char riff[4];
    uint32_t chunkSize;
    char wave[4];
    char fmt[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};
#pragma pack(pop)

struct PlaybackGuard {
    std::atomic<bool>* playing = nullptr;
    std::function<void(double)>* audioCallback = nullptr;

    ~PlaybackGuard()
    {
        if (audioCallback && *audioCallback) {
            (*audioCallback)(0.0);
        }
        if (playing) {
            playing->store(false);
        }
    }
};
} // namespace

VieNeuTtsWorker::VieNeuTtsWorker(const Config& cfg)
    : cfg_(cfg)
{
}

VieNeuTtsWorker::~VieNeuTtsWorker()
{
    stop();
}

void VieNeuTtsWorker::setAudioLevelCallback(std::function<void(double)> callback)
{
    audioLevelCallback_ = std::move(callback);
}

void VieNeuTtsWorker::setPlaybackProgressCallback(std::function<void(double)> callback)
{
    playbackProgressCallback_ = std::move(callback);
}

void VieNeuTtsWorker::resetStopRequested()
{
    stop_requested_.store(false);
}

void VieNeuTtsWorker::stop()
{
    stop_requested_.store(true);

    if (audioLevelCallback_) {
        audioLevelCallback_(0.0);
    }
}

bool VieNeuTtsWorker::isPlaying() const
{
    return playing_.load();
}

bool VieNeuTtsWorker::loadWavPcm16(const std::string& path, WavInfo* out)
{
    if (!out) {
        return false;
    }

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cerr << "[VieNeuTTS] Cannot open wav: " << path << "\n";
        return false;
    }

    BasicWavHeader hdr{};
    in.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    if (!in) {
        std::cerr << "[VieNeuTTS] Failed to read wav header\n";
        return false;
    }

    if (std::string(hdr.riff, 4) != "RIFF" ||
        std::string(hdr.wave, 4) != "WAVE") {
        std::cerr << "[VieNeuTTS] Not a valid WAV file\n";
        return false;
    }

    if (hdr.audioFormat != 1 || hdr.bitsPerSample != 16) {
        std::cerr << "[VieNeuTTS] Unsupported WAV format. audioFormat="
                  << hdr.audioFormat
                  << " bits=" << hdr.bitsPerSample << "\n";
        return false;
    }

    bool foundData = false;
    uint32_t dataSize = 0;

    while (in) {
        char chunkId[4] = {};
        uint32_t chunkSize = 0;

        in.read(chunkId, 4);
        in.read(reinterpret_cast<char*>(&chunkSize), 4);
        if (!in) {
            break;
        }

        if (std::string(chunkId, 4) == "data") {
            dataSize = chunkSize;
            foundData = true;
            break;
        }

        in.seekg(chunkSize, std::ios::cur);
    }

    if (!foundData || dataSize == 0) {
        std::cerr << "[VieNeuTTS] WAV data chunk not found\n";
        return false;
    }

    out->sampleRate = hdr.sampleRate;
    out->channels = hdr.numChannels;
    out->bitsPerSample = hdr.bitsPerSample;
    out->samples.resize(dataSize / sizeof(int16_t));

    in.read(reinterpret_cast<char*>(out->samples.data()), dataSize);
    if (!in) {
        std::cerr << "[VieNeuTTS] Failed to read WAV PCM data\n";
        return false;
    }

    return true;
}

bool VieNeuTtsWorker::loadWavPcm16File(const std::string& path,
                                       std::vector<int16_t>* samples,
                                       uint32_t* sample_rate,
                                       uint16_t* channels)
{
    if (!samples) {
        return false;
    }

    WavInfo wav;
    if (!loadWavPcm16(path, &wav)) {
        return false;
    }

    *samples = std::move(wav.samples);
    if (sample_rate) {
        *sample_rate = wav.sampleRate;
    }
    if (channels) {
        *channels = wav.channels;
    }
    return true;
}

VieNeuTtsWorker::AudioLevelInfo VieNeuTtsWorker::analyzeAudioLevel(const int16_t* data,
                                                                   size_t sampleCount)
{
    AudioLevelInfo out;

    if (!data || sampleCount == 0) {
        return out;
    }

    long double sumSquares = 0.0;

    for (size_t i = 0; i < sampleCount; ++i) {
        const long double v = static_cast<long double>(data[i]);
        sumSquares += v * v;
    }

    out.rms = std::sqrt(sumSquares / sampleCount);
    out.rmsNormalized = out.rms / 32768.0;

    if (out.rmsNormalized > 0.0) {
        out.dbfs = 20.0 * std::log10(out.rmsNormalized);
    }

    return out;
}

bool VieNeuTtsWorker::playWavWithAlsaAndMeter(const std::string& wav_path)
{
    if (stop_requested_.load()) {
        return false;
    }

    WavInfo wav;
    if (!loadWavPcm16(wav_path, &wav)) {
        return false;
    }

    std::cout << "[VieNeuTTS] wav sr=" << wav.sampleRate
              << " ch=" << wav.channels
              << " bits=" << wav.bitsPerSample
              << " samples=" << wav.samples.size()
              << "\n";

    return playPcm16WithAlsaAndMeter(wav.samples, wav.sampleRate, wav.channels);
}

bool VieNeuTtsWorker::playPcm16WithAlsaAndMeter(const std::vector<int16_t>& samples,
                                                uint32_t sample_rate,
                                                uint16_t channels)
{
    if (stop_requested_.load()) {
        return false;
    }

    if (samples.empty()) {
        std::cerr << "[VieNeuTTS] Empty PCM data\n";
        return false;
    }

    const AudioLevelInfo fullInfo = analyzeAudioLevel(samples.data(), samples.size());
    std::cout << "[VieNeuTTS] playback samples="
              << samples.size()
              << " duration_ms="
              << (samples.size() * 1000u / std::max<uint32_t>(1, sample_rate))
              << " rms="
              << fullInfo.rms
              << " dbfs="
              << fullInfo.dbfs
              << "\n";

    playing_.store(true);
    PlaybackGuard guard{&playing_, &audioLevelCallback_};

    snd_pcm_t* handle = nullptr;
    int rc = snd_pcm_open(
        &handle,
        cfg_.alsa_playback_device.c_str(),
        SND_PCM_STREAM_PLAYBACK,
        0
    );

    if (rc < 0) {
        std::cerr << "[VieNeuTTS] snd_pcm_open failed: "
                  << snd_strerror(rc) << "\n";
        return false;
    }

    snd_pcm_hw_params_t* params = nullptr;
    rc = snd_pcm_hw_params_malloc(&params);
    if (rc < 0 || !params) {
        std::cerr << "[VieNeuTTS] snd_pcm_hw_params_malloc failed: "
                  << snd_strerror(rc) << "\n";
        snd_pcm_close(handle);
        return false;
    }

    bool ok = true;

    do {
        rc = snd_pcm_hw_params_any(handle, params);
        if (rc < 0) {
            std::cerr << "[VieNeuTTS] hw_params_any failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        rc = snd_pcm_hw_params_set_access(
            handle,
            params,
            SND_PCM_ACCESS_RW_INTERLEAVED
        );
        if (rc < 0) {
            std::cerr << "[VieNeuTTS] set_access failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        rc = snd_pcm_hw_params_set_format(
            handle,
            params,
            SND_PCM_FORMAT_S16_LE
        );
        if (rc < 0) {
            std::cerr << "[VieNeuTTS] set_format failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        rc = snd_pcm_hw_params_set_channels(handle, params, channels);
        if (rc < 0) {
            std::cerr << "[VieNeuTTS] set_channels failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        unsigned int rate = sample_rate;
        int dir = 0;

        rc = snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);
        if (rc < 0) {
            std::cerr << "[VieNeuTTS] set_rate_near failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }
        if (rate != sample_rate) {
            std::cout << "[VieNeuTTS] ALSA playback rate adjusted "
                      << sample_rate << " -> " << rate << "\n";
        }

        snd_pcm_uframes_t period = static_cast<snd_pcm_uframes_t>(
            std::max(1u, cfg_.playback_chunk_frames)
        );

        rc = snd_pcm_hw_params_set_period_size_near(handle, params, &period, &dir);
        if (rc < 0) {
            std::cerr << "[VieNeuTTS] set_period_size_near failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        snd_pcm_uframes_t bufferSize = period * 4;
        rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &bufferSize);
        if (rc < 0) {
            std::cerr << "[VieNeuTTS] set_buffer_size_near failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        rc = snd_pcm_hw_params(handle, params);
        if (rc < 0) {
            std::cerr << "[VieNeuTTS] snd_pcm_hw_params failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        rc = snd_pcm_prepare(handle);
        if (rc < 0) {
            std::cerr << "[VieNeuTTS] snd_pcm_prepare failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        const size_t channelCount = std::max<uint16_t>(1, channels);
        const size_t chunkFrames = std::max<unsigned int>(1u, cfg_.playback_chunk_frames);
        const size_t chunkSamples = chunkFrames * channelCount;

        for (size_t pos = 0; pos < samples.size(); pos += chunkSamples) {
            if (stop_requested_.load()) {
                snd_pcm_drop(handle);
                break;
            }

            const size_t samplesThisChunk = std::min(
                chunkSamples,
                samples.size() - pos
            );

            const size_t framesThisChunk = samplesThisChunk / channelCount;
            if (framesThisChunk == 0) {
                continue;
            }

            const AudioLevelInfo info = analyzeAudioLevel(
                samples.data() + pos,
                framesThisChunk * channelCount
            );

            if (audioLevelCallback_) {
                audioLevelCallback_(std::clamp(info.rmsNormalized, 0.0, 1.0));
            }
            if (playbackProgressCallback_) {
                playbackProgressCallback_(
                    std::clamp(static_cast<double>(pos) /
                               static_cast<double>(std::max<size_t>(1, samples.size())),
                               0.0,
                               1.0));
            }

            const int16_t* ptr = samples.data() + pos;
            size_t framesRemaining = framesThisChunk;

            while (framesRemaining > 0) {
                if (stop_requested_.load()) {
                    snd_pcm_drop(handle);
                    framesRemaining = 0;
                    break;
                }

                snd_pcm_sframes_t written = snd_pcm_writei(
                    handle,
                    ptr,
                    framesRemaining
                );

                if (written < 0) {
                    written = snd_pcm_recover(handle, static_cast<int>(written), 1);
                    if (written < 0) {
                        std::cerr << "[VieNeuTTS] snd_pcm_writei failed: "
                                  << snd_strerror(static_cast<int>(written))
                                  << "\n";
                        ok = false;
                        break;
                    }
                    continue;
                }

                ptr += written * channelCount;
                framesRemaining -= static_cast<size_t>(written);
            }

            if (!ok) {
                break;
            }
        }

        if (ok && playbackProgressCallback_) {
            playbackProgressCallback_(1.0);
        }

        if (!stop_requested_.load() && ok) {
            snd_pcm_drain(handle);
        }
    } while (false);

    snd_pcm_hw_params_free(params);
    snd_pcm_close(handle);

    return ok && !stop_requested_.load();
}
