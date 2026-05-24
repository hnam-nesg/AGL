#include "openwakeword_engine.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace {

Ort::SessionOptions makeSessionOptions() {
    Ort::SessionOptions opts;
    opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    opts.SetIntraOpNumThreads(1);
    opts.SetInterOpNumThreads(1);
    return opts;
}

} // namespace

OpenWakeWordEngine::OpenWakeWordEngine(const std::string& mel_model_path,
                                       const std::string& embed_model_path,
                                       const std::string& ww_model_path)
    : env_(ORT_LOGGING_LEVEL_WARNING, "OpenWakeWord"),
      memory_info_(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)),
      mel_model_path_(mel_model_path),
      embed_model_path_(embed_model_path),
      ww_model_path_(ww_model_path) {
}

OpenWakeWordEngine::~OpenWakeWordEngine() = default;

bool OpenWakeWordEngine::initialize() {
    try {
        mel_session_ = std::make_unique<Ort::Session>(
            env_, mel_model_path_.c_str(), makeSessionOptions());

        embed_session_ = std::make_unique<Ort::Session>(
            env_, embed_model_path_.c_str(), makeSessionOptions());

        ww_session_ = std::make_unique<Ort::Session>(
            env_, ww_model_path_.c_str(), makeSessionOptions());

        std::cout << "[OWW] Engine initialized successfully." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[OWW] initialize() failed: " << e.what() << std::endl;
        return false;
    }
}

void OpenWakeWordEngine::reset() {
    sample_buffer_.clear();
    mel_buffer_.clear();
    embedding_buffer_.clear();
    last_score_ = 0.0f;
    activation_ = 0;
}

void OpenWakeWordEngine::setDebugStep(int step, bool enabled) {
    debug_step_ = step;
    debug_enabled_ = enabled;
}

std::string OpenWakeWordEngine::shapeToString(const std::vector<int64_t>& shape) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < shape.size(); ++i) {
        if (i) {
            oss << ", ";
        }
        oss << shape[i];
    }
    oss << "]";
    return oss.str();
}

void OpenWakeWordEngine::printFloatHead(const std::string& tag,
                                        const std::vector<float>& data,
                                        size_t n) {
    std::cout << tag;
    const size_t limit = std::min(n, data.size());
    for (size_t i = 0; i < limit; ++i) {
        if (i) {
            std::cout << ", ";
        }
        std::cout << std::fixed << std::setprecision(6) << data[i];
    }
    std::cout << std::endl;
}

bool OpenWakeWordEngine::processAudioChunk(const std::vector<int16_t>& audio_data) {
    if (audio_data.empty()) {
        return false;
    }

    if (!mel_session_ || !embed_session_ || !ww_session_) {
        std::cerr << "[OWW] processAudioChunk called before initialize()." << std::endl;
        return false;
    }

    // GitHub file: keep PCM16 scale, DO NOT normalize to [-1, 1]
    for (int16_t s : audio_data) {
        sample_buffer_.push_back(static_cast<float>(s));
    }

    bool detected = false;
    const size_t frame_size = step_frames_ * CHUNK_SAMPLES;

    try {
        // audio -> mels
        while (sample_buffer_.size() >= frame_size) {
            std::vector<float> pcm_frame(sample_buffer_.begin(),
                                         sample_buffer_.begin() + static_cast<std::ptrdiff_t>(frame_size));
            sample_buffer_.erase(sample_buffer_.begin(),
                                 sample_buffer_.begin() + static_cast<std::ptrdiff_t>(frame_size));

            if (debug_enabled_) {
                printFloatHead("[OWW] pcmFrame head: ", pcm_frame, 10);
            }

            std::vector<float> mel_features = runMelSpectrogram(pcm_frame);
            if (!mel_features.empty()) {
                mel_buffer_.insert(mel_buffer_.end(), mel_features.begin(), mel_features.end());
            }

            if (debug_enabled_) {
                std::cout << "[OWW] mel_buffer frames = "
                          << (mel_buffer_.size() / NUM_MELS) << std::endl;
            }

            // mels -> embeddings (sliding by EMB_STEP_SIZE = 8 mel frames)
            size_t mel_frames = mel_buffer_.size() / NUM_MELS;
            while (mel_frames >= EMB_WINDOW_SIZE) {
                std::vector<float> mel_window(
                    mel_buffer_.begin(),
                    mel_buffer_.begin() + static_cast<std::ptrdiff_t>(EMB_WINDOW_SIZE * NUM_MELS));

                if (debug_enabled_) {
                    printFloatHead("[OWW] melWindow head: ", mel_window, 10);
                }

                std::vector<float> emb = runEmbedding(mel_window);
                if (!emb.empty()) {
                    embedding_buffer_.insert(embedding_buffer_.end(), emb.begin(), emb.end());
                }

                // Trượt 8 mel frames như file GitHub
                mel_buffer_.erase(
                    mel_buffer_.begin(),
                    mel_buffer_.begin() + static_cast<std::ptrdiff_t>(EMB_STEP_SIZE * NUM_MELS)
                );

                mel_frames = mel_buffer_.size() / NUM_MELS;
            }

            if (debug_enabled_) {
                std::cout << "[OWW] embedding_buffer frames = "
                          << (embedding_buffer_.size() / EMB_FEATURES) << std::endl;
            }

            // embeddings -> wakeword (sliding by 1 embedding)
            size_t embed_frames = embedding_buffer_.size() / EMB_FEATURES;
            while (embed_frames >= WW_FEATURES) {
                std::vector<float> ww_input(
                    embedding_buffer_.begin(),
                    embedding_buffer_.begin() + static_cast<std::ptrdiff_t>(WW_FEATURES * EMB_FEATURES));

                if (debug_enabled_) {
                    printFloatHead("[OWW] wwInput head: ", ww_input, 10);
                }

                last_score_ = runWakeWord(ww_input);

                if (debug_enabled_) {
                    std::cout << "[OWW] probability = " << last_score_
                              << " activation = " << activation_ << std::endl;
                }

                if (last_score_ > threshold_) {
                    ++activation_;
                    if (activation_ >= trigger_level_) {
                        detected = true;
                        activation_ = -refractory_;
                    }
                } else {
                    if (activation_ > 0) {
                        activation_ = std::max(0, activation_ - 1);
                    } else {
                        activation_ = std::min(0, activation_ + 1);
                    }
                }

                // Trượt 1 embedding như file GitHub
                embedding_buffer_.erase(
                    embedding_buffer_.begin(),
                    embedding_buffer_.begin() + static_cast<std::ptrdiff_t>(EMB_FEATURES)
                );

                embed_frames = embedding_buffer_.size() / EMB_FEATURES;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[OWW] Error: " << e.what() << std::endl;
        reset();
        return false;
    }

    return detected;
}

std::vector<float> OpenWakeWordEngine::runMelSpectrogram(const std::vector<float>& pcm_frame) {
    if (!mel_session_) {
        throw std::runtime_error("Mel session is not initialized");
    }

    const size_t frame_size = step_frames_ * CHUNK_SAMPLES;
    if (pcm_frame.size() != frame_size) {
        throw std::runtime_error("runMelSpectrogram got unexpected frame size");
    }

    std::vector<int64_t> input_shape = {
        1,
        static_cast<int64_t>(frame_size)
    };

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_,
        const_cast<float*>(pcm_frame.data()),
        pcm_frame.size(),
        input_shape.data(),
        input_shape.size()
    );

    Ort::AllocatorWithDefaultOptions allocator;
    auto input_name_alloc = mel_session_->GetInputNameAllocated(0, allocator);
    auto output_name_alloc = mel_session_->GetOutputNameAllocated(0, allocator);

    const char* input_names[] = {input_name_alloc.get()};
    const char* output_names[] = {output_name_alloc.get()};

    auto output_tensors = mel_session_->Run(
        Ort::RunOptions{nullptr},
        input_names, &input_tensor, 1,
        output_names, 1
    );

    if (output_tensors.empty()) {
        throw std::runtime_error("Mel model returned no outputs");
    }

    auto& out = output_tensors.front();
    const auto shape = out.GetTensorTypeAndShapeInfo().GetShape();
    const size_t count = out.GetTensorTypeAndShapeInfo().GetElementCount();
    const float* data = out.GetTensorData<float>();

    if (debug_enabled_) {
        std::cout << "[mel] output shape = " << shapeToString(shape)
                  << ", count = " << count << std::endl;
    }

    std::vector<float> mel_scaled;
    mel_scaled.reserve(count);

    // GitHub file scales mel for embedding model
    for (size_t i = 0; i < count; ++i) {
        mel_scaled.push_back((data[i] / 10.0f) + 2.0f);
    }

    if (debug_enabled_) {
        printFloatHead("[OWW] melScaled head: ", mel_scaled, 10);
    }

    return mel_scaled;
}

std::vector<float> OpenWakeWordEngine::runEmbedding(const std::vector<float>& mel_window_76x32) {
    if (!embed_session_) {
        throw std::runtime_error("Embedding session is not initialized");
    }

    if (mel_window_76x32.size() != EMB_WINDOW_SIZE * NUM_MELS) {
        throw std::runtime_error("runEmbedding expects exactly 76*32 floats");
    }

    std::vector<int64_t> input_shape = {
        1,
        static_cast<int64_t>(EMB_WINDOW_SIZE),
        static_cast<int64_t>(NUM_MELS),
        1
    };

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_,
        const_cast<float*>(mel_window_76x32.data()),
        mel_window_76x32.size(),
        input_shape.data(),
        input_shape.size()
    );

    Ort::AllocatorWithDefaultOptions allocator;
    auto input_name_alloc = embed_session_->GetInputNameAllocated(0, allocator);
    auto output_name_alloc = embed_session_->GetOutputNameAllocated(0, allocator);

    const char* input_names[] = {input_name_alloc.get()};
    const char* output_names[] = {output_name_alloc.get()};

    auto output_tensors = embed_session_->Run(
        Ort::RunOptions{nullptr},
        input_names, &input_tensor, 1,
        output_names, 1
    );

    if (output_tensors.empty()) {
        throw std::runtime_error("Embedding model returned no outputs");
    }

    auto& out = output_tensors.front();
    const auto shape = out.GetTensorTypeAndShapeInfo().GetShape();
    const size_t count = out.GetTensorTypeAndShapeInfo().GetElementCount();
    const float* data = out.GetTensorData<float>();

    if (debug_enabled_) {
        std::cout << "[embed] output shape = " << shapeToString(shape)
                  << ", count = " << count << std::endl;
    }

    std::vector<float> emb(data, data + count);

    if (debug_enabled_) {
        printFloatHead("[OWW] embedOut head: ", emb, 10);
    }

    return emb;
}

float OpenWakeWordEngine::runWakeWord(const std::vector<float>& embed_window_16x96) {
    if (!ww_session_) {
        throw std::runtime_error("Wakeword session is not initialized");
    }

    if (embed_window_16x96.size() != WW_FEATURES * EMB_FEATURES) {
        throw std::runtime_error("runWakeWord expects exactly 16*96 floats");
    }

    std::vector<int64_t> input_shape = {
        1,
        static_cast<int64_t>(WW_FEATURES),
        static_cast<int64_t>(EMB_FEATURES)
    };

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_,
        const_cast<float*>(embed_window_16x96.data()),
        embed_window_16x96.size(),
        input_shape.data(),
        input_shape.size()
    );

    Ort::AllocatorWithDefaultOptions allocator;
    auto input_name_alloc = ww_session_->GetInputNameAllocated(0, allocator);
    auto output_name_alloc = ww_session_->GetOutputNameAllocated(0, allocator);

    const char* input_names[] = {input_name_alloc.get()};
    const char* output_names[] = {output_name_alloc.get()};

    auto output_tensors = ww_session_->Run(
        Ort::RunOptions{nullptr},
        input_names, &input_tensor, 1,
        output_names, 1
    );

    if (output_tensors.empty()) {
        throw std::runtime_error("Wakeword model returned no outputs");
    }

    auto& out = output_tensors.front();
    const auto shape = out.GetTensorTypeAndShapeInfo().GetShape();
    const size_t count = out.GetTensorTypeAndShapeInfo().GetElementCount();
    const float* data = out.GetTensorData<float>();

    if (debug_enabled_) {
        std::cout << "[ww] output shape = " << shapeToString(shape)
                  << ", count = " << count << std::endl;
    }

    if (count < 1) {
        throw std::runtime_error("Wakeword output is empty");
    }

    return data[0];
}