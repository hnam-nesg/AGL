#pragma once

#include <string>

struct VieNeuTtsConfig {
    std::string model_path = "/opt/vieneu-gguf/models/vieneu-q4/VieNeu-TTS-0_3B-Q4_0.gguf";
    std::string voices_path = "/opt/vieneu-gguf/models/vieneu-q4/voices.json";
    std::string codec_path = "/opt/vieneu-gguf/models/neucodec/model.onnx";
    std::string phoneme_dict_path = "/opt/vieneu-gguf/models/vieneu-q4/phoneme_dict.json";

    std::string voice = "Vinh";
    // Use "standard" for VieNeu-TTS-0.3B GGUF + integer voice codes.
    // Use "turbo" only with VieNeu v2/Turbo GGUF + 128-float voice embeddings.
    std::string prompt_mode = "standard";
    std::string output_wav_path = "/tmp/vieneu_tts.wav";

    int threads = 4;
    int batch_threads = 4;
    int codec_threads = 2;
    int ctx = 2048;
    int batch = 512;
    int ubatch = 512;
    int max_tokens = 2048;
    int min_generation_tokens = 160;
    int generation_tokens_per_char = 7;
    int generation_tokens_extra = 160;
    int top_k = 50;
    float top_p = 0.95f;
    float min_p = 0.05f;
    float temperature = 1.0f;
    int n_gpu_layers = 0;

    bool use_mmap = true;
    bool use_mlock = true;
    bool warmup = false;
    bool debug_prompt = false;
    bool dump_wav = false;
    bool require_phoneme_dict = true;
    bool allow_raw_phoneme_fallback = false;
    int max_unknown_phoneme_preview = 12;
    int min_speech_tokens = 8;
    int min_audio_ms = 500;
    int min_chunk_chars = 45;
    int target_chunk_chars = 150;
    int max_chunk_chars = 220;
    bool enable_audio_cache = true;
    std::string audio_preset_dir = "/usr/share/assistant/vieneu_tts_cache";
    std::string audio_cache_dir = "/tmp/vieneu_tts_cache";
    float playback_speed = 1.0f;
    float playback_volume = 1.0f;
    bool trim_audio_edges = false;
    int silence_trim_threshold = 192;
    int silence_keep_ms = 20;
    int edge_fade_ms = 0;
    std::string warmup_text = "Xin chao";

    std::string alsa_playback_device = "default";
    unsigned int playback_chunk_frames = 1024;
};
