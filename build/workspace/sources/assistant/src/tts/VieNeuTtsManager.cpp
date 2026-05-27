#include "VieNeuTtsManager.h"

#include <cstdint>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {
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
    auto scan = [&](auto predicate) {
        for (size_t i = capped; i > minPos; --i) {
            if (predicate(text[i - 1])) {
                return i;
            }
        }
        return std::string::npos;
    };

    size_t split = scan(isSentenceBoundary);
    if (split != std::string::npos) {
        return split;
    }

    split = scan(isSoftBoundary);
    if (split != std::string::npos) {
        return split;
    }

    split = scan([](char c) {
        return std::isspace(static_cast<unsigned char>(c)) != 0;
    });
    if (split != std::string::npos) {
        return split;
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
    if (split == std::string::npos || split == 0) {
        split = std::min(maxChars, remaining);
    }
    return pos + split;
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

uint64_t fnv1a64(const std::string& text)
{
    uint64_t hash = 14695981039346656037ull;
    for (unsigned char c : text) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ull;
    }
    return hash;
}

std::string hex64(uint64_t value)
{
    std::ostringstream out;
    out << std::hex << std::setw(16) << std::setfill('0') << value;
    return out.str();
}

size_t msToSamples(int ms, uint32_t sampleRate)
{
    return static_cast<size_t>(
        (static_cast<uint64_t>(std::max(0, ms)) *
         static_cast<uint64_t>(std::max<uint32_t>(1, sampleRate))) / 1000u);
}

} // namespace

VieNeuTtsManager::VieNeuTtsManager(const Config& cfg)
    : cfg_(cfg),
      engine_(cfg),
      worker_(cfg)
{
}

VieNeuTtsManager::~VieNeuTtsManager()
{
    stop();
}

bool VieNeuTtsManager::initialize()
{
    return engine_.initialize();
}

bool VieNeuTtsManager::isReady() const
{
    return engine_.isReady();
}

bool VieNeuTtsManager::isSpeaking() const
{
    return speaking_.load() || engine_.isSynthesizing() || worker_.isPlaying();
}

void VieNeuTtsManager::setAudioLevelCallback(std::function<void(double)> callback)
{
    audioLevelCallback_ = std::move(callback);
    worker_.setAudioLevelCallback(audioLevelCallback_);
}

void VieNeuTtsManager::setPlaybackStateCallback(std::function<void(bool)> callback)
{
    playbackStateCallback_ = std::move(callback);
}

void VieNeuTtsManager::setPlaybackProgressCallback(std::function<void(double)> callback)
{
    worker_.setPlaybackProgressCallback(std::move(callback));
}

void VieNeuTtsManager::setRuntimeOptions(float playbackSpeed,
                                         float playbackVolume,
                                         const std::string& voice)
{
    cfg_.playback_speed = std::clamp(playbackSpeed, 0.75f, 1.5f);
    cfg_.playback_volume = std::clamp(playbackVolume, 0.0f, 1.0f);

    if (!voice.empty() && voice != cfg_.voice) {
        if (engine_.setVoice(voice)) {
            cfg_.voice = voice;
        } else {
            std::cerr << "[VieNeuTTS] Runtime voice change failed, requested="
                      << voice
                      << "\n";
        }
    }
}

void VieNeuTtsManager::stop()
{
    const bool wasSpeaking = isSpeaking();
    stop_requested_.store(true);
    engine_.stop();
    worker_.stop();

    if (audioLevelCallback_) {
        audioLevelCallback_(0.0);
    }

    if (wasSpeaking && playbackStateCallback_) {
        playbackStateCallback_(false);
    }
}

bool VieNeuTtsManager::synthesizeToFile(const std::string& text,
                                        const std::string& wav_path)
{
    engine_.resetStopRequested();
    return engine_.synthesizeToFile(text, wav_path);
}

std::vector<std::string> VieNeuTtsManager::splitTextForSynthesis(const std::string& text) const
{
    size_t minChars = static_cast<size_t>(std::max(0, cfg_.min_chunk_chars));
    size_t targetChars = static_cast<size_t>(std::max(80, cfg_.target_chunk_chars));
    size_t maxChars = static_cast<size_t>(std::max(120, cfg_.max_chunk_chars));
    if (minChars >= targetChars) {
        minChars = targetChars / 2;
    }
    if (targetChars >= maxChars) {
        targetChars = std::max<size_t>(80, maxChars * 2 / 3);
    }

    const size_t naturalMinChars = std::min(
        minChars,
        std::max<size_t>(16, minChars / 2));

    std::vector<std::string> chunks;
    chunks.reserve(std::max<size_t>(1, text.size() / std::max<size_t>(1, targetChars) + 1));

    for (size_t pos = 0; pos < text.size();) {
        while (pos < text.size() &&
               std::isspace(static_cast<unsigned char>(text[pos]))) {
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

    if (chunks.size() > 1) {
        for (size_t i = 0; i < chunks.size();) {
            if (chunks[i].size() >= naturalMinChars) {
                ++i;
                continue;
            }

            if (i + 1 < chunks.size() &&
                chunks[i].size() + 1 + chunks[i + 1].size() <= maxChars) {
                chunks[i] += " ";
                chunks[i] += chunks[i + 1];
                chunks.erase(chunks.begin() + static_cast<std::ptrdiff_t>(i + 1));
                continue;
            }

            if (i > 0 && chunks[i - 1].size() + 1 + chunks[i].size() <= maxChars) {
                chunks[i - 1] += " ";
                chunks[i - 1] += chunks[i];
                chunks.erase(chunks.begin() + static_cast<std::ptrdiff_t>(i));
                continue;
            }

            if (i + 1 < chunks.size()) {
                const size_t room = maxChars > chunks[i].size()
                    ? maxChars - chunks[i].size() - 1
                    : 0;
                if (room > 0) {
                    const size_t take = findSplitBefore(chunks[i + 1], room, 0);
                    if (take != std::string::npos && take > 0 && take < chunks[i + 1].size()) {
                        const std::string borrowed = trimChunk(chunks[i + 1].substr(0, take));
                        const std::string rest = trimChunk(chunks[i + 1].substr(take));
                        if (!borrowed.empty() && !rest.empty()) {
                            chunks[i] += " ";
                            chunks[i] += borrowed;
                            chunks[i + 1] = rest;
                            ++i;
                            continue;
                        }
                    }
                }
            }
            ++i;
        }
    }

    for (const std::string& chunk : chunks) {
        if (chunk.size() > maxChars) {
            std::vector<std::string> fixed;
            for (const std::string& existing : chunks) {
                if (existing.size() <= maxChars) {
                    fixed.push_back(existing);
                    continue;
                }
                size_t start = 0;
                while (start < existing.size()) {
                    const size_t split = findSplitBefore(existing.substr(start), maxChars, 0);
                    std::string part = trimChunk(existing.substr(start, split));
                    if (!part.empty()) {
                        fixed.push_back(std::move(part));
                    }
                    start += std::max<size_t>(1, split);
                }
            }
            chunks = std::move(fixed);
            break;
        }
    }

    std::cout << "[VieNeuTTS] chunk plan min="
              << minChars
              << " natural_min="
              << naturalMinChars
              << " target="
              << targetChars
              << " max="
              << maxChars;
    for (size_t i = 0; i < chunks.size(); ++i) {
        std::cout << (i == 0 ? " sizes=" : ",")
                  << chunks[i].size();
    }
    std::cout << "\n";

    return chunks;
}

std::string VieNeuTtsManager::cacheFileNameForChunk(const std::string& text) const
{
    if (text.empty()) {
        return {};
    }

    std::ostringstream material;
    material << "vieneu-cache-v7\n"
             << cfg_.model_path << "\n"
             << cfg_.codec_path << "\n"
             << cfg_.voices_path << "\n"
             << cfg_.voice << "\n"
             << cfg_.prompt_mode << "\n"
             << cfg_.phoneme_dict_path << "\n"
             << cfg_.allow_raw_phoneme_fallback << "\n"
             << cfg_.min_chunk_chars << "\n"
             << cfg_.target_chunk_chars << "\n"
             << cfg_.max_chunk_chars << "\n"
             << cfg_.top_k << "\n"
             << static_cast<int>(std::lround(cfg_.top_p * 1000.0f)) << "\n"
             << static_cast<int>(std::lround(cfg_.min_p * 1000.0f)) << "\n"
             << static_cast<int>(std::lround(cfg_.temperature * 1000.0f)) << "\n"
             << static_cast<int>(std::lround(cfg_.playback_speed * 100.0f)) << "\n"
             << cfg_.trim_audio_edges << "\n"
             << cfg_.silence_trim_threshold << "\n"
             << cfg_.silence_keep_ms << "\n"
             << cfg_.edge_fade_ms << "\n"
             << text;

    return hex64(fnv1a64(material.str())) + ".wav";
}

std::string VieNeuTtsManager::presetPathForChunk(const std::string& text) const
{
    if (!cfg_.enable_audio_cache || cfg_.audio_preset_dir.empty()) {
        return {};
    }

    const std::string fileName = cacheFileNameForChunk(text);
    if (fileName.empty()) {
        return {};
    }

    fs::path path(cfg_.audio_preset_dir);
    path /= fileName;
    return path.string();
}

std::string VieNeuTtsManager::cachePathForChunk(const std::string& text) const
{
    if (!cfg_.enable_audio_cache || cfg_.audio_cache_dir.empty()) {
        return {};
    }

    const std::string fileName = cacheFileNameForChunk(text);
    if (fileName.empty()) {
        return {};
    }

    fs::path path(cfg_.audio_cache_dir);
    path /= fileName;
    return path.string();
}

std::vector<int16_t> VieNeuTtsManager::prepareForPlayback(const std::vector<int16_t>& pcm,
                                                          uint32_t sample_rate) const
{
    if (pcm.empty()) {
        return {};
    }

    std::vector<int16_t> out = pcm;

    if (cfg_.trim_audio_edges && out.size() > 2) {
        const int threshold = std::max(0, cfg_.silence_trim_threshold);
        const size_t keep = msToSamples(cfg_.silence_keep_ms, sample_rate);

        size_t begin = 0;
        while (begin < out.size() &&
               std::abs(static_cast<int>(out[begin])) <= threshold) {
            ++begin;
        }

        size_t end = out.size();
        while (end > begin &&
               std::abs(static_cast<int>(out[end - 1])) <= threshold) {
            --end;
        }

        if (begin < end) {
            begin = begin > keep ? begin - keep : 0;
            end = std::min(out.size(), end + keep);
            if (begin > 0 || end < out.size()) {
                out = std::vector<int16_t>(
                    out.begin() + static_cast<std::ptrdiff_t>(begin),
                    out.begin() + static_cast<std::ptrdiff_t>(end));
            }
        }
    }

    const float speed = std::clamp(cfg_.playback_speed, 0.75f, 1.5f);
    if (std::abs(speed - 1.0f) > 0.02f && out.size() > 2) {
        const size_t targetSize = std::max<size_t>(
            1,
            static_cast<size_t>(static_cast<double>(out.size()) / speed));
        std::vector<int16_t> scaled;
        scaled.resize(targetSize);

        for (size_t i = 0; i < targetSize; ++i) {
            const double source = static_cast<double>(i) * speed;
            const size_t left = std::min(
                static_cast<size_t>(source),
                out.size() - 1);
            const size_t right = std::min(left + 1, out.size() - 1);
            const double fraction = source - static_cast<double>(left);
            const double sample =
                static_cast<double>(out[left]) * (1.0 - fraction) +
                static_cast<double>(out[right]) * fraction;
            scaled[i] = static_cast<int16_t>(
                std::clamp(std::lround(sample), -32768l, 32767l));
        }

        out = std::move(scaled);
    }

    const size_t fadeSamples = std::min(msToSamples(cfg_.edge_fade_ms, sample_rate),
                                       out.size() / 2);
    if (fadeSamples > 1) {
        for (size_t i = 0; i < fadeSamples; ++i) {
            const double gain = static_cast<double>(i) /
                static_cast<double>(fadeSamples);
            out[i] = static_cast<int16_t>(std::lround(out[i] * gain));
            const size_t tail = out.size() - 1 - i;
            out[tail] = static_cast<int16_t>(std::lround(out[tail] * gain));
        }
    }

    return out;
}

void VieNeuTtsManager::applyPlaybackVolume(std::vector<int16_t>* pcm) const
{
    if (!pcm || pcm->empty()) {
        return;
    }

    const float volume = std::clamp(cfg_.playback_volume, 0.0f, 1.0f);
    if (std::abs(volume - 1.0f) <= 0.001f) {
        return;
    }

    for (int16_t& sample : *pcm) {
        const long scaled = std::lround(static_cast<float>(sample) * volume);
        sample = static_cast<int16_t>(std::clamp(scaled, -32768l, 32767l));
    }
}

bool VieNeuTtsManager::speak(const std::string& text)
{
    if (text.empty()) {
        std::cerr << "[VieNeuTTS] Empty text\n";
        return false;
    }

    if (!initialize()) {
        return false;
    }

    stop();

    stop_requested_.store(false);
    engine_.resetStopRequested();
    worker_.resetStopRequested();
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

    const std::vector<std::string> chunks = splitTextForSynthesis(text);
    if (chunks.empty()) {
        std::cerr << "[VieNeuTTS] Empty text after chunking\n";
        return false;
    }

    std::cout << "[VieNeuTTS] synthesis chunks="
              << chunks.size()
              << " min_chars="
              << std::max(0, cfg_.min_chunk_chars)
              << " target_chars="
              << std::max(80, cfg_.target_chunk_chars)
              << " max_chars="
              << std::max(120, cfg_.max_chunk_chars)
              << "\n";

    std::vector<int16_t> combinedPcm;
    uint32_t combinedSampleRate = 0;

    for (size_t i = 0; i < chunks.size(); ++i) {
        if (stop_requested_.load()) {
            return false;
        }

        std::vector<int16_t> pcm;
        uint32_t sampleRate = 0;

        const std::string presetPath = presetPathForChunk(chunks[i]);
        if (!presetPath.empty() && fs::exists(presetPath)) {
            std::cout << "[VieNeuTTS] preset hit chunk="
                      << (i + 1)
                      << "/"
                      << chunks.size()
                      << " path="
                      << presetPath
                      << "\n";
            uint16_t channels = 0;
            if (!VieNeuTtsWorker::loadWavPcm16File(presetPath, &pcm, &sampleRate, &channels)) {
                return false;
            }
            if (channels != 1) {
                std::cerr << "[VieNeuTTS] Preset chunk is not mono: "
                          << presetPath
                          << " channels="
                          << channels
                          << "\n";
                return false;
            }
        } else {
            const std::string cachePath = cachePathForChunk(chunks[i]);
            if (!cachePath.empty() && fs::exists(cachePath)) {
                std::cout << "[VieNeuTTS] cache hit chunk="
                          << (i + 1)
                          << "/"
                          << chunks.size()
                          << " path="
                          << cachePath
                          << "\n";
                uint16_t channels = 0;
                if (!VieNeuTtsWorker::loadWavPcm16File(cachePath, &pcm, &sampleRate, &channels)) {
                    return false;
                }
                if (channels != 1) {
                    std::cerr << "[VieNeuTTS] Cache chunk is not mono: "
                              << cachePath
                              << " channels="
                              << channels
                              << "\n";
                    return false;
                }
            } else {
                if (!engine_.synthesizeToPcm16(chunks[i], &pcm, &sampleRate)) {
                    std::cerr << "[VieNeuTTS] Failed to synthesize chunk "
                              << (i + 1)
                              << "/"
                              << chunks.size()
                              << ": "
                              << chunks[i].substr(0, 160)
                              << "\n";
                    return false;
                }

                pcm = prepareForPlayback(pcm, sampleRate);
                if (pcm.empty()) {
                    std::cerr << "[VieNeuTTS] Empty PCM after playback preparation\n";
                    return false;
                }

                if (cfg_.dump_wav && !cfg_.output_wav_path.empty()) {
                    const std::string dumpPath = chunks.size() == 1
                        ? cfg_.output_wav_path
                        : cfg_.output_wav_path + ".chunk" + std::to_string(i + 1) + ".wav";
                    (void)engine_.savePcm16ToFile(dumpPath, pcm);
                }

                if (!cachePath.empty()) {
                    try {
                        fs::create_directories(fs::path(cachePath).parent_path());
                        if (engine_.savePcm16ToFile(cachePath, pcm)) {
                            std::cout << "[VieNeuTTS] cache saved path="
                                      << cachePath
                                      << "\n";
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "[VieNeuTTS] cache save failed: "
                                  << e.what()
                                  << "\n";
                    }
                }
            }
        }

        if (pcm.empty()) {
            std::cerr << "[VieNeuTTS] Empty PCM chunk "
                      << (i + 1)
                      << "/"
                      << chunks.size()
                      << "\n";
            return false;
        }

        if (combinedSampleRate == 0) {
            combinedSampleRate = sampleRate;
        } else if (sampleRate != combinedSampleRate) {
            std::cerr << "[VieNeuTTS] Chunk sample rate mismatch: "
                      << sampleRate
                      << " != "
                      << combinedSampleRate
                      << "\n";
            return false;
        }

        const int pauseMs = i > 0 ? pauseMsAfterText(chunks[i - 1]) : 0;
        const size_t joinSilenceSamples = msToSamples(pauseMs, combinedSampleRate);
        if (!combinedPcm.empty() && joinSilenceSamples > 0) {
            combinedPcm.insert(combinedPcm.end(), joinSilenceSamples, 0);
        }
        combinedPcm.insert(combinedPcm.end(), pcm.begin(), pcm.end());
    }

    if (combinedPcm.empty() || combinedSampleRate == 0) {
        std::cerr << "[VieNeuTTS] Empty combined PCM\n";
        return false;
    }

    if (cfg_.dump_wav && !cfg_.output_wav_path.empty() && chunks.size() > 1) {
        (void)engine_.savePcm16ToFile(cfg_.output_wav_path, combinedPcm);
    }

    applyPlaybackVolume(&combinedPcm);

    if (!worker_.playPcm16WithAlsaAndMeter(combinedPcm, combinedSampleRate, 1)) {
        return false;
    }

    return !stop_requested_.load();
}
