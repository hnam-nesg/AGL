#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QByteArray>
#include <QUrl>
#include <QCoreApplication>
#include <QColor>

#include <iostream>

#include "settings/AssistantUiSettings.h"
#include "wakeword/WakeWordTester.h"
#include "ThemeSettingsManager.h"

int main(int argc, char *argv[])
{
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);

    QQuickWindow::setDefaultAlphaBuffer(true);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    qputenv("QT_MEDIA_BACKEND", QByteArray("alsa"));
    qputenv("QT_QUICK_CONTROLS_STYLE", QByteArray("Basic"));
    qputenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", QByteArray("1"));

    QGuiApplication app(argc, argv);
    app.setDesktopFileName("assistant");

    QQmlApplicationEngine engine;

    WakeWordTest::Config cfg;

    // ============================================================
    // Audio / Wakeword config
    // ============================================================

    cfg.alsa_device = "plughw:2,0";

    cfg.mel_model_path = "/usr/share/openwakeword/models/melspectrogram.onnx";
    cfg.embed_model_path = "/usr/share/openwakeword/models/embedding_model.onnx";
    cfg.wakeword_model_path = "/usr/share/openwakeword/models/hây_au_to.onnx";

    cfg.sample_rate = 16000;
    cfg.channels = 1;
    cfg.format = SND_PCM_FORMAT_S16_LE;

    cfg.wakeword_chunk_samples = 1280;
    cfg.wakeword_threshold = 0.3f;
    cfg.wakeword_step_frames = 2;
    cfg.wakeword_trigger_level = 2;
    cfg.wakeword_refractory = 20;
    cfg.startup_ignore_steps = 20;
    cfg.alsa_buffer_periods = 4;
    cfg.debug = false;

    // ============================================================
    // ASR base config
    // ============================================================

    cfg.command_asr_cfg.sample_rate = 16000;
    cfg.command_asr_cfg.channels = 1;

    cfg.command_asr_cfg.sherpa_encoder_path =
        "/usr/share/zipformer-hynt/models/encoder-epoch-20-avg-10.int8.onnx";
    cfg.command_asr_cfg.sherpa_decoder_path =
        "/usr/share/zipformer-hynt/models/decoder-epoch-20-avg-10.int8.onnx";
    cfg.command_asr_cfg.sherpa_joiner_path =
        "/usr/share/zipformer-hynt/models/joiner-epoch-20-avg-10.int8.onnx";
    cfg.command_asr_cfg.sherpa_tokens_path =
        "/usr/share/zipformer-hynt/models/tokens.txt";

    cfg.command_asr_cfg.sherpa_threads = 4;
    cfg.command_asr_cfg.sherpa_provider = "cpu";
    cfg.command_asr_cfg.sherpa_debug = false;

    cfg.command_asr_cfg.command_wav_path = "/tmp/command.wav";
    cfg.command_asr_cfg.dump_cleaned_command_wav = true;
    cfg.command_asr_cfg.cleaned_command_wav_path = "/tmp/command_clean.wav";

    // ============================================================
    // Command segmenter config
    // ============================================================

    cfg.command_asr_cfg.segmenter_cfg.sample_rate = 16000;

    cfg.command_asr_cfg.segmenter_cfg.enable_high_pass = false;
    cfg.command_asr_cfg.segmenter_cfg.high_pass_alpha = 0.995f;
    cfg.command_asr_cfg.segmenter_cfg.enable_rnnoise = false;

    cfg.command_asr_cfg.segmenter_cfg.preroll_chunks = 2;

    cfg.command_asr_cfg.segmenter_cfg.no_speech_timeout_ms = 5000;
    cfg.command_asr_cfg.segmenter_cfg.force_final_after_ms = 15000;
    cfg.command_asr_cfg.segmenter_cfg.min_final_audio_ms = 3000;

    cfg.command_asr_cfg.segmenter_cfg.partial_window_ms = 3000;
    cfg.command_asr_cfg.segmenter_cfg.partial_update_interval_ms = 1200;

    cfg.command_asr_cfg.segmenter_cfg.fallback_start_speech_rms_threshold = 0.05f;

    cfg.command_asr_cfg.segmenter_cfg.enable_silero_vad = true;
    cfg.command_asr_cfg.segmenter_cfg.debug = false;

    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.model_path =
        "/usr/share/openwakeword/models/silero_vad.onnx";
    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.sample_rate = 16000;
    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.window_size_samples = 512;
    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.context_size_samples = 64;
    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.speech_threshold = 0.50f;
    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.silence_threshold = 0.10f;
    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.start_trigger_chunks = 2;
    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.end_trigger_chunks = 24;
    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.intra_threads = 1;
    cfg.command_asr_cfg.segmenter_cfg.silero_vad_cfg.debug = true;

    // ============================================================
    // PhoBERT multi-head task-oriented NLU config.
    // ============================================================

    cfg.command_asr_cfg.enable_nlu = true;

    cfg.command_asr_cfg.enable_phobert_classifier = true;
    cfg.command_asr_cfg.phobert_cfg.enabled = cfg.command_asr_cfg.enable_phobert_classifier;
    cfg.command_asr_cfg.phobert_cfg.model_path =
        "/usr/share/assistant/intent/phobert_multihead_int8.onnx";
    cfg.command_asr_cfg.phobert_cfg.vocab_path =
        "/usr/share/assistant/intent/vocab.txt";
    cfg.command_asr_cfg.phobert_cfg.bpe_codes_path =
        "/usr/share/assistant/intent/bpe.codes";
    cfg.command_asr_cfg.phobert_cfg.label_map_path =
        QStringLiteral("/usr/share/assistant/intent/multihead_label_maps.json");
    cfg.command_asr_cfg.phobert_cfg.max_seq_len = 64;
    cfg.command_asr_cfg.phobert_cfg.intra_threads = 2;
    cfg.command_asr_cfg.phobert_cfg.debug = true;

    // ============================================================
    // Groq LLM fallback config for GENERAL_QA / FALLBACK_LLM.
    // ============================================================

    cfg.command_asr_cfg.enable_llm_query = true;
    cfg.command_asr_cfg.llm_cfg.enabled = cfg.command_asr_cfg.enable_llm_query;
    cfg.command_asr_cfg.llm_cfg.endpoint_url =
        QStringLiteral("https://api.groq.com/openai/v1/chat/completions");
    cfg.command_asr_cfg.llm_cfg.model = QStringLiteral("openai/gpt-oss-120b");
    cfg.command_asr_cfg.llm_cfg.api_key = QStringLiteral("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");
    cfg.command_asr_cfg.llm_cfg.max_completion_tokens = 220;
    cfg.command_asr_cfg.llm_cfg.temperature = 0.2;
    cfg.command_asr_cfg.llm_cfg.reasoning_effort = QStringLiteral("low");
    cfg.command_asr_cfg.llm_cfg.include_reasoning = true;
    cfg.command_asr_cfg.llm_cfg.timeout_ms = 20000;

    cfg.command_asr_cfg.enable_tts_reply = true;
    cfg.command_asr_cfg.require_tts_reply = false;
    cfg.command_asr_cfg.reply_max_chars = 0;

    // HVAC context tạm thời.
    // Sau này đọc từ VehicleSignals / Kuksa / HVAC backend thật.
    cfg.command_asr_cfg.initial_ac_on = false;
    cfg.command_asr_cfg.initial_cabin_temp = 31;
    cfg.command_asr_cfg.initial_target_temp = -1;
    cfg.command_asr_cfg.initial_fan_level = 0;

    // ============================================================
    // VieNeu TTS config
    // ============================================================

    // Old Piper TTS config kept for reference; not used:
    // cfg.command_asr_cfg.tts_cfg.piper_path =
    //     "/usr/bin/piper";
    // cfg.command_asr_cfg.tts_cfg.model_path =
    //     "/usr/share/piper/models/vi_VN-vais1000-medium.onnx";
    // cfg.command_asr_cfg.tts_cfg.output_wav_path =
    //     "/tmp/assistant_reply.wav";
    // cfg.command_asr_cfg.tts_cfg.aplay_path =
    //     "/usr/bin/aplay";
    // cfg.command_asr_cfg.tts_cfg.output_sample_rate = 22050;
    // cfg.command_asr_cfg.tts_cfg.output_channels = 1;
    // cfg.command_asr_cfg.tts_cfg.length_scale = 1.0f;

    cfg.command_asr_cfg.tts_cfg.model_path =
        "/opt/vieneu-gguf/models/vieneu-q4/VieNeu-TTS-0_3B-Q4_0.gguf";
    cfg.command_asr_cfg.tts_cfg.voices_path =
        "/opt/vieneu-gguf/models/vieneu-q4/voices.json";
    cfg.command_asr_cfg.tts_cfg.codec_path =
        "/opt/vieneu-gguf/models/neucodec/model.onnx";
    cfg.command_asr_cfg.tts_cfg.phoneme_dict_path =
        "/opt/vieneu-gguf/models/vieneu-q4/phoneme_dict.json";
    cfg.command_asr_cfg.tts_cfg.voice =
        "Vinh";
    cfg.command_asr_cfg.tts_cfg.prompt_mode =
        "standard";
    cfg.command_asr_cfg.tts_cfg.output_wav_path =
        "/tmp/vieneu_tts.wav";

    cfg.command_asr_cfg.tts_cfg.debug_prompt = false;
    cfg.command_asr_cfg.tts_cfg.dump_wav = false;
    cfg.command_asr_cfg.tts_cfg.require_phoneme_dict = true;
    cfg.command_asr_cfg.tts_cfg.allow_raw_phoneme_fallback = false;
    cfg.command_asr_cfg.tts_cfg.min_speech_tokens = 8;
    cfg.command_asr_cfg.tts_cfg.min_audio_ms = 500;
    cfg.command_asr_cfg.tts_cfg.min_chunk_chars = 45;
    cfg.command_asr_cfg.tts_cfg.target_chunk_chars = 150;
    cfg.command_asr_cfg.tts_cfg.max_chunk_chars = 220;
    cfg.command_asr_cfg.tts_cfg.enable_audio_cache = true;
    cfg.command_asr_cfg.tts_cfg.audio_preset_dir = "/usr/share/assistant/vieneu_tts_cache";
    cfg.command_asr_cfg.tts_cfg.audio_cache_dir = "/tmp/vieneu_tts_cache";
    cfg.command_asr_cfg.tts_cfg.playback_speed = 1.0f;
    cfg.command_asr_cfg.tts_cfg.playback_volume = 1.0f;
    cfg.command_asr_cfg.tts_cfg.trim_audio_edges = false;
    cfg.command_asr_cfg.tts_cfg.silence_trim_threshold = 192;
    cfg.command_asr_cfg.tts_cfg.silence_keep_ms = 20;
    cfg.command_asr_cfg.tts_cfg.edge_fade_ms = 0;
    cfg.command_asr_cfg.tts_cfg.warmup = false;
    cfg.command_asr_cfg.tts_cfg.threads = 4;
    cfg.command_asr_cfg.tts_cfg.batch_threads = 4;
    cfg.command_asr_cfg.tts_cfg.codec_threads = 2;
    cfg.command_asr_cfg.tts_cfg.ctx = 2048;
    cfg.command_asr_cfg.tts_cfg.batch = 512;
    cfg.command_asr_cfg.tts_cfg.ubatch = 512;
    cfg.command_asr_cfg.tts_cfg.max_tokens = 2048;
    cfg.command_asr_cfg.tts_cfg.min_generation_tokens = 160;
    cfg.command_asr_cfg.tts_cfg.generation_tokens_per_char = 7;
    cfg.command_asr_cfg.tts_cfg.generation_tokens_extra = 160;
    cfg.command_asr_cfg.tts_cfg.top_k = 50;
    cfg.command_asr_cfg.tts_cfg.top_p = 0.95f;
    cfg.command_asr_cfg.tts_cfg.min_p = 0.05f;
    cfg.command_asr_cfg.tts_cfg.temperature = 1.0f;
    cfg.command_asr_cfg.tts_cfg.n_gpu_layers = 0;
    cfg.command_asr_cfg.tts_cfg.use_mmap = true;
    cfg.command_asr_cfg.tts_cfg.use_mlock = true;
    cfg.command_asr_cfg.tts_cfg.alsa_playback_device = "default";
    cfg.command_asr_cfg.tts_cfg.playback_chunk_frames = 1024;

    cfg.command_asr_cfg.piper_tts_cfg.piper_path = "/opt/piper/piper";
    cfg.command_asr_cfg.piper_tts_cfg.model_path =
        "/usr/share/piper/models/vi_VN-vais1000-medium.onnx";
    cfg.command_asr_cfg.piper_tts_cfg.config_path =
        "/usr/share/piper/models/vi_VN-vais1000-medium.onnx.json";
    cfg.command_asr_cfg.piper_tts_cfg.espeak_data_path =
        "/opt/piper/espeak-ng-data";
    cfg.command_asr_cfg.piper_tts_cfg.tashkeel_model_path =
        "/opt/piper/libtashkeel_model.ort";
    cfg.command_asr_cfg.piper_tts_cfg.output_wav_path = "/tmp/assistant_piper_tts.wav";
    cfg.command_asr_cfg.piper_tts_cfg.length_scale = 1.0f;
    cfg.command_asr_cfg.piper_tts_cfg.noise_scale = 0.667f;
    cfg.command_asr_cfg.piper_tts_cfg.noise_w = 0.8f;
    cfg.command_asr_cfg.piper_tts_cfg.alsa_playback_device =
        cfg.command_asr_cfg.tts_cfg.alsa_playback_device;
    cfg.command_asr_cfg.piper_tts_cfg.playback_chunk_frames =
        cfg.command_asr_cfg.tts_cfg.playback_chunk_frames;

    // ============================================================
    // Start app
    // ============================================================

    auto* wakeword = new WakeWordTest(cfg);
    AssistantUiSettings assistantUiSettings;
    ThemeSettingsManager themeSettings;

    if (!wakeword->initialize()) {
        std::cerr << "[main] WakeWordTest initialize failed\n";
        delete wakeword;
        return -1;
    }

    engine.rootContext()->setContextProperty("wakeword", wakeword);
    engine.rootContext()->setContextProperty("assistantUiSettings", &assistantUiSettings);
    engine.rootContext()->setContextProperty("themeSettings", &themeSettings);
    engine.load(QUrl(QStringLiteral("qrc:/Assistant.qml")));

    if (engine.rootObjects().isEmpty()) {
        std::cerr << "[main] Failed to load Assistant.qml\n";
        delete wakeword;
        return -1;
    }

    for (QObject* rootObject : engine.rootObjects()) {
        if (auto* window = qobject_cast<QQuickWindow*>(rootObject))
            window->setColor(QColor(0, 0, 0, 0));
    }

    if (!wakeword->start()) {
        std::cerr << "[main] WakeWordTest start failed\n";
        delete wakeword;
        return -1;
    }

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [wakeword]() {
        wakeword->stop();
    });

    const int ret = app.exec();

    delete wakeword;
    return ret;
}
