#include "WakeWordTester.h"

#include <QMetaObject>
#include <QTimer>

#include <algorithm>
#include <iomanip>
#include <iostream>

WakeWordTest::WakeWordTest(const Config& cfg)
    : cfg_(cfg),
      audioInput_(AudioInputService::Config{
          cfg.alsa_device,
          cfg.sample_rate,
          cfg.channels,
          cfg.format,
          cfg.wakeword_chunk_samples,
          cfg.alsa_buffer_periods,
          cfg.debug
      }),
      engine_(cfg.mel_model_path, cfg.embed_model_path, cfg.wakeword_model_path),
      commandAsr_(cfg.command_asr_cfg)
{
}

WakeWordTest::~WakeWordTest()
{
    stop();
}

bool WakeWordTest::initialize()
{
    if (!engine_.initialize()) {
        std::cerr << "[WakeWordTest] Failed to initialize wakeword engine\n";
        return false;
    }

    engine_.reset();
    engine_.setThreshold(cfg_.wakeword_threshold);
    engine_.setStepFrames(cfg_.wakeword_step_frames);
    engine_.setTriggerLevel(cfg_.wakeword_trigger_level);
    engine_.setRefractory(cfg_.wakeword_refractory);

    QString asrError;
    if (!commandAsr_.initialize(&asrError)) {
        std::cerr << "[WakeWordTest] Failed to initialize command ASR: "
                  << asrError.toStdString() << "\n";
        return false;
    }

    commandAsr_.setAudioLevelCallback([this](double level) {
        this->setAudioLevelOnGuiThread(level);
    });

    commandAsr_.setTtsAudioLevelCallback([this](double level) {
        this->setAudioLevelOnGuiThread(level);
    });

    commandAsr_.setPlaybackStateCallback([this](bool active) {
        this->setActiveGraphicOnGuiThread(active);
        std::lock_guard<std::mutex> lock(stateMutex_);

        if (active && state_ != State::RecordingCommand) {
            state_ = State::SpeakingReply;
            return;
        }

        if (!active && state_ == State::SpeakingReply) {
            std::cout << "[WakeWord] Reply finished -> back to waiting wakeword\n";
            engine_.reset();
            wakeword_step_index_ = 0;
            state_ = State::WaitingWakeword;
            QTimer::singleShot(3000, this, [this]() {
                std::lock_guard<std::mutex> lock(stateMutex_);
                if (state_ == State::WaitingWakeword) {
                    hideInteractionUiOnGuiThread();
                }
            });
        }
    });

    commandAsr_.setConversationUpdateCallback(
        [this](const QString& userText,
               const QString& assistantText,
               bool thinking,
               double replyProgress) {
            this->setConversationOnGuiThread(userText,
                                             assistantText,
                                             thinking,
                                             replyProgress);
        });

    std::cout << "[WakeWordTest] Initialized successfully\n";

    return true;
}

bool WakeWordTest::start()
{
    stop();

    wakeword_step_index_ = 0;
    wakeword_log_counter_ = 0;
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        state_ = State::WaitingWakeword;
    }
    return audioInput_.start([this](const std::vector<int16_t>& chunk) {
        this->onAudioChunk(chunk);
    });
}

void WakeWordTest::stop()
{
    audioInput_.stop();
    commandAsr_.interruptAllAndPrepareForNewCommand();

    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        state_ = State::WaitingWakeword;
    }
    clearInteractionUiOnGuiThread();
}

void WakeWordTest::dismissPanel()
{
    setPanelActive(false);
}

void WakeWordTest::onAudioChunk(const std::vector<int16_t>& chunk)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    switch (state_) {
    case State::WaitingWakeword:
        processWakewordChunk(chunk);
        break;

    case State::RecordingCommand:
        processCommandChunk(chunk);
        break;

    case State::SpeakingReply:
        // TTS đang phát: bỏ qua mic input để không tự cắt câu trả lời hoặc tạo command rỗng.
        break;
    }
}

void WakeWordTest::processWakewordChunk(const std::vector<int16_t>& chunk)
{

    engine_.setDebugStep(
        static_cast<int>(wakeword_step_index_),
        cfg_.debug && wakeword_step_index_ < 40
    );

    const bool detected = engine_.processAudioChunk(chunk);
    const float score = engine_.lastScore();
    const double rms = calcRms(chunk);

    if ((wakeword_log_counter_++ % 20) == 0) {
        std::cout << "[WakeWord] step=" << wakeword_step_index_
                  << " rms=" << std::fixed << std::setprecision(2) << rms
                  << " score=" << std::setprecision(6) << score
                  << "\n";
    }

    bool acceptDetection = detected;
    if (wakeword_step_index_ < static_cast<size_t>(cfg_.startup_ignore_steps)) {
        acceptDetection = false;
    }

    ++wakeword_step_index_;

    if (!acceptDetection) {
        return;
    }

    std::cout << "[WakeWord] DETECTED\n";
    std::cout << "[WakeWord] Wake event accepted\n";

    state_ = State::RecordingCommand;
    commandAsr_.reset();
    resetConversationOnGuiThread();
    setActiveGraphicOnGuiThread(true);
}

void WakeWordTest::processCommandChunk(const std::vector<int16_t>& chunk)
{
    bool endOfUtterance = false;
    commandAsr_.feedCommandAudioChunk(chunk, &endOfUtterance);

    // CHANGED: bỏ log partial vì ASR giờ là tuần tự, không partial
    if (!endOfUtterance) {
        return;
    }

    const QString finalText = commandAsr_.lastFinalText().trimmed();
    if (!finalText.isEmpty()) {
        std::cout << "[ASR] Final text: " << finalText.toStdString() << "\n";

        // CHANGED: sau khi đã có command thì chuyển sang trạng thái assistant busy
        state_ = State::SpeakingReply;
        commandAsr_.handleFinalCommandText(finalText);
    } else {
        std::cout << "[Command] No usable command detected\n";
        if (!commandAsr_.lastError().isEmpty()) {
            std::cout << "[Command] Last error: "
                    << commandAsr_.lastError().toStdString() << "\n";
        }

        std::cout << "[WakeWord] Back to waiting wakeword\n";
        engine_.reset();
        wakeword_step_index_ = 0;
        state_ = State::WaitingWakeword;
        clearInteractionUiOnGuiThread();
    }
}

double WakeWordTest::calcRms(const std::vector<int16_t>& chunk)
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

void WakeWordTest::setActiveGraphicOnGuiThread(bool active)
{
    QMetaObject::invokeMethod(this, [this, active]() {
        setActiveGraphic(active);
    }, Qt::QueuedConnection);
}

void WakeWordTest::setAudioLevelOnGuiThread(double level)
{
    QMetaObject::invokeMethod(this, [this, level]() {
        setAudioLevel(level);
    }, Qt::QueuedConnection);
}

void WakeWordTest::setPanelActiveOnGuiThread(bool active)
{
    QMetaObject::invokeMethod(this, [this, active]() {
        setPanelActive(active);
    }, Qt::QueuedConnection);
}

void WakeWordTest::clearInteractionUiOnGuiThread()
{
    QMetaObject::invokeMethod(this, [this]() {
        resetConversation();
        setPanelActive(false);
        setActiveGraphic(false);
        setAudioLevel(0.0);
    }, Qt::QueuedConnection);
}

void WakeWordTest::hideInteractionUiOnGuiThread()
{
    QMetaObject::invokeMethod(this, [this]() {
        setPanelActive(false);
        setActiveGraphic(false);
        setAudioLevel(0.0);
    }, Qt::QueuedConnection);
}

void WakeWordTest::resetConversationOnGuiThread()
{
    QMetaObject::invokeMethod(this, [this]() {
        resetConversation();
    }, Qt::QueuedConnection);
}

void WakeWordTest::setConversationOnGuiThread(const QString& userText,
                                              const QString& assistantText,
                                              bool thinking,
                                              double replyProgress)
{
    QMetaObject::invokeMethod(this, [this, userText, assistantText, thinking, replyProgress]() {
        setConversation(userText, assistantText, thinking, replyProgress);
    }, Qt::QueuedConnection);
}

void WakeWordTest::setActiveGraphic(bool active)
{
    if (active != m_active) {
        m_active = active;
        emit activeGraphicChanged(active);
    }
}

void WakeWordTest::setAudioLevel(double level)
{
    if (std::abs(m_audioLevel - level) > 0.005) {
        m_audioLevel = level;
        emit audioLevelChanged(level);
    }
}

void WakeWordTest::setPanelActive(bool active)
{
    if (m_panelActive == active) {
        return;
    }

    m_panelActive = active;
    emit panelActiveChanged(active);
}

void WakeWordTest::resetConversation()
{
    const bool changed = !m_lastUserText.isEmpty() ||
        !m_lastAssistantText.isEmpty() ||
        !m_visibleAssistantText.isEmpty() ||
        m_assistantThinking;

    m_lastUserText.clear();
    m_lastAssistantText.clear();
    m_visibleAssistantText.clear();
    m_assistantThinking = false;

    if (changed) {
        emit conversationChanged();
    }
}

void WakeWordTest::setConversation(const QString& userText,
                                   const QString& assistantText,
                                   bool thinking,
                                   double replyProgress)
{
    if (assistantText.isEmpty() && !thinking && replyProgress >= 1.0) {
        const QString nextUserText = userText.isEmpty() ? m_lastUserText : userText;
        const bool changed = m_lastUserText != nextUserText ||
            !m_lastAssistantText.isEmpty() ||
            !m_visibleAssistantText.isEmpty() ||
            m_assistantThinking ||
            m_panelActive;

        m_lastUserText = nextUserText;
        m_lastAssistantText.clear();
        m_visibleAssistantText.clear();
        m_assistantThinking = false;
        setPanelActive(false);

        if (changed) {
            emit conversationChanged();
        }
        return;
    }

    QString nextUserText = m_lastUserText;
    QString nextAssistantText = m_lastAssistantText;
    if (!userText.isEmpty()) {
        nextUserText = userText;
    }
    if (!assistantText.isEmpty()) {
        nextAssistantText = assistantText;
    }

    const int replyLength = nextAssistantText.size();
    const int visibleLength = replyLength == 0
        ? 0
        : std::clamp(static_cast<int>(std::ceil(replyProgress * replyLength)),
                     0,
                     replyLength);
    const QString nextVisibleAssistantText = replyLength == 0
        ? QString()
        : nextAssistantText.left(visibleLength);
    const bool nextThinking = thinking && assistantText.isEmpty();

    if (m_lastUserText == nextUserText &&
        m_lastAssistantText == nextAssistantText &&
        m_visibleAssistantText == nextVisibleAssistantText &&
        m_assistantThinking == nextThinking) {
        return;
    }

    m_lastUserText = nextUserText;
    m_lastAssistantText = nextAssistantText;
    m_visibleAssistantText = nextVisibleAssistantText;
    m_assistantThinking = nextThinking;
    setPanelActive(true);
    emit conversationChanged();
}
