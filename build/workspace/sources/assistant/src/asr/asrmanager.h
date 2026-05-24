#pragma once

#include "../vad/CommandSegmenter.h"
#include "../tts/PiperTTS.h"
#include "../tts/VieNeuTtsManager.h"
#include "sherpaonnxengine.h"

#include "../nlu/PhoBertExecutionPolicy.h"
#include "../nlu/PhoBertMultiHeadClassifier.h"
#include "../nlu/PhoBertSlotExtractor.h"
#include "../llm/LlmQueryService.h"
#include "../services/HvacService.h"
#include "../services/MediaService.h"
#include "../services/PhoneService.h"
#include "../services/NavigationService.h"
#include "../services/VehicleService.h"

#include "AppLauncherClient.h"

#include <QString>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class AsrManager
{
public:
    struct Config {
        unsigned int sample_rate = 16000;
        int channels = 1;

        // Sherpa-ONNX offline ASR
        std::string sherpa_encoder_path = "/usr/share/zipformer-hynt/models/encoder-epoch-20-avg-10.int8.onnx";
        std::string sherpa_decoder_path = "/usr/share/zipformer-hynt/models/decoder-epoch-20-avg-10.int8.onnx";
        std::string sherpa_joiner_path  = "/usr/share/zipformer-hynt/models/joiner-epoch-20-avg-10.int8.onnx";
        std::string sherpa_tokens_path  = "/usr/share/zipformer-hynt/models/tokens.txt";
        int sherpa_threads = 4;
        std::string sherpa_provider = "cpu";
        bool sherpa_debug = false;

        std::string command_wav_path = "/tmp/command.wav";
        bool dump_cleaned_command_wav = true;
        std::string cleaned_command_wav_path = "/tmp/command_clean.wav";

        CommandSegmenter::Config segmenter_cfg;
        VieNeuTtsManager::Config tts_cfg;
        PiperTTS::Config piper_tts_cfg;

        // PhoBERT multi-head task-oriented NLU
        bool enable_nlu = true;
        bool enable_phobert_classifier = false;
        nlu::PhoBertMultiHeadClassifier::Config phobert_cfg;

        // Local LLM fallback for GENERAL_QA/FALLBACK_LLM query intents.
        bool enable_llm_query = false;
        llm::LlmQueryService::Config llm_cfg;

        // Tạm thời context HVAC lấy từ config.
        // Sau này nối VehicleSignals/Kuksa để đọc trạng thái thật.
        bool initial_ac_on = false;
        int initial_cabin_temp = -1;
        int initial_target_temp = -1;
        int initial_fan_level = -1;

        bool enable_tts_reply = true;
        bool require_tts_reply = false;
        int reply_max_chars = 250;
    };

    struct ReplyTask {
        uint64_t epoch = 0;
        QString text;
    };

public:
    explicit AsrManager(const Config& cfg);
    ~AsrManager();

    bool initialize(QString* errorMessage = nullptr);
    void reset();

    void feedCommandAudioChunk(const std::vector<int16_t>& chunk, bool* endOfUtterance);
    void handleFinalCommandText(const QString& text);

    void setAudioLevelCallback(std::function<void(double)> callback);
    void setTtsAudioLevelCallback(std::function<void(double)> callback);
    void setPlaybackStateCallback(std::function<void(bool)> callback);
    void setConversationUpdateCallback(
        std::function<void(const QString&, const QString&, bool, double)> callback);

    QString lastPartialText() const;
    QString lastFinalText() const;
    QString lastError() const;

    void interruptAllAndPrepareForNewCommand();
    bool isPlaybackActive() const;
    bool isBusyReplying() const;

    std::function<void(double)> audioLevelCallback_;
    std::function<void(bool)> playbackStateCallback_;
    std::function<void(const QString&, const QString&, bool, double)> conversationUpdateCallback_;

private:
    void dumpCommandToWav(const std::vector<int16_t>& pcm16, const std::string& path);
    QString cleanupReplyText(const QString& text) const;

    void startReplyWorker();
    void stopReplyWorker();
    void replyWorkerLoop();

    static double calcRms16(const std::vector<int16_t>& chunk);

    void setPlaybackActive(bool active);

    nlu::VehicleContext currentVehicleContext() const;
    QString processNluAndPrepare(const QString& text,
                                 QString* deferredAction,
                                 nlu::SlotMap* deferredSlotValues,
                                 QString* unsupportedPrefix);
    QString executeResolvedAction(const QString& action,
                                  const nlu::SlotMap& slotValues,
                                  const QString& unsupportedPrefix);
    void launchUiForAction(const QString& action);
    bool waitForUiBackendForAction(const QString& action, int timeoutMs) const;
    nlu::ResolvedAction resolveWithPhoBert(const QString& text,
                                           const nlu::VehicleContext& context);

private:
    Config cfg_;

    SherpaOnnxOfflineEngine sherpa_;
    CommandSegmenter segmenter_;
    VieNeuTtsManager tts_;
    PiperTTS piper_tts_;

    nlu::PhoBertMultiHeadClassifier phobertClassifier_;
    nlu::PhoBertSlotExtractor slotExtractor_;
    nlu::PhoBertExecutionPolicy executionPolicy_;
    llm::LlmQueryService llmQueryService_;
    HvacService hvacService_;
    MediaService mediaService_;
    PhoneService phoneService_;
    NavigationService navigationService_;
    VehicleService vehicleService_;
    AppLauncherClient appLauncher_;

    QString command_last_partial_;
    QString command_last_final_;
    QString command_last_error_;

    std::thread reply_thread_;
    std::mutex reply_mutex_;
    std::condition_variable reply_cv_;
    bool stop_reply_worker_ = false;
    std::deque<ReplyTask> reply_queue_;

    std::atomic<bool> playback_active_{false};
    std::atomic<bool> reply_busy_{false};
    std::atomic<bool> interrupt_requested_{false};
    std::atomic<uint64_t> epoch_{0};
    std::atomic<bool> tts_available_{false};
};
