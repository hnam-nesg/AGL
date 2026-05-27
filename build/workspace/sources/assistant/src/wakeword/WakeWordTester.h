#pragma once

#include "../audio/AudioInputService.h"
#include "../asr/asrmanager.h"
#include "openwakeword/openwakeword_engine.h"

#include <QObject>
#include <atomic>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>

class WakeWordTest : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool activeGraphic READ getActiveGraphic NOTIFY activeGraphicChanged)
    Q_PROPERTY(double audioLevel READ getAudioLevel NOTIFY audioLevelChanged)
    Q_PROPERTY(QString lastUserText READ getLastUserText NOTIFY conversationChanged)
    Q_PROPERTY(QString lastAssistantText READ getLastAssistantText NOTIFY conversationChanged)
    Q_PROPERTY(QString visibleAssistantText READ getVisibleAssistantText NOTIFY conversationChanged)
    Q_PROPERTY(bool assistantThinking READ getAssistantThinking NOTIFY conversationChanged)
    Q_PROPERTY(bool panelActive READ getPanelActive NOTIFY panelActiveChanged)

public:
    struct Config {
        std::string alsa_device = "plughw:2,0";

        std::string mel_model_path = "/usr/share/openwakeword/models/melspectrogram.onnx";
        std::string embed_model_path = "/usr/share/openwakeword/models/embedding_model.onnx";
        std::string wakeword_model_path = "/usr/share/openwakeword/models/hey_kiki.onnx";

        unsigned int sample_rate = 16000;
        int channels = 1;
        snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

        size_t wakeword_chunk_samples = 1280;
        float wakeword_threshold = 0.5f;
        size_t wakeword_step_frames = 4;
        int wakeword_trigger_level = 4;
        int wakeword_refractory = 20;

        int startup_ignore_steps = 20;
        int alsa_buffer_periods = 4;

        bool debug = false;

        AsrManager::Config command_asr_cfg;
    };

public:
    explicit WakeWordTest(const Config& cfg);
    ~WakeWordTest();

    bool initialize();
    bool start();
    void stop();
    Q_INVOKABLE void dismissPanel();

signals:
    void activeGraphicChanged(bool active);
    void audioLevelChanged(double level);
    void conversationChanged();
    void panelActiveChanged(bool active);

private:
    enum class State {
        WaitingWakeword,
        RecordingCommand,
        SpeakingReply
    };

private:
    void onAudioChunk(const std::vector<int16_t>& chunk);
    void processWakewordChunk(const std::vector<int16_t>& chunk);
    void processCommandChunk(const std::vector<int16_t>& chunk);

    static double calcRms(const std::vector<int16_t>& chunk);

    void setActiveGraphicOnGuiThread(bool active);
    void setAudioLevelOnGuiThread(double level);
    void setPlaybackStateCallback(std::function<void(bool)> callback);

    bool getActiveGraphic() const { return m_active; }
    double getAudioLevel() const { return m_audioLevel; }
    QString getLastUserText() const { return m_lastUserText; }
    QString getLastAssistantText() const { return m_lastAssistantText; }
    QString getVisibleAssistantText() const { return m_visibleAssistantText; }
    bool getAssistantThinking() const { return m_assistantThinking; }
    bool getPanelActive() const { return m_panelActive; }

    void setActiveGraphic(bool active);
    void setAudioLevel(double level);
    void setPanelActiveOnGuiThread(bool active);
    void setPanelActive(bool active);
    void hideInteractionUiOnGuiThread();
    void clearInteractionUiOnGuiThread();
    void resetConversationOnGuiThread();
    void resetConversation();
    void setConversationOnGuiThread(const QString& userText,
                                    const QString& assistantText,
                                    bool thinking,
                                    double replyProgress);
    void setConversation(const QString& userText,
                         const QString& assistantText,
                         bool thinking,
                         double replyProgress);

private:
    Config cfg_;

    AudioInputService audioInput_;
    OpenWakeWordEngine engine_;
    AsrManager commandAsr_;

    std::atomic<bool> started_{false};
    std::atomic<bool> playbackActive_{false};

    std::mutex stateMutex_;
    State state_ = State::WaitingWakeword;
    size_t wakeword_step_index_ = 0;
    size_t wakeword_log_counter_ = 0;

    bool m_active = false;
    double m_audioLevel = 0.0;
    QString m_lastUserText;
    QString m_lastAssistantText;
    QString m_visibleAssistantText;
    bool m_assistantThinking = false;
    bool m_panelActive = false;

    std::function<void(bool)> playbackStateCallback_;
};
