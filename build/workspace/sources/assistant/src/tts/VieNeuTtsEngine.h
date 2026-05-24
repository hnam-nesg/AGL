#pragma once

#include "VieNeuTtsConfig.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

struct llama_context;
struct llama_model;
struct llama_sampler;
struct llama_vocab;

namespace Ort {
class Env;
class Session;
class SessionOptions;
} // namespace Ort

class VieNeuTtsEngine {
public:
    using Config = VieNeuTtsConfig;

    explicit VieNeuTtsEngine(const Config& cfg);
    ~VieNeuTtsEngine();

    bool initialize();
    bool isReady() const;
    bool synthesizeToPcm16(const std::string& text,
                           std::vector<int16_t>* pcm,
                           uint32_t* sample_rate);
    bool savePcm16ToFile(const std::string& wav_path,
                         const std::vector<int16_t>& pcm) const;
    bool synthesizeToFile(const std::string& text, const std::string& wav_path);

    bool setVoice(const std::string& voice);
    void stop();
    void resetStopRequested();
    bool isSynthesizing() const;

private:
    enum class PromptMode {
        Auto,
        Standard,
        Turbo,
    };

    struct VoicePreset {
        std::vector<int32_t> codes;
        std::vector<float> embedding;
        std::string text;
    };

private:
    bool loadBackbone();
    bool loadCodec();
    bool loadVoice();
    bool loadPhonemeDictionary();
    bool warmup();

    std::vector<int32_t> tokenize(const std::string& text, bool addSpecial, bool parseSpecial) const;
    std::string tokenToPiece(int32_t token, bool special) const;
    int32_t tokenForPiece(const std::string& text) const;
    std::string generateSpeechTokens(const std::string& prompt, int max_tokens);
    std::vector<int32_t> extractSpeechIds(const std::string& text) const;
    std::vector<float> decodeSpeechIds(const std::vector<int32_t>& codes);
    bool saveWav16(const std::string& path, const std::vector<int16_t>& pcm) const;

    std::string buildPrompt(const std::string& text) const;
    std::string preparePromptText(const std::string& text) const;
    std::string normalizePromptText(const std::string& text) const;
    std::string phonemizeWithDictionary(const std::string& text) const;
    PromptMode configuredPromptMode() const;
    PromptMode activePromptMode() const;
    std::string speechCodesToText(const std::vector<int32_t>& codes) const;
    static std::vector<int16_t> floatToPcm16(const std::vector<float>& audio);
    static std::string normalizeNumbers(std::string text);
    static std::string stripUtf8Bom(std::string text);
    static std::string readFile(const std::string& path);

private:
    Config cfg_;

    std::atomic<bool> stop_requested_{false};
    std::atomic<bool> synthesizing_{false};
    bool initialized_ = false;

    mutable std::mutex mutex_;

    llama_model* model_ = nullptr;
    llama_context* ctx_ = nullptr;
    const llama_vocab* vocab_ = nullptr;

    std::unique_ptr<Ort::Env> ort_env_;
    std::unique_ptr<Ort::SessionOptions> ort_session_options_;
    std::unique_ptr<Ort::Session> ort_session_;
    std::string codec_content_input_name_;
    std::string codec_voice_input_name_;
    std::string codec_output_name_;
    int codec_content_input_element_type_ = 0;
    size_t codec_content_input_rank_ = 0;
    size_t codec_voice_embedding_size_ = 0;
    bool codec_uses_voice_embedding_ = false;

    VoicePreset voice_;
    std::unordered_map<std::string, std::string> phoneme_dict_;
};
