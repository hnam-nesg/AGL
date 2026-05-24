#include "CommandSegmenter.h"

#include <algorithm>
#include <cmath>
#include <iostream>

CommandSegmenter::CommandSegmenter(const Config& cfg)
    : cfg_(cfg), silero_(cfg.silero_vad_cfg), hpf_(cfg.high_pass_alpha)
{
}

CommandSegmenter::~CommandSegmenter()
{
#ifdef USE_RNNOISE
    destroyRnnoise();
#endif
}

bool CommandSegmenter::initialize(QString* errorMessage)
{
#ifdef USE_RNNOISE
    if (cfg_.enable_rnnoise) {
        if (!initRnnoise(errorMessage)) {
            return false;
        }
    }
#endif

    if (cfg_.enable_silero_vad) {
        std::string err;
        if (!silero_.initialize(&err)) {
            if (errorMessage) {
                *errorMessage = QString::fromStdString("Silero VAD init failed: " + err);
            }
            return false;
        }
    }

    resetForNewCommand();
    return true;
}

void CommandSegmenter::resetForNewCommand()
{
    state_ = State::ArmedAfterWake;

    vad_pending_samples_.clear();
    preroll_raw_chunks_.clear();
    preroll_clean_chunks_.clear();
    preroll_raw_chunks_.reserve(std::max(1, cfg_.preroll_chunks));
    preroll_clean_chunks_.reserve(std::max(1, cfg_.preroll_chunks));

    utterance_raw_pcm16_.clear();
    utterance_clean_pcm16_.clear();
    utterance_f32_.clear();
    partial_window_f32_.clear();

    total_received_samples_ = 0;
    in_speech_samples_ = 0;
    partial_samples_since_last_run_ = 0;

    consecutive_speech_chunks_ = 0;
    consecutive_silent_chunks_ = 0;
    vad_in_speech_ = false;

    hpf_.reset();

    if (cfg_.enable_silero_vad) {
        silero_.reset();
    }
}

int CommandSegmenter::samplesFromMs(int ms) const
{
    return std::max(1, static_cast<int>((static_cast<long long>(cfg_.sample_rate) * ms) / 1000));
}

double CommandSegmenter::calcRms16(const std::vector<int16_t>& chunk)
{
    if (chunk.empty()) {
        return 0.0;
    }

    long double sumsq = 0.0;
    for (int16_t v : chunk) {
        const long double x = static_cast<long double>(v);
        sumsq += x * x;
    }
    return std::sqrt(static_cast<double>(sumsq / chunk.size()));
}

std::vector<float> CommandSegmenter::pcm16ToFloat(const std::vector<int16_t>& pcm16) const
{
    std::vector<float> out;
    out.reserve(pcm16.size());
    for (int16_t s : pcm16) {
        out.push_back(static_cast<float>(s) / 32768.0f);
    }
    return out;
}

std::vector<int16_t> CommandSegmenter::gateChunk(const std::vector<int16_t>& in16k)
{
    return in16k;
}

std::vector<int16_t> CommandSegmenter::denoiseChunk(const std::vector<int16_t>& in16k)
{
    if (in16k.empty()) {
        return {};
    }

    std::vector<int16_t> work = in16k;

    if (cfg_.enable_high_pass) {
        for (auto& s : work) {
            s = hpf_.process(s);
        }
    }

#ifdef USE_RNNOISE
    if (cfg_.enable_rnnoise && rnnoise_state_) {
        std::vector<int16_t> up48 = resample16kTo48k(work);
        std::vector<int16_t> clean48 = applyRnnoise48k(up48);
        return resample48kTo16k(clean48);
    }
#endif

    return work;
}

CommandSegmenter::VadDecision CommandSegmenter::detectSpeechBySileroChunked(const std::vector<int16_t>& gateAudio)
{
    VadDecision decision;

    if (!cfg_.enable_silero_vad || !silero_.isInitialized()) {
        return decision;
    }

    const int win = silero_.windowSizeSamples();
    if (win <= 0) {
        return decision;
    }

    vad_pending_samples_.insert(vad_pending_samples_.end(), gateAudio.begin(), gateAudio.end());

    bool anySpeech = false;
    float maxProb = 0.0f;
    int processed = 0;

    while (static_cast<int>(vad_pending_samples_.size()) >= win) {
        std::vector<int16_t> frame(
            vad_pending_samples_.begin(),
            vad_pending_samples_.begin() + win
        );

        vad_pending_samples_.erase(
            vad_pending_samples_.begin(),
            vad_pending_samples_.begin() + win
        );

        const auto result = silero_.processPcm16(frame);
        if (!result.ok) {
            continue;
        }

        ++processed;
        maxProb = std::max(maxProb, result.speech_prob);

        bool frameSpeech = false;
        if (vad_in_speech_) {
            frameSpeech = (result.speech_prob >= cfg_.silero_vad_cfg.silence_threshold);
        } else {
            frameSpeech = (result.speech_prob >= cfg_.silero_vad_cfg.speech_threshold);
        }

        if (frameSpeech) {
            anySpeech = true;
        }

        if (cfg_.silero_vad_cfg.debug) {
            std::cerr << "[SileroVAD] prob=" << result.speech_prob
                      << " vad_in_speech=" << (vad_in_speech_ ? 1 : 0)
                      << " frameSpeech=" << (frameSpeech ? 1 : 0)
                      << "\n";
        }
    }

    if (processed > 0) {
        decision.valid = true;
        decision.isSpeech = anySpeech;
        decision.maxSpeechProb = maxProb;
        decision.processedFrames = processed;
    }

    return decision;
}

bool CommandSegmenter::fallbackSpeechByRms(const std::vector<int16_t>& gateAudio, double* rms16Out) const
{
    const double rms16 = calcRms16(gateAudio);
    if (rms16Out) {
        *rms16Out = rms16;
    }
    return rms16 >= (cfg_.fallback_start_speech_rms_threshold * 32768.0f);
}

void CommandSegmenter::finalize(Output* out, const QString& reason)
{
    if (!out) return;

    state_ = State::Finalized;
    out->utterance_finalized = true;
    out->finalize_reason = reason;
    out->final_utterance_f32 = utterance_f32_;
    out->final_raw_pcm16 = utterance_raw_pcm16_;
    out->final_clean_pcm16 = utterance_clean_pcm16_;
}

void CommandSegmenter::pushChunk(const std::vector<int16_t>& rawChunk, Output* out)
{
    if (!out) return;
    *out = Output{};

    if (state_ == State::Idle || state_ == State::Finalized || rawChunk.empty()) {
        return;
    }

    total_received_samples_ += static_cast<int>(rawChunk.size());

    const std::vector<int16_t> gate = gateChunk(rawChunk);
    const std::vector<int16_t> clean = denoiseChunk(rawChunk);

    out->rms16 = calcRms16(gate);

    bool isSpeech = false;
    if (cfg_.enable_silero_vad && silero_.isInitialized()) {
        const auto dec = detectSpeechBySileroChunked(gate);
        if (dec.valid) {
            isSpeech = dec.isSpeech;
            out->speech_prob = dec.maxSpeechProb;
        } else {
            isSpeech = fallbackSpeechByRms(gate);
        }
    } else {
        isSpeech = fallbackSpeechByRms(gate);
    }
    vad_in_speech_ = isSpeech;

    const int startNeeded =
        (cfg_.enable_silero_vad && silero_.isInitialized())
            ? std::max(1, cfg_.silero_vad_cfg.start_trigger_chunks)
            : 1;

    const int endNeeded =
        (cfg_.enable_silero_vad && silero_.isInitialized())
            ? std::max(1, cfg_.silero_vad_cfg.end_trigger_chunks)
            : 6;

    if (state_ == State::ArmedAfterWake) {
        preroll_raw_chunks_.push_back(rawChunk);
        preroll_clean_chunks_.push_back(clean);

        if (static_cast<int>(preroll_raw_chunks_.size()) > std::max(1, cfg_.preroll_chunks)) {
            preroll_raw_chunks_.erase(preroll_raw_chunks_.begin());
        }
        if (static_cast<int>(preroll_clean_chunks_.size()) > std::max(1, cfg_.preroll_chunks)) {
            preroll_clean_chunks_.erase(preroll_clean_chunks_.begin());
        }

        consecutive_speech_chunks_ = isSpeech ? (consecutive_speech_chunks_ + 1) : 0;

        if (consecutive_speech_chunks_ >= startNeeded) {
            state_ = State::InSpeech;
            consecutive_silent_chunks_ = 0;
            in_speech_samples_ = 0;
            partial_samples_since_last_run_ = 0;
            out->speech_started = true;

            for (const auto& raw : preroll_raw_chunks_) {
                utterance_raw_pcm16_.insert(utterance_raw_pcm16_.end(), raw.begin(), raw.end());
            }

            for (const auto& c : preroll_clean_chunks_) {
                utterance_clean_pcm16_.insert(utterance_clean_pcm16_.end(), c.begin(), c.end());
                auto f = pcm16ToFloat(c);
                utterance_f32_.insert(utterance_f32_.end(), f.begin(), f.end());
                partial_window_f32_.insert(partial_window_f32_.end(), f.begin(), f.end());
                in_speech_samples_ += static_cast<int>(c.size());
                partial_samples_since_last_run_ += static_cast<int>(c.size());
            }

            preroll_raw_chunks_.clear();
            preroll_clean_chunks_.clear();
            return;
        }

        if (total_received_samples_ >= samplesFromMs(cfg_.no_speech_timeout_ms)) {
            finalize(out, QStringLiteral("no_speech_timeout"));
        }
        return;
    }

    if (state_ == State::InSpeech) {
        utterance_raw_pcm16_.insert(utterance_raw_pcm16_.end(), rawChunk.begin(), rawChunk.end());
        utterance_clean_pcm16_.insert(utterance_clean_pcm16_.end(), clean.begin(), clean.end());

        auto f = pcm16ToFloat(clean);
        utterance_f32_.insert(utterance_f32_.end(), f.begin(), f.end());
        partial_window_f32_.insert(partial_window_f32_.end(), f.begin(), f.end());

        in_speech_samples_ += static_cast<int>(clean.size());
        partial_samples_since_last_run_ += static_cast<int>(clean.size());

        const int partialWindowSamples = samplesFromMs(cfg_.partial_window_ms);
        if (static_cast<int>(partial_window_f32_.size()) > partialWindowSamples) {
            partial_window_f32_.erase(
                partial_window_f32_.begin(),
                partial_window_f32_.begin() + (static_cast<int>(partial_window_f32_.size()) - partialWindowSamples)
            );
        }

        consecutive_silent_chunks_ = isSpeech ? 0 : (consecutive_silent_chunks_ + 1);

        if (partial_samples_since_last_run_ >= samplesFromMs(cfg_.partial_update_interval_ms) &&
            static_cast<int>(utterance_f32_.size()) >= samplesFromMs(std::max(600, cfg_.min_final_audio_ms / 2))) {
            out->partial_ready = true;
            out->partial_window_f32 = partial_window_f32_;
            partial_samples_since_last_run_ = 0;
        }

        if (consecutive_silent_chunks_ >= endNeeded &&
            static_cast<int>(utterance_f32_.size()) >= samplesFromMs(cfg_.min_final_audio_ms)) {
            finalize(out, QStringLiteral("silence_end"));
            return;
        }

        if (in_speech_samples_ >= samplesFromMs(cfg_.force_final_after_ms)) {
            finalize(out, QStringLiteral("force_final_timeout"));
            return;
        }
    }
}

#ifdef USE_RNNOISE
bool CommandSegmenter::initRnnoise(QString* errorMessage)
{
    destroyRnnoise();
    rnnoise_state_ = rnnoise_create(nullptr);
    if (!rnnoise_state_) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("rnnoise_create failed");
        }
        return false;
    }
    return true;
}

void CommandSegmenter::destroyRnnoise()
{
    if (rnnoise_state_) {
        rnnoise_destroy(rnnoise_state_);
        rnnoise_state_ = nullptr;
    }
}

std::vector<int16_t> CommandSegmenter::resample16kTo48k(const std::vector<int16_t>& in) const
{
    if (in.empty()) return {};
    if (in.size() == 1) return {in[0], in[0], in[0]};

    std::vector<int16_t> out;
    out.reserve(in.size() * 3);

    for (size_t i = 0; i + 1 < in.size(); ++i) {
        const float s0 = static_cast<float>(in[i]);
        const float s1 = static_cast<float>(in[i + 1]);

        out.push_back(static_cast<int16_t>(std::clamp(s0, -32768.0f, 32767.0f)));
        out.push_back(static_cast<int16_t>(std::clamp(s0 + (s1 - s0) / 3.0f, -32768.0f, 32767.0f)));
        out.push_back(static_cast<int16_t>(std::clamp(s0 + 2.0f * (s1 - s0) / 3.0f, -32768.0f, 32767.0f)));
    }

    out.push_back(in.back());
    out.push_back(in.back());
    out.push_back(in.back());
    return out;
}

std::vector<int16_t> CommandSegmenter::resample48kTo16k(const std::vector<int16_t>& in) const
{
    if (in.empty()) return {};
    std::vector<int16_t> out;
    out.reserve((in.size() + 2) / 3);
    for (size_t i = 0; i < in.size(); i += 3) {
        out.push_back(in[i]);
    }
    return out;
}

std::vector<int16_t> CommandSegmenter::applyRnnoise48k(const std::vector<int16_t>& pcm48k)
{
    std::vector<int16_t> out;
    out.reserve(pcm48k.size());

    size_t pos = 0;
    while (pos < pcm48k.size()) {
        float frame[480] = {0.0f};
        const size_t remain = pcm48k.size() - pos;
        const size_t copyN = std::min<size_t>(480, remain);

        for (size_t i = 0; i < copyN; ++i) {
            frame[i] = static_cast<float>(pcm48k[pos + i]);
        }

        rnnoise_process_frame(rnnoise_state_, frame, frame);

        for (size_t i = 0; i < copyN; ++i) {
            float v = frame[i];
            if (v > 32767.0f) v = 32767.0f;
            if (v < -32768.0f) v = -32768.0f;
            out.push_back(static_cast<int16_t>(v));
        }

        pos += copyN;
    }

    return out;
}
#endif