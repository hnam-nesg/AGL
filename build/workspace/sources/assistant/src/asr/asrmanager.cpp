#include "asrmanager.h"

#include "../tts/AssistantTtsSettings.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QStringList>
#include <QThread>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>

namespace {

struct UiTarget {
    QString appId;
    QString dbusService;
};

static UiTarget uiTargetForAction(const QString& action)
{
    if (action == QStringLiteral("MEDIA_PLAY_SONG") ||
        action == QStringLiteral("MEDIA_PLAY_ARTIST") ||
        action == QStringLiteral("MEDIA_LIST_BY_ARTIST")) {
        return {QStringLiteral("social"), QStringLiteral("org.agl.social")};
    }

    // if (action.startsWith(QStringLiteral("MEDIA_"))) {
    //     return {QStringLiteral("mediaplayer"), QStringLiteral("org.agl.media")};
    // }

    if (action.startsWith(QStringLiteral("HVAC_"))) {
        return {QStringLiteral("hvac"), QStringLiteral("org.agl.hvac")};
    }

    if (action.startsWith(QStringLiteral("PHONE_"))) {
        return {QStringLiteral("phone"), QStringLiteral("org.agl.Phone")};
    }

    if (action.startsWith(QStringLiteral("NAVIGATION_"))) {
        return {QStringLiteral("navigation"), QStringLiteral("org.agl.navigation")};
    }

    return {};
}

static bool isDbusServiceRegisteredOn(const QDBusConnection& bus, const QString& service)
{
    if (!bus.isConnected() || service.isEmpty()) {
        return false;
    }

    QDBusConnectionInterface* iface = bus.interface();
    if (!iface) {
        return false;
    }

    const QDBusReply<bool> reply = iface->isServiceRegistered(service);
    return reply.isValid() && reply.value();
}

static bool isDbusServiceRegistered(const QString& service)
{
    return isDbusServiceRegisteredOn(QDBusConnection::systemBus(), service) ||
           isDbusServiceRegisteredOn(QDBusConnection::sessionBus(), service);
}

static bool requestHomescreenShowApp(const QString& appId)
{
    struct CandidateBus {
        const char* name;
        QDBusConnection connection;
    };

    const CandidateBus buses[] = {
        {"system", QDBusConnection::systemBus()},
        {"session", QDBusConnection::sessionBus()}
    };

    QStringList errors;

    for (const CandidateBus& bus : buses) {
        if (!bus.connection.isConnected()) {
            errors.push_back(QStringLiteral("%1 bus unavailable: %2")
                                 .arg(QString::fromLatin1(bus.name),
                                      bus.connection.lastError().message()));
            continue;
        }

        QDBusMessage message = QDBusMessage::createMethodCall(
            QStringLiteral("org.agl.homescreen"),
            QStringLiteral("/org/agl/homescreen"),
            QStringLiteral("org.agl.homescreen"),
            QStringLiteral("ShowApp"));
        message << appId;

        const QDBusMessage reply = bus.connection.call(message, QDBus::Block, 1200);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            errors.push_back(QStringLiteral("%1 bus ShowApp failed: %2")
                                 .arg(QString::fromLatin1(bus.name),
                                      reply.errorMessage()));
            continue;
        }

        if (!reply.arguments().isEmpty() && reply.arguments().first().canConvert<bool>()) {
            const bool ok = reply.arguments().first().toBool();
            std::cout << "[Assistant] Homescreen ShowApp "
                      << appId.toStdString()
                      << " via "
                      << bus.name
                      << " bus result="
                      << ok
                      << "\n";
            return ok;
        }

        return true;
    }

    std::cerr << "[Assistant] Failed to request homescreen ShowApp "
              << appId.toStdString()
              << ": "
              << errors.join(QStringLiteral("; ")).toStdString()
              << "\n";
    return false;
}

static bool shouldSkipTtsForAction(const QString& action)
{
    const UiTarget target = uiTargetForAction(action);
    return target.appId == QStringLiteral("phone") ||
           target.appId == QStringLiteral("social");
}

static void writeWavHeader(std::ofstream& f,
                           uint32_t sample_rate,
                           uint16_t channels,
                           uint16_t bits_per_sample,
                           uint32_t data_size)
{
    const uint32_t byte_rate = sample_rate * channels * bits_per_sample / 8;
    const uint16_t block_align = channels * bits_per_sample / 8;
    const uint32_t riff_chunk_size = 36 + data_size;

    f.seekp(0, std::ios::beg);

    f.write("RIFF", 4);
    f.write(reinterpret_cast<const char*>(&riff_chunk_size), 4);
    f.write("WAVE", 4);

    f.write("fmt ", 4);
    const uint32_t fmt_chunk_size = 16;
    const uint16_t audio_format = 1;
    f.write(reinterpret_cast<const char*>(&fmt_chunk_size), 4);
    f.write(reinterpret_cast<const char*>(&audio_format), 2);
    f.write(reinterpret_cast<const char*>(&channels), 2);
    f.write(reinterpret_cast<const char*>(&sample_rate), 4);
    f.write(reinterpret_cast<const char*>(&byte_rate), 4);
    f.write(reinterpret_cast<const char*>(&block_align), 2);
    f.write(reinterpret_cast<const char*>(&bits_per_sample), 2);

    f.write("data", 4);
    f.write(reinterpret_cast<const char*>(&data_size), 4);
}

static QString cutAtStopMarkers(QString text)
{
    const QStringList stops = {
        QStringLiteral("Người dùng:"),
        QStringLiteral("User:"),
        QStringLiteral("<|user|>"),
        QStringLiteral("<|assistant|>"),
        QStringLiteral("Assistant:"),
        QStringLiteral("Trợ lý:")
    };

    for (const QString& s : stops) {
        const int idx = text.indexOf(s, 1, Qt::CaseInsensitive);
        if (idx > 0) {
            text = text.left(idx).trimmed();
        }
    }

    return text.trimmed();
}

static QString digitToVietnameseWord(QChar digit)
{
    switch (digit.unicode()) {
    case '0': return QStringLiteral("không");
    case '1': return QStringLiteral("một");
    case '2': return QStringLiteral("hai");
    case '3': return QStringLiteral("ba");
    case '4': return QStringLiteral("bốn");
    case '5': return QStringLiteral("năm");
    case '6': return QStringLiteral("sáu");
    case '7': return QStringLiteral("bảy");
    case '8': return QStringLiteral("tám");
    case '9': return QStringLiteral("chín");
    default: return QString(digit);
    }
}

static QString integerToVietnameseWords(int value)
{
    static const QStringList underTen = {
        QStringLiteral("không"), QStringLiteral("một"), QStringLiteral("hai"),
        QStringLiteral("ba"), QStringLiteral("bốn"), QStringLiteral("năm"),
        QStringLiteral("sáu"), QStringLiteral("bảy"), QStringLiteral("tám"),
        QStringLiteral("chín")
    };

    if (value >= 0 && value < underTen.size()) {
        return underTen[value];
    }
    if (value == 10) {
        return QStringLiteral("mười");
    }
    if (value > 10 && value < 20) {
        const int ones = value % 10;
        return ones == 5
            ? QStringLiteral("mười lăm")
            : QStringLiteral("mười %1").arg(underTen[ones]);
    }
    if (value >= 20 && value < 100) {
        const int tens = value / 10;
        const int ones = value % 10;
        QString out = QStringLiteral("%1 mươi").arg(underTen[tens]);
        if (ones == 1) return out + QStringLiteral(" mốt");
        if (ones == 4) return out + QStringLiteral(" tư");
        if (ones == 5) return out + QStringLiteral(" lăm");
        if (ones > 0) return out + QStringLiteral(" ") + underTen[ones];
        return out;
    }
    if (value >= 100 && value < 1000) {
        const int hundreds = value / 100;
        const int remainder = value % 100;
        QString out = QStringLiteral("%1 trăm").arg(underTen[hundreds]);
        if (remainder == 0) return out;
        if (remainder < 10) return out + QStringLiteral(" lẻ ") + underTen[remainder];
        return out + QStringLiteral(" ") + integerToVietnameseWords(remainder);
    }

    QStringList digits;
    const QString raw = QString::number(value);
    for (const QChar& ch : raw) {
        digits.push_back(digitToVietnameseWord(ch));
    }
    return digits.join(QStringLiteral(" "));
}

static QString numericTokenToVietnameseWords(const QString& token)
{
    if ((token.size() > 1 && token.startsWith(QStringLiteral("0"))) || token.size() >= 4) {
        QStringList digits;
        for (const QChar& ch : token) {
            digits.push_back(digitToVietnameseWord(ch));
        }
        return digits.join(QStringLiteral(" "));
    }

    bool ok = false;
    const int value = token.toInt(&ok);
    if (!ok) {
        return token;
    }
    return integerToVietnameseWords(value);
}

static QString textForTts(QString text)
{
    text.remove(QRegularExpression(QStringLiteral("<\\|[^>]+\\|>")));
    text.remove(QRegularExpression(QStringLiteral("[*_`#>\\[\\]{}]")));
    text.replace(QRegularExpression(QStringLiteral("https?://\\S+")), QStringLiteral(" đường dẫn "));
    text.replace(QStringLiteral("&"), QStringLiteral(" và "));
    text.replace(QChar(0x2010), QStringLiteral("-"));
    text.replace(QChar(0x2011), QStringLiteral("-"));
    text.replace(QChar(0x2012), QStringLiteral("-"));
    text.replace(QChar(0x2013), QStringLiteral("-"));
    text.replace(QChar(0x2014), QStringLiteral("-"));
    text.replace(QChar(0x2212), QStringLiteral("-"));
    text.replace(QRegularExpression(QStringLiteral("\\s*-\\s*")), QStringLiteral(" "));
    text.replace(QRegularExpression(QStringLiteral("\\.{2,}")), QStringLiteral("."));
    text.replace(QRegularExpression(QStringLiteral("\\s+([,.!?;:])")), QStringLiteral("\\1"));
    text.replace(QRegularExpression(QStringLiteral("([,.!?;:]){2,}")), QStringLiteral("\\1"));

    const QRegularExpression numberRe(QStringLiteral("\\d+"));
    qsizetype offset = 0;
    QRegularExpressionMatch match;

    while ((match = numberRe.match(text, offset)).hasMatch()) {
        const qsizetype start = match.capturedStart();
        const qsizetype length = match.capturedLength();
        const bool touchesAsciiLetters =
            (start > 0 && text.at(start - 1).isLetter() && text.at(start - 1).unicode() < 128) ||
            (start + length < text.size() &&
             text.at(start + length).isLetter() &&
             text.at(start + length).unicode() < 128);
        if (touchesAsciiLetters) {
            offset = start + length;
            continue;
        }

        const QString replacement = numericTokenToVietnameseWords(match.captured(0));
        QString spacedReplacement = replacement;
        if (start > 0 && text.at(start - 1).isLetterOrNumber()) {
            spacedReplacement.prepend(QStringLiteral(" "));
        }
        if (start + length < text.size() &&
            text.at(start + length).isLetterOrNumber()) {
            spacedReplacement.append(QStringLiteral(" "));
        }
        text.replace(start, length, spacedReplacement);
        offset = start + spacedReplacement.size();
    }

    text.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    return text.trimmed();
}

} // namespace

AsrManager::AsrManager(const Config& cfg)
    : cfg_(cfg),
      sherpa_(),
      segmenter_(cfg.segmenter_cfg),
      tts_(cfg.tts_cfg),
      piper_tts_(cfg.piper_tts_cfg),
      phobertClassifier_([&cfg]() {
          auto classifierCfg = cfg.phobert_cfg;
          classifierCfg.enabled = cfg.enable_phobert_classifier;
          return classifierCfg;
      }()),
      llmQueryService_([&cfg]() {
          auto llmCfg = cfg.llm_cfg;
          llmCfg.enabled = cfg.enable_llm_query;
          return llmCfg;
      }()),
      hvacService_()
{
}

AsrManager::~AsrManager()
{
    stopReplyWorker();
}

bool AsrManager::initialize(QString* errorMessage)
{
    reset();

    if (!segmenter_.initialize(errorMessage)) {
        return false;
    }

    SherpaOnnxOfflineEngine::Config sherpaCfg;
    sherpaCfg.encoder_path = cfg_.sherpa_encoder_path;
    sherpaCfg.decoder_path = cfg_.sherpa_decoder_path;
    sherpaCfg.joiner_path  = cfg_.sherpa_joiner_path;
    sherpaCfg.tokens_path  = cfg_.sherpa_tokens_path;
    sherpaCfg.num_threads  = cfg_.sherpa_threads;
    sherpaCfg.provider     = cfg_.sherpa_provider;
    sherpaCfg.debug        = cfg_.sherpa_debug;

    if (!sherpa_.initialize(sherpaCfg, errorMessage)) {
        return false;
    }

    if (cfg_.enable_nlu) {
        if (cfg_.enable_phobert_classifier) {
            QString classifierError;
            if (!phobertClassifier_.initialize(&classifierError)) {
                if (errorMessage) {
                    *errorMessage = QStringLiteral("PhoBERT classifier init failed: %1")
                                        .arg(classifierError);
                }
                return false;
            }

            std::cout << "[Assistant] PhoBERT multi-head classifier enabled\n";
        }

        std::cout << "[Assistant] PhoBERT task NLU ready\n";
    }

    if (cfg_.enable_llm_query) {
        QString llmError;
        if (!llmQueryService_.isReady(&llmError)) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("LLM query service init failed: %1").arg(llmError);
            }
            return false;
        }

        std::cout << "[Assistant] LLM query service enabled\n";
    }

    tts_available_.store(false);
    if (cfg_.enable_tts_reply) {
        const AssistantTtsRuntimeSettings runtimeTts = AssistantTtsSettings::load();
        tts_.setRuntimeOptions(runtimeTts.speed, runtimeTts.volume, runtimeTts.voice);
        piper_tts_.setRuntimeOptions(runtimeTts.speed, runtimeTts.volume, runtimeTts.voice);

        if (tts_.initialize()) {
            tts_available_.store(true);
            std::cout << "[Assistant] VieNeu TTS ready\n";
        } else {
            std::cerr << "[Assistant] VieNeu TTS init failed";
            if (cfg_.require_tts_reply && runtimeTts.engine != "piper") {
                std::cerr << " and is required\n";
                if (errorMessage) {
                    *errorMessage = QStringLiteral("VieNeu TTS init failed");
                }
                return false;
            }
            std::cerr << "; continuing without spoken replies\n";
        }

        if (runtimeTts.engine == "piper") {
            tts_available_.store(true);
        }
    }

    startReplyWorker();
    return true;
}

void AsrManager::reset()
{
    segmenter_.resetForNewCommand();

    command_last_partial_.clear();
    command_last_final_.clear();
    command_last_error_.clear();
}

void AsrManager::setAudioLevelCallback(std::function<void(double)> callback)
{
    audioLevelCallback_ = std::move(callback);
}

void AsrManager::setTtsAudioLevelCallback(std::function<void(double)> callback)
{
    auto shared = std::make_shared<std::function<void(double)>>(std::move(callback));
    tts_.setAudioLevelCallback([shared](double level) {
        if (*shared) {
            (*shared)(level);
        }
    });
    piper_tts_.setAudioLevelCallback([shared](double level) {
        if (*shared) {
            (*shared)(level);
        }
    });
}

void AsrManager::setPlaybackStateCallback(std::function<void(bool)> callback)
{
    playbackStateCallback_ = std::move(callback);

    // AsrManager owns the assistant playback state. TTS backends may call
    // stop() internally before synthesis/playback; forwarding those backend
    // callbacks resets the wakeword state machine at the wrong time.
    tts_.setPlaybackStateCallback({});
    piper_tts_.setPlaybackStateCallback({});
}

void AsrManager::setConversationUpdateCallback(
    std::function<void(const QString&, const QString&, bool, double)> callback)
{
    conversationUpdateCallback_ = std::move(callback);

    tts_.setPlaybackProgressCallback([this](double progress) {
        if (conversationUpdateCallback_) {
            conversationUpdateCallback_({}, {}, false, progress);
        }
    });
    piper_tts_.setPlaybackProgressCallback([this](double progress) {
        if (conversationUpdateCallback_) {
            conversationUpdateCallback_({}, {}, false, progress);
        }
    });
}

void AsrManager::setPlaybackActive(bool active)
{
    playback_active_.store(active);
}

double AsrManager::calcRms16(const std::vector<int16_t>& chunk)
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

void AsrManager::feedCommandAudioChunk(const std::vector<int16_t>& chunk, bool* endOfUtterance)
{
    if (endOfUtterance) {
        *endOfUtterance = false;
    }

    if (audioLevelCallback_) {
        const double rms16 = calcRms16(chunk);
        const double normalized = std::clamp(rms16 / 16000.0, 0.0, 1.0);
        audioLevelCallback_(normalized);
    }

    CommandSegmenter::Output out;
    segmenter_.pushChunk(chunk, &out);

    if (!out.utterance_finalized) {
        if (out.partial_ready && !out.partial_window_f32.empty()) {
            QString partialError;
            const QString partialText = sherpa_.transcribeUtterance(
                out.partial_window_f32,
                static_cast<int>(cfg_.sample_rate),
                &partialError
            ).trimmed();
            if (!partialText.isEmpty()) {
                if (command_last_partial_.isEmpty() ||
                    partialText.size() >= command_last_partial_.size()) {
                    command_last_partial_ = partialText;
                } else if (!command_last_partial_.contains(partialText)) {
                    command_last_partial_ += QStringLiteral(" ");
                    command_last_partial_ += partialText;
                }
                if (conversationUpdateCallback_) {
                    conversationUpdateCallback_(command_last_partial_, {}, false, 0.0);
                }
            }
        }
        return;
    }

    if (!out.final_raw_pcm16.empty() && !cfg_.command_wav_path.empty()) {
        dumpCommandToWav(out.final_raw_pcm16, cfg_.command_wav_path);
    }

    if (cfg_.dump_cleaned_command_wav &&
        !out.final_clean_pcm16.empty() &&
        !cfg_.cleaned_command_wav_path.empty()) {
        dumpCommandToWav(out.final_clean_pcm16, cfg_.cleaned_command_wav_path);
    }

    const double secs =
        static_cast<double>(out.final_utterance_f32.size()) /
        static_cast<double>(std::max(1u, cfg_.sample_rate));

    std::cout << "[ASR] Finalize whole utterance, reason="
              << out.finalize_reason.toStdString()
              << " samples=" << out.final_utterance_f32.size()
              << " seconds=" << secs
              << "\n";

    if (out.final_utterance_f32.empty()) {
        command_last_error_ = QStringLiteral("Final utterance is empty");

        if (endOfUtterance) {
            *endOfUtterance = true;
        }

        return;
    }

    QString error;

    const QString finalText = sherpa_.transcribeUtterance(
        out.final_utterance_f32,
        static_cast<int>(cfg_.sample_rate),
        &error
    ).trimmed();

    if (!error.isEmpty()) {
        command_last_error_ = error;

        std::cerr << "[ASR] Final Sherpa error: "
                  << error.toStdString()
                  << "\n";
    } else {
        command_last_final_ = finalText;
        if (conversationUpdateCallback_) {
            conversationUpdateCallback_(finalText, {}, true, 0.0);
        }
    }

    if (endOfUtterance) {
        *endOfUtterance = true;
    }
}

void AsrManager::dumpCommandToWav(const std::vector<int16_t>& pcm16, const std::string& path)
{
    std::ofstream wav(path, std::ios::binary);

    if (!wav) {
        std::cerr << "[Command] Cannot open wav file: " << path << "\n";
        return;
    }

    const uint32_t dataBytes = static_cast<uint32_t>(pcm16.size() * sizeof(int16_t));

    writeWavHeader(
        wav,
        cfg_.sample_rate,
        static_cast<uint16_t>(cfg_.channels),
        16,
        dataBytes
    );

    wav.write(
        reinterpret_cast<const char*>(pcm16.data()),
        static_cast<std::streamsize>(dataBytes)
    );

    const double seconds =
        static_cast<double>(pcm16.size()) /
        static_cast<double>(std::max(1u, cfg_.sample_rate));

    std::cout << "[Command] Dumped wav: " << path
              << " samples=" << pcm16.size()
              << " seconds=" << seconds
              << "\n";
}

QString AsrManager::cleanupReplyText(const QString& text) const
{
    QString out = cutAtStopMarkers(text).trimmed();

    if (cfg_.reply_max_chars > 0 && out.size() > cfg_.reply_max_chars) {
        QString limited = out.left(cfg_.reply_max_chars).trimmed();
        const int lastSentence = std::max({
            limited.lastIndexOf(QStringLiteral(".")),
            limited.lastIndexOf(QStringLiteral("!")),
            limited.lastIndexOf(QStringLiteral("?"))
        });
        if (lastSentence >= cfg_.reply_max_chars / 2) {
            limited = limited.left(lastSentence + 1).trimmed();
        }
        std::cout << "[Assistant] Reply truncated for TTS chars="
                  << out.size()
                  << " -> "
                  << limited.size()
                  << "\n";
        out = limited;
    }

    return out;
}

void AsrManager::handleFinalCommandText(const QString& text)
{
    const QString normalized = text.trimmed();

    if (normalized.isEmpty()) {
        return;
    }

    if (conversationUpdateCallback_) {
        conversationUpdateCallback_(normalized, {}, true, 0.0);
    }

    interrupt_requested_.store(false);

    const uint64_t taskEpoch = epoch_.fetch_add(1) + 1;

    {
        std::lock_guard<std::mutex> lock(reply_mutex_);
        reply_queue_.push_back(ReplyTask{taskEpoch, normalized});
    }

    reply_cv_.notify_one();
}

void AsrManager::startReplyWorker()
{
    stopReplyWorker();

    stop_reply_worker_ = false;

    reply_thread_ = std::thread([this]() {
        this->replyWorkerLoop();
    });
}

void AsrManager::stopReplyWorker()
{
    {
        std::lock_guard<std::mutex> lock(reply_mutex_);
        stop_reply_worker_ = true;
        reply_queue_.clear();
    }

    reply_cv_.notify_all();

    interrupt_requested_.store(true);
    tts_.stop();
    piper_tts_.stop();

    if (reply_thread_.joinable()) {
        reply_thread_.join();
    }

    reply_busy_.store(false);
    setPlaybackActive(false);
}

nlu::VehicleContext AsrManager::currentVehicleContext() const
{
    nlu::VehicleContext ctx;

    // Hiện tại dùng context tĩnh từ config.
    // Sau này nối VehicleSignals/Kuksa tại đây.
    ctx.acOn = cfg_.initial_ac_on;
    ctx.cabinTemp = cfg_.initial_cabin_temp;
    ctx.targetTemp = cfg_.initial_target_temp;
    ctx.fanLevel = cfg_.initial_fan_level;

    return ctx;
}

nlu::ResolvedAction AsrManager::resolveWithPhoBert(const QString& text,
                                                   const nlu::VehicleContext& /*context*/)
{
    const auto prediction = phobertClassifier_.predict(text);
    if (!prediction.ok) {
        std::cout << "[PhoBERT] prediction failed: "
                  << prediction.error.toStdString()
                  << "\n";
        return {};
    }

    std::cout << "[PhoBERT] action=" << prediction.action.label.toStdString()
              << " domain=" << prediction.domain.label.toStdString()
              << " op=" << prediction.op.label.toStdString()
              << " target=" << prediction.target.label.toStdString()
              << " execute=" << prediction.execute.label.toStdString()
              << " slot=" << prediction.slotType.label.toStdString()
              << " confidence=" << prediction.action.confidence
              << "\n";

    const QString action = nlu::PhoBertExecutionPolicy::normalizeModelAction(
        prediction.action.label,
        prediction.target.label,
        prediction.slotType.label
    );
    const nlu::SlotMap extractedSlots = slotExtractor_.extract({
        text,
        prediction.normalizedText,
        action,
        prediction.action.label,
        prediction.slotType.label,
        prediction.target.label
    });

    std::cout << "[PhoBERT] resolved_action=" << action.toStdString()
              << " slotValues={";
    bool firstSlot = true;
    for (auto it = extractedSlots.constBegin(); it != extractedSlots.constEnd(); ++it) {
        if (!firstSlot) std::cout << ", ";
        firstSlot = false;
        std::cout << it.key().toStdString()
                  << "="
                  << it.value().toString().toStdString();
    }
    std::cout << "}\n";

    return executionPolicy_.apply({
        prediction.action.label,
        prediction.normalizedText,
        prediction.domain.label,
        prediction.op.label,
        prediction.target.label,
        prediction.execute.label,
        prediction.slotType.label,
        prediction.action.confidence,
        action,
        extractedSlots
    });
}

QString AsrManager::executeResolvedAction(const QString& action,
                                          const nlu::SlotMap& slotValues,
                                          const QString& unsupportedPrefix)
{
    if (action.startsWith(QStringLiteral("HVAC_"))) {
        const bool ok = hvacService_.executeAction(action, slotValues);
        if (!ok) return QStringLiteral("Tôi chưa thực thi được lệnh điều hòa này.");
        return {};
    }
    if (action.startsWith(QStringLiteral("MEDIA_"))) {
        const bool ok = mediaService_.executeAction(action, slotValues);
        if (!ok) return QStringLiteral("Tôi chưa thực thi được lệnh media này.");
        return {};
    }
    if (action.startsWith(QStringLiteral("PHONE_"))) {
        const bool ok = phoneService_.executeAction(action, slotValues);
        if (!ok) return QStringLiteral("Tôi chưa thực thi được lệnh điện thoại này.");
        return {};
    }
    if (action.startsWith(QStringLiteral("NAVIGATION_"))) {
        const bool ok = navigationService_.executeAction(action, slotValues);
        if (!ok) return QStringLiteral("Tôi chưa thực thi được lệnh dẫn đường này.");
        return {};
    }
    if (action.startsWith(QStringLiteral("VEHICLE_"))) {
        const bool ok = vehicleService_.executeAction(action, slotValues);
        if (!ok) return QStringLiteral("Tôi chưa thực thi được lệnh kiểm tra xe này.");
        return {};
    }

    std::cout << "[Assistant] " << unsupportedPrefix.toStdString()
              << ": " << action.toStdString()
              << "\n";
    return QStringLiteral("Tôi chưa hỗ trợ hành động này.");
}

void AsrManager::launchUiForAction(const QString& action)
{
    const UiTarget target = uiTargetForAction(action);
    if (target.appId.isEmpty()) {
        return;
    }

    const bool ok = appLauncher_.startApplication(target.appId);
    if (!ok) {
        std::cerr << "[Assistant] Failed to launch UI app "
                  << target.appId.toStdString()
                  << " for action "
                  << action.toStdString()
                  << "\n";
    }

    requestHomescreenShowApp(target.appId);

    std::cout << "[Assistant] Requested UI app "
              << target.appId.toStdString()
              << " for action "
              << action.toStdString()
              << "\n";
}

bool AsrManager::waitForUiBackendForAction(const QString& action, int timeoutMs) const
{
    const UiTarget target = uiTargetForAction(action);
    if (target.dbusService.isEmpty()) {
        return true;
    }

    if (isDbusServiceRegistered(target.dbusService)) {
        return true;
    }

    QElapsedTimer timer;
    timer.start();

    while (!interrupt_requested_.load() && timer.elapsed() < timeoutMs) {
        QThread::msleep(100);
        if (isDbusServiceRegistered(target.dbusService)) {
            return true;
        }
    }

    std::cerr << "[Assistant] UI backend D-Bus service is not ready: "
              << target.dbusService.toStdString()
              << " for action "
              << action.toStdString()
              << "\n";
    return false;
}

QString AsrManager::processNluAndPrepare(const QString& text,
                                         QString* deferredAction,
                                         nlu::SlotMap* deferredSlotValues,
                                         QString* unsupportedPrefix)
{
    if (deferredAction) deferredAction->clear();
    if (deferredSlotValues) deferredSlotValues->clear();
    if (unsupportedPrefix) unsupportedPrefix->clear();

    if (!cfg_.enable_nlu) {
        return QStringLiteral("NLU chưa được bật.");
    }

    const nlu::VehicleContext ctx = currentVehicleContext();

    std::cout << "[Assistant] User: " << text.toStdString() << "\n";

    if (cfg_.enable_phobert_classifier) {
        const nlu::ResolvedAction sem = resolveWithPhoBert(text, ctx);
        if (sem.matched) {
            if (sem.decision == QStringLiteral("LLM_QUERY")) {
                QString llmError;
                const QString llmReply = llmQueryService_.answer({
                    text,
                    sem.normalizedText,
                    sem.domain,
                    sem.op,
                    sem.target,
                    sem.slotType,
                    sem.slotValues
                }, &llmError);

                if (!llmReply.trimmed().isEmpty()) {
                    return llmReply;
                }

                std::cout << "[LLM] query failed: "
                          << llmError.toStdString()
                          << "\n";
                if (llmError.contains(QStringLiteral("Host "), Qt::CaseInsensitive) ||
                    llmError.contains(QStringLiteral("network"), Qt::CaseInsensitive) ||
                    llmError.contains(QStringLiteral("timeout"), Qt::CaseInsensitive)) {
                    return QStringLiteral("Hiện tại xe chưa kết nối được mạng, nên tôi chưa tra cứu được câu này.");
                }
                return QStringLiteral("Tôi chưa trả lời được câu này.");
            }

            if (sem.shouldExecute && !sem.action.isEmpty()) {
                if (deferredAction) *deferredAction = sem.action;
                if (deferredSlotValues) *deferredSlotValues = sem.slotValues;
                if (unsupportedPrefix) {
                    *unsupportedPrefix = QStringLiteral("Unsupported PhoBERT action");
                }
            }
            return sem.reply;
        }
    }

    return QStringLiteral("Tôi chưa rõ lệnh này, bạn nói lại cụ thể hơn giúp tôi.");
}

void AsrManager::replyWorkerLoop()
{
    for (;;) {
        ReplyTask task;

        {
            std::unique_lock<std::mutex> lock(reply_mutex_);

            reply_cv_.wait(lock, [this]() {
                return stop_reply_worker_ || !reply_queue_.empty();
            });

            if (stop_reply_worker_) {
                break;
            }

            task = reply_queue_.front();
            reply_queue_.pop_front();
        }

        if (task.text.trimmed().isEmpty()) {
            continue;
        }

        if (task.epoch != epoch_.load()) {
            continue;
        }

        reply_busy_.store(true);

        if (conversationUpdateCallback_) {
            conversationUpdateCallback_(task.text, {}, true, 0.0);
        }

        QString deferredAction;
        nlu::SlotMap deferredSlotValues;
        QString unsupportedPrefix;
        QString reply = processNluAndPrepare(task.text,
                                             &deferredAction,
                                             &deferredSlotValues,
                                             &unsupportedPrefix);

        if (interrupt_requested_.load() || task.epoch != epoch_.load()) {
            std::cout << "[Assistant] Reply interrupted before execute\n";
            reply_busy_.store(false);
            continue;
        }

        if (!deferredAction.isEmpty()) {
            launchUiForAction(deferredAction);

            if (!interrupt_requested_.load() && task.epoch == epoch_.load()) {
                waitForUiBackendForAction(deferredAction, 2500);

                const QString errorReply = executeResolvedAction(
                    deferredAction,
                    deferredSlotValues,
                    unsupportedPrefix.isEmpty()
                        ? QStringLiteral("Unsupported deferred action")
                        : unsupportedPrefix
                );

                if (!errorReply.isEmpty()) {
                    std::cerr << "[Assistant] deferred execute failed: "
                              << errorReply.toStdString()
                              << "\n";
                    reply = errorReply;
                }
            }

            if (interrupt_requested_.load() || task.epoch != epoch_.load()) {
                std::cout << "[Assistant] Reply interrupted after execute\n";
                reply_busy_.store(false);
                setPlaybackActive(false);
                continue;
            }

            if (shouldSkipTtsForAction(deferredAction)) {
                std::cout << "[Assistant] Skip TTS for action "
                          << deferredAction.toStdString()
                          << "\n";

                if (conversationUpdateCallback_) {
                    conversationUpdateCallback_(task.text, {}, false, 1.0);
                }

                reply_busy_.store(false);
                setPlaybackActive(false);

                if (playbackStateCallback_) {
                    playbackStateCallback_(false);
                }
                continue;
            }
        }

        reply = cleanupReplyText(reply);

        if (reply.isEmpty()) {
            std::cout << "[Assistant] Empty reply; skip TTS\n";

            if (conversationUpdateCallback_) {
                conversationUpdateCallback_(task.text, {}, false, 1.0);
            }

            reply_busy_.store(false);
            setPlaybackActive(false);

            if (playbackStateCallback_) {
                playbackStateCallback_(false);
            }
            continue;
        }

        std::cout << "[Assistant] Reply: " << reply.toStdString() << "\n";

        if (conversationUpdateCallback_) {
            conversationUpdateCallback_(task.text, reply, false, 0.0);
        }

        if (cfg_.enable_tts_reply && !reply.isEmpty()) {
            const AssistantTtsRuntimeSettings runtimeTts = AssistantTtsSettings::load();
            setPlaybackActive(true);

            if (playbackStateCallback_) {
                playbackStateCallback_(true);
            }

            const QString ttsText = textForTts(reply);
            if (ttsText != reply) {
                std::cout << "[Assistant] TTS text: " << ttsText.toStdString() << "\n";
            }

            bool ok = false;
            const std::string speakText = ttsText.toStdString();
            if (runtimeTts.engine == "piper") {
                piper_tts_.setRuntimeOptions(
                    runtimeTts.speed,
                    runtimeTts.volume,
                    runtimeTts.voice);
                ok = piper_tts_.speak(speakText);
            } else {
                tts_.setRuntimeOptions(
                    runtimeTts.speed,
                    runtimeTts.volume,
                    runtimeTts.voice);
                ok = tts_.speak(speakText);
            }

            if (interrupt_requested_.load() || task.epoch != epoch_.load()) {
                std::cout << "[Assistant] TTS interrupted by user speech\n";
                reply_busy_.store(false);
                setPlaybackActive(false);
                continue;
            }

            if (!ok) {
                std::cerr << "[Assistant] TTS failed\n";
            }
        }

        reply_busy_.store(false);
        setPlaybackActive(false);

        if (playbackStateCallback_) {
            playbackStateCallback_(false);
        }
    }
}

void AsrManager::interruptAllAndPrepareForNewCommand()
{
    interrupt_requested_.store(true);
    epoch_.fetch_add(1);

    {
        std::lock_guard<std::mutex> lock(reply_mutex_);
        reply_queue_.clear();
    }

    tts_.stop();
    piper_tts_.stop();
    reply_busy_.store(false);
    setPlaybackActive(false);

    segmenter_.resetForNewCommand();
    command_last_partial_.clear();
    command_last_final_.clear();
    command_last_error_.clear();
}

QString AsrManager::lastPartialText() const
{
    return command_last_partial_;
}

QString AsrManager::lastFinalText() const
{
    return command_last_final_;
}

QString AsrManager::lastError() const
{
    return command_last_error_;
}

bool AsrManager::isPlaybackActive() const
{
    return playback_active_.load();
}

bool AsrManager::isBusyReplying() const
{
    return reply_busy_.load() || playback_active_.load();
}
