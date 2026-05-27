#include "PiperTTS.h"

// Legacy standalone TTS adapter kept as the selectable "piper" backend.

#include <alsa/asoundlib.h>

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

namespace fs = std::filesystem;

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

struct SpeakingGuard {
    std::atomic<bool>* speaking = nullptr;
    std::atomic<bool>* stopRequested = nullptr;
    std::function<void(double)>* audioCallback = nullptr;
    std::function<void(bool)>* playbackCallback = nullptr;

    ~SpeakingGuard()
    {
        if (audioCallback && *audioCallback) {
            (*audioCallback)(0.0);
        }
        if (playbackCallback && *playbackCallback) {
            (*playbackCallback)(false);
        }
        if (speaking) {
            speaking->store(false);
        }
        if (stopRequested) {
            stopRequested->store(false);
        }
    }
};

std::string trimChunk(std::string text)
{
    while (!text.empty() &&
           std::isspace(static_cast<unsigned char>(text.front()))) {
        text.erase(text.begin());
    }
    while (!text.empty() &&
           std::isspace(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }
    return text;
}

bool isSentenceBoundary(char c)
{
    return c == '.' || c == '?' || c == '!';
}

bool isSoftBoundary(char c)
{
    return c == ',' || c == ';' || c == ':';
}

bool isClosingPunctuation(char c)
{
    return c == '"' || c == '\'' || c == ')' || c == ']';
}

bool isBoundaryFollowedBySpaceOrEnd(const std::string& text, size_t index)
{
    size_t next = index + 1;
    while (next < text.size() && isClosingPunctuation(text[next])) {
        ++next;
    }
    return next >= text.size() ||
           std::isspace(static_cast<unsigned char>(text[next])) != 0;
}

size_t naturalBoundaryEnd(const std::string& text, size_t index)
{
    size_t end = index + 1;
    while (end < text.size()) {
        const char c = text[end];
        if (isSentenceBoundary(c) || isClosingPunctuation(c)) {
            ++end;
            continue;
        }
        break;
    }
    return end;
}

bool naturalBoundaryAt(const std::string& text, size_t index, bool* hard)
{
    const char c = text[index];
    if (c == '\n') {
        if (hard) *hard = true;
        return true;
    }

    if (!isSentenceBoundary(c) && !isSoftBoundary(c)) {
        return false;
    }

    if (!isBoundaryFollowedBySpaceOrEnd(text, index)) {
        return false;
    }

    if (hard) *hard = isSentenceBoundary(c);
    return true;
}

size_t findSplitBefore(const std::string& text, size_t limit, size_t minPos)
{
    const size_t capped = std::min(limit, text.size());
    for (size_t i = capped; i > minPos; --i) {
        if (isSentenceBoundary(text[i - 1]) ||
            isSoftBoundary(text[i - 1]) ||
            std::isspace(static_cast<unsigned char>(text[i - 1])) != 0) {
            return i;
        }
    }
    return capped;
}

size_t findNaturalSplit(const std::string& text,
                        size_t pos,
                        size_t naturalMinChars,
                        size_t targetChars,
                        size_t maxChars)
{
    const size_t remaining = text.size() - pos;
    const size_t maxEnd = std::min(text.size(), pos + maxChars);
    const bool tailFits = remaining <= maxChars;
    size_t softCandidate = std::string::npos;

    for (size_t i = pos; i < maxEnd; ++i) {
        bool hard = false;
        if (!naturalBoundaryAt(text, i, &hard)) {
            continue;
        }

        const size_t end = std::min(naturalBoundaryEnd(text, i), maxEnd);
        const std::string candidate = trimChunk(text.substr(pos, end - pos));
        if (candidate.empty()) {
            continue;
        }

        const size_t len = candidate.size();
        const size_t restLen = trimChunk(text.substr(end)).size();

        if (hard) {
            if (len >= 8 || restLen == 0) {
                return end;
            }
            continue;
        }

        if (len >= naturalMinChars && restLen >= 8) {
            if (softCandidate == std::string::npos) {
                softCandidate = end;
            }
            if (len >= std::max<size_t>(naturalMinChars, targetChars / 2)) {
                return end;
            }
        }
    }

    if (softCandidate != std::string::npos && tailFits) {
        return softCandidate;
    }

    if (tailFits) {
        return text.size();
    }

    if (softCandidate != std::string::npos) {
        return softCandidate;
    }

    size_t split = findSplitBefore(text.substr(pos, maxChars),
                                   targetChars,
                                   naturalMinChars);
    if (split == 0) {
        split = std::min(maxChars, remaining);
    }
    return pos + split;
}

std::vector<std::string> splitTextForPacing(const std::string& text)
{
    constexpr size_t naturalMinChars = 18;
    constexpr size_t targetChars = 110;
    constexpr size_t maxChars = 180;

    std::vector<std::string> chunks;
    chunks.reserve(std::max<size_t>(1, text.size() / targetChars + 1));

    for (size_t pos = 0; pos < text.size();) {
        while (pos < text.size() &&
               std::isspace(static_cast<unsigned char>(text[pos])) != 0) {
            ++pos;
        }
        if (pos >= text.size()) {
            break;
        }

        const size_t splitEnd = findNaturalSplit(text,
                                                 pos,
                                                 naturalMinChars,
                                                 targetChars,
                                                 maxChars);
        const size_t split = std::max<size_t>(1, splitEnd - pos);
        std::string chunk = trimChunk(text.substr(pos, split));
        if (!chunk.empty()) {
            chunks.push_back(std::move(chunk));
        }
        pos += split;
    }

    if (chunks.empty()) {
        const std::string chunk = trimChunk(text);
        if (!chunk.empty()) {
            chunks.push_back(chunk);
        }
    }

    return chunks;
}

char lastNonSpaceChar(const std::string& text)
{
    for (size_t i = text.size(); i > 0; --i) {
        const char c = text[i - 1];
        if (std::isspace(static_cast<unsigned char>(c)) == 0) {
            return c;
        }
    }
    return '\0';
}

int pauseMsAfterText(const std::string& text)
{
    switch (lastNonSpaceChar(text)) {
    case '?':
    case '!':
        return 460;
    case '.':
        return 380;
    case ';':
    case ':':
        return 300;
    case ',':
        return 190;
    default:
        return 120;
    }
}

size_t msToInterleavedSamples(int ms, uint32_t sampleRate, uint16_t channels)
{
    const size_t frames = static_cast<size_t>(
        (static_cast<uint64_t>(std::max(0, ms)) *
         static_cast<uint64_t>(std::max<uint32_t>(1, sampleRate))) / 1000u);
    return frames * std::max<uint16_t>(1, channels);
}
} // namespace

PiperTTS::PiperTTS(const Config& cfg)
    : cfg_(cfg)
{
}

PiperTTS::~PiperTTS()
{
    stop();
}

bool PiperTTS::isReady() const
{
    if (!fs::exists(cfg_.piper_path)) {
        std::cerr << "[PiperTTS] Piper binary not found: "
                  << cfg_.piper_path << "\n";
        return false;
    }

    if (!fs::exists(cfg_.model_path)) {
        std::cerr << "[PiperTTS] model not found: "
                  << cfg_.model_path << "\n";
        return false;
    }

    const std::string configPath = cfg_.config_path.empty()
        ? cfg_.model_path + ".json"
        : cfg_.config_path;
    if (!fs::exists(configPath)) {
        std::cerr << "[PiperTTS] model config not found: "
                  << configPath << "\n";
        return false;
    }

    return true;
}

bool PiperTTS::isSpeaking() const
{
    return speaking_.load();
}

void PiperTTS::setAudioLevelCallback(std::function<void(double)> callback)
{
    audioLevelCallback_ = std::move(callback);
}

void PiperTTS::setPlaybackStateCallback(std::function<void(bool)> callback)
{
    playbackStateCallback_ = std::move(callback);
}

void PiperTTS::setPlaybackProgressCallback(std::function<void(double)> callback)
{
    playbackProgressCallback_ = std::move(callback);
}

void PiperTTS::setRuntimeOptions(float playbackSpeed,
                                 float playbackVolume,
                                 const std::string& /*voice*/)
{
    std::lock_guard<std::mutex> lock(process_mutex_);
    cfg_.playback_speed = std::clamp(playbackSpeed, 0.75f, 1.5f);
    cfg_.playback_volume = std::clamp(playbackVolume, 0.0f, 1.0f);
    cfg_.length_scale = 1.0f / std::max(0.1f, cfg_.playback_speed);
}

std::string PiperTTS::shellQuote(const std::string& input)
{
    std::string out = "'";
    out.reserve(input.size() + 16);

    for (char c : input) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }

    out += "'";
    return out;
}

std::string PiperTTS::shellQuoteEnvValue(const std::string& input)
{
    std::string out = "\"";
    out.reserve(input.size() + 16);

    for (size_t i = 0; i < input.size(); ++i) {
        const char c = input[i];
        const bool keepVariableExpansion =
            c == '$' &&
            input.compare(i, std::string("${LD_LIBRARY_PATH}").size(), "${LD_LIBRARY_PATH}") == 0;
        if (!keepVariableExpansion && (c == '"' || c == '\\' || c == '$' || c == '`')) {
            out += '\\';
        }
        out += c;
    }

    out += '"';
    return out;
}

void PiperTTS::stop()
{
    const bool wasSpeaking = speaking_.load();
    stop_requested_.store(true);

    int pid = -1;
    {
        std::lock_guard<std::mutex> lock(process_mutex_);
        pid = piper_pid_;
    }

    if (pid > 0) {
        ::kill(pid, SIGTERM);
    }

    if (audioLevelCallback_) {
        audioLevelCallback_(0.0);
    }

    if (wasSpeaking && playbackStateCallback_) {
        playbackStateCallback_(false);
    }
}

void PiperTTS::clearRuntimeState()
{
    std::lock_guard<std::mutex> lock(process_mutex_);
    piper_pid_ = -1;
}

bool PiperTTS::synthesizeToFile(const std::string& text, const std::string& wav_path)
{
    if (text.empty()) {
        std::cerr << "[PiperTTS] Empty text\n";
        return false;
    }

    if (!isReady()) {
        return false;
    }

    fs::remove(wav_path);

    std::ostringstream cmd;

    const std::string configPath = cfg_.config_path.empty()
        ? cfg_.model_path + ".json"
        : cfg_.config_path;
    const std::string libraryPath = shellQuoteEnvValue(
        "/opt/piper:/usr/lib:/tmp/lib:${LD_LIBRARY_PATH}");

    cmd << "printf '%s\\n' " << shellQuote(text)
        << " | "
        << "PATH=/opt/piper:/usr/bin:/bin "
        << "LD_LIBRARY_PATH=" << libraryPath << " "
        << "ESPEAK_DATA_PATH=/opt/piper/espeak-ng-data "
        << "PHONEMIZER_ESPEAK_LIBRARY=/opt/piper/libespeak-ng.so "
        << shellQuote(cfg_.piper_path)
        << " --model " << shellQuote(cfg_.model_path)
        << " --config " << shellQuote(configPath)
        << " --output_file " << shellQuote(wav_path)
        << " --speaker " << cfg_.speaker_id
        << " --noise_scale " << cfg_.noise_scale
        << " --length_scale " << cfg_.length_scale
        << " --noise_w " << cfg_.noise_w;

    if (!cfg_.espeak_data_path.empty()) {
        cmd << " --espeak_data " << shellQuote(cfg_.espeak_data_path);
    }
    if (!cfg_.tashkeel_model_path.empty() && fs::exists(cfg_.tashkeel_model_path)) {
        cmd << " --tashkeel_model " << shellQuote(cfg_.tashkeel_model_path);
    }

    if (cfg_.quiet) {
        cmd << " >/tmp/piper_tts_last.log 2>&1";
    }

    std::cout << "[PiperTTS] Synthesizing: " << text << "\n";

    const int rc = std::system(cmd.str().c_str());
    if (rc != 0) {
        std::cerr << "[PiperTTS] Synthesize failed, rc=" << rc
                  << ". See /tmp/piper_tts_last.log\n";
        return false;
    }

    if (!fs::exists(wav_path)) {
        std::cerr << "[PiperTTS] Output wav not created: " << wav_path << "\n";
        return false;
    }

    return true;
}

bool PiperTTS::loadWavPcm16(const std::string& path, WavInfo* out)
{
    if (!out) {
        return false;
    }

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cerr << "[PiperTTS] Cannot open wav: " << path << "\n";
        return false;
    }

    BasicWavHeader hdr{};
    in.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    if (!in) {
        std::cerr << "[PiperTTS] Failed to read wav header\n";
        return false;
    }

    if (std::string(hdr.riff, 4) != "RIFF" ||
        std::string(hdr.wave, 4) != "WAVE") {
        std::cerr << "[PiperTTS] Not a valid WAV file\n";
        return false;
    }

    if (hdr.audioFormat != 1 || hdr.bitsPerSample != 16) {
        std::cerr << "[PiperTTS] Unsupported WAV format. audioFormat="
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
        std::cerr << "[PiperTTS] WAV data chunk not found\n";
        return false;
    }

    out->sampleRate = hdr.sampleRate;
    out->channels = hdr.numChannels;
    out->bitsPerSample = hdr.bitsPerSample;
    out->samples.resize(dataSize / sizeof(int16_t));

    in.read(reinterpret_cast<char*>(out->samples.data()), dataSize);
    if (!in) {
        std::cerr << "[PiperTTS] Failed to read WAV PCM data\n";
        return false;
    }

    return true;
}

bool PiperTTS::saveWavPcm16(const std::string& path, const WavInfo& wav)
{
    if (wav.sampleRate == 0 || wav.channels == 0 || wav.bitsPerSample != 16) {
        std::cerr << "[PiperTTS] Cannot save invalid WAV metadata\n";
        return false;
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cerr << "[PiperTTS] Cannot open WAV for write: "
                  << path
                  << "\n";
        return false;
    }

    const uint32_t dataBytes =
        static_cast<uint32_t>(wav.samples.size() * sizeof(int16_t));
    const uint32_t riffChunkSize = 36 + dataBytes;
    const uint32_t fmtChunkSize = 16;
    const uint16_t audioFormat = 1;
    const uint16_t channels = wav.channels;
    const uint32_t sampleRate = wav.sampleRate;
    const uint16_t bitsPerSample = 16;
    const uint16_t blockAlign = channels * bitsPerSample / 8;
    const uint32_t byteRate = sampleRate * blockAlign;

    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char*>(&riffChunkSize), 4);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    out.write(reinterpret_cast<const char*>(&fmtChunkSize), 4);
    out.write(reinterpret_cast<const char*>(&audioFormat), 2);
    out.write(reinterpret_cast<const char*>(&channels), 2);
    out.write(reinterpret_cast<const char*>(&sampleRate), 4);
    out.write(reinterpret_cast<const char*>(&byteRate), 4);
    out.write(reinterpret_cast<const char*>(&blockAlign), 2);
    out.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    out.write("data", 4);
    out.write(reinterpret_cast<const char*>(&dataBytes), 4);

    if (!wav.samples.empty()) {
        out.write(reinterpret_cast<const char*>(wav.samples.data()),
                  static_cast<std::streamsize>(dataBytes));
    }

    if (!out) {
        std::cerr << "[PiperTTS] Failed to write WAV: "
                  << path
                  << "\n";
        return false;
    }

    return true;
}

PiperTTS::AudioLevelInfo PiperTTS::analyzeAudioLevel(const int16_t* data, size_t sampleCount)
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

bool PiperTTS::playWavWithAlsaAndMeter(const std::string& wavPath)
{
    WavInfo wav;

    if (!loadWavPcm16(wavPath, &wav)) {
        return false;
    }

    if (wav.samples.empty()) {
        std::cerr << "[PiperTTS] Empty PCM data\n";
        return false;
    }

    const float volume = std::clamp(cfg_.playback_volume, 0.0f, 1.0f);
    if (std::abs(volume - 1.0f) > 0.001f) {
        for (int16_t& sample : wav.samples) {
            const long scaled = std::lround(static_cast<float>(sample) * volume);
            sample = static_cast<int16_t>(std::clamp(scaled, -32768l, 32767l));
        }
    }

    std::cout << "[PiperTTS] wav sr=" << wav.sampleRate
              << " ch=" << wav.channels
              << " bits=" << wav.bitsPerSample
              << " samples=" << wav.samples.size()
              << "\n";

    snd_pcm_t* handle = nullptr;
    int rc = snd_pcm_open(
        &handle,
        cfg_.alsa_playback_device.c_str(),
        SND_PCM_STREAM_PLAYBACK,
        0
    );

    if (rc < 0) {
        std::cerr << "[PiperTTS] snd_pcm_open failed: "
                  << snd_strerror(rc) << "\n";
        return false;
    }

    snd_pcm_hw_params_t* params = nullptr;
    snd_pcm_hw_params_malloc(&params);

    bool ok = true;

    do {
        rc = snd_pcm_hw_params_any(handle, params);
        if (rc < 0) {
            std::cerr << "[PiperTTS] hw_params_any failed: "
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
            std::cerr << "[PiperTTS] set_access failed: "
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
            std::cerr << "[PiperTTS] set_format failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        rc = snd_pcm_hw_params_set_channels(handle, params, wav.channels);
        if (rc < 0) {
            std::cerr << "[PiperTTS] set_channels failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        unsigned int rate = wav.sampleRate;
        int dir = 0;

        rc = snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);
        if (rc < 0) {
            std::cerr << "[PiperTTS] set_rate_near failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        snd_pcm_uframes_t period = static_cast<snd_pcm_uframes_t>(
            std::max(1u, cfg_.playback_chunk_frames)
        );

        rc = snd_pcm_hw_params_set_period_size_near(handle, params, &period, &dir);
        if (rc < 0) {
            std::cerr << "[PiperTTS] set_period_size_near failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        snd_pcm_uframes_t bufferSize = period * 4;
        rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &bufferSize);
        if (rc < 0) {
            std::cerr << "[PiperTTS] set_buffer_size_near failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        rc = snd_pcm_hw_params(handle, params);
        if (rc < 0) {
            std::cerr << "[PiperTTS] snd_pcm_hw_params failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        rc = snd_pcm_prepare(handle);
        if (rc < 0) {
            std::cerr << "[PiperTTS] snd_pcm_prepare failed: "
                      << snd_strerror(rc) << "\n";
            ok = false;
            break;
        }

        const size_t channels = std::max<uint16_t>(1, wav.channels);
        const size_t chunkFrames = std::max<unsigned int>(1u, cfg_.playback_chunk_frames);
        const size_t chunkSamples = chunkFrames * channels;

        for (size_t pos = 0; pos < wav.samples.size(); pos += chunkSamples) {
            if (stop_requested_.load()) {
                snd_pcm_drop(handle);
                break;
            }

            const size_t samplesThisChunk = std::min(
                chunkSamples,
                wav.samples.size() - pos
            );

            const size_t framesThisChunk = samplesThisChunk / channels;
            if (framesThisChunk == 0) {
                continue;
            }

            const AudioLevelInfo info = analyzeAudioLevel(
                wav.samples.data() + pos,
                framesThisChunk * channels
            );

            if (audioLevelCallback_) {
                audioLevelCallback_(std::clamp(info.rmsNormalized, 0.0, 1.0));
            }
            if (playbackProgressCallback_) {
                playbackProgressCallback_(
                    std::clamp(static_cast<double>(pos) /
                               static_cast<double>(std::max<size_t>(1, wav.samples.size())),
                               0.0,
                               1.0));
            }

            const int16_t* ptr = wav.samples.data() + pos;
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
                        std::cerr << "[PiperTTS] snd_pcm_writei failed: "
                                  << snd_strerror(static_cast<int>(written))
                                  << "\n";
                        ok = false;
                        break;
                    }
                    continue;
                }

                ptr += written * channels;
                framesRemaining -= static_cast<size_t>(written);
            }

            if (!ok) {
                break;
            }
        }

        if (!stop_requested_.load() && ok) {
            rc = snd_pcm_drain(handle);
            if (rc < 0) {
                std::cerr << "[PiperTTS] snd_pcm_drain failed: "
                          << snd_strerror(rc)
                          << "\n";
                ok = false;
            }
        }

        if (!stop_requested_.load() && ok && playbackProgressCallback_) {
            playbackProgressCallback_(1.0);
        }
    } while (false);

    snd_pcm_hw_params_free(params);
    snd_pcm_close(handle);

    if (audioLevelCallback_) {
        audioLevelCallback_(0.0);
    }

    return ok;
}

bool PiperTTS::speak(const std::string& text)
{
    if (text.empty()) {
        std::cerr << "[PiperTTS] Empty text\n";
        return false;
    }

    if (!isReady()) {
        return false;
    }

    stop();

    stop_requested_.store(false);
    speaking_.store(true);

    if (playbackStateCallback_) {
        playbackStateCallback_(true);
    }

    SpeakingGuard guard{
        &speaking_,
        &stop_requested_,
        &audioLevelCallback_,
        &playbackStateCallback_
    };

    const std::string wavPath = cfg_.output_wav_path.empty()
        ? "/tmp/piper_tts.wav"
        : cfg_.output_wav_path;

    const std::vector<std::string> chunks = splitTextForPacing(text);
    if (chunks.empty()) {
        std::cerr << "[PiperTTS] Empty text after chunking\n";
        return false;
    }

    std::cout << "[PiperTTS] synthesis chunks=" << chunks.size() << "\n";

    if (chunks.size() == 1) {
        if (!synthesizeToFile(chunks.front(), wavPath)) {
            return false;
        }

        if (stop_requested_.load()) {
            return false;
        }

        return playWavWithAlsaAndMeter(wavPath);
    }

    WavInfo combined;
    combined.bitsPerSample = 16;

    for (size_t i = 0; i < chunks.size(); ++i) {
        if (stop_requested_.load()) {
            return false;
        }

        const std::string chunkPath =
            wavPath + ".chunk" + std::to_string(i + 1) + ".wav";

        if (!synthesizeToFile(chunks[i], chunkPath)) {
            return false;
        }

        if (stop_requested_.load()) {
            return false;
        }

        WavInfo part;
        if (!loadWavPcm16(chunkPath, &part)) {
            return false;
        }
        fs::remove(chunkPath);

        if (part.samples.empty()) {
            std::cerr << "[PiperTTS] Empty PCM chunk "
                      << (i + 1)
                      << "/"
                      << chunks.size()
                      << "\n";
            return false;
        }

        if (combined.samples.empty()) {
            combined.sampleRate = part.sampleRate;
            combined.channels = part.channels;
            combined.bitsPerSample = part.bitsPerSample;
        } else if (part.sampleRate != combined.sampleRate ||
                   part.channels != combined.channels ||
                   part.bitsPerSample != combined.bitsPerSample) {
            std::cerr << "[PiperTTS] Chunk WAV format mismatch\n";
            return false;
        }

        const int pauseMs = i > 0 ? pauseMsAfterText(chunks[i - 1]) : 0;
        const size_t silenceSamples = msToInterleavedSamples(
            pauseMs,
            combined.sampleRate,
            combined.channels);
        if (!combined.samples.empty() && silenceSamples > 0) {
            combined.samples.insert(combined.samples.end(), silenceSamples, 0);
        }

        combined.samples.insert(combined.samples.end(),
                                part.samples.begin(),
                                part.samples.end());
    }

    if (combined.samples.empty()) {
        std::cerr << "[PiperTTS] Empty combined PCM\n";
        return false;
    }

    if (!saveWavPcm16(wavPath, combined)) {
        return false;
    }

    if (stop_requested_.load()) {
        return false;
    }

    return playWavWithAlsaAndMeter(wavPath);
}
