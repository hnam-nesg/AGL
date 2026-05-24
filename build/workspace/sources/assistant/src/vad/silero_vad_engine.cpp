#include "silero_vad_engine.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>

SileroVadEngine::SileroVadEngine(const Config& cfg)
    : cfg_(cfg)
{
}

bool SileroVadEngine::initialize(std::string* error)
{
    try {
        session_options_.SetIntraOpNumThreads(std::max(1, cfg_.intra_threads));
        session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        session_ = std::make_unique<Ort::Session>(
            env_,
            cfg_.model_path.c_str(),
            session_options_
        );

        if (!inspectModelIO(error)) {
            session_.reset();
            return false;
        }

        reset();
        return true;
    } catch (const std::exception& e) {
        if (error) {
            *error = e.what();
        }
        session_.reset();
        return false;
    }
}

void SileroVadEngine::reset()
{
    context_.assign(static_cast<size_t>(cfg_.context_size_samples), 0.0f);

    // Silero C++ examples/model family thường dùng [2,1,64] cho h/c
    h_state_.assign(2 * 1 * 64, 0.0f);
    c_state_.assign(2 * 1 * 64, 0.0f);
}

SileroVadEngine::Result SileroVadEngine::processPcm16(const std::vector<int16_t>& pcm16)
{
    return processFloat(pcm16ToFloat(pcm16));
}

SileroVadEngine::Result SileroVadEngine::processFloat(const std::vector<float>& audioFloat)
{
    Result out;

    if (!session_) {
        return out;
    }

    if (static_cast<int>(audioFloat.size()) != cfg_.window_size_samples) {
        if (cfg_.debug) {
            std::cerr << "[SileroVAD] Invalid window size: got=" << audioFloat.size()
                      << " expected=" << cfg_.window_size_samples << "\n";
        }
        return out;
    }

    try {
        std::vector<float> inputAudio;
        inputAudio.reserve(
            static_cast<size_t>(cfg_.context_size_samples + cfg_.window_size_samples)
        );

        inputAudio.insert(inputAudio.end(), context_.begin(), context_.end());
        inputAudio.insert(inputAudio.end(), audioFloat.begin(), audioFloat.end());

        context_.assign(
            audioFloat.end() - cfg_.context_size_samples,
            audioFloat.end()
        );

        Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator, OrtMemTypeDefault
        );

        std::vector<int64_t> audioShape = {1, static_cast<int64_t>(inputAudio.size())};
        Ort::Value audioTensor = Ort::Value::CreateTensor<float>(
            memInfo,
            inputAudio.data(),
            inputAudio.size(),
            audioShape.data(),
            audioShape.size()
        );

        std::vector<const char*> inputNames;
        std::vector<Ort::Value> inputValues;

        inputNames.push_back(input_audio_name_.c_str());
        inputValues.push_back(std::move(audioTensor));

        int64_t srValue = cfg_.sample_rate;
        std::vector<int64_t> srShape = {1};
        Ort::Value srTensor(nullptr);
        if (has_sr_input_) {
            srTensor = Ort::Value::CreateTensor<int64_t>(
                memInfo,
                &srValue,
                1,
                srShape.data(),
                srShape.size()
            );
            inputNames.push_back(input_sr_name_.c_str());
            inputValues.push_back(std::move(srTensor));
        }

        std::vector<int64_t> hcShape = {2, 1, 64};
        Ort::Value hTensor(nullptr);
        Ort::Value cTensor(nullptr);
        if (has_hc_inputs_) {
            hTensor = Ort::Value::CreateTensor<float>(
                memInfo,
                h_state_.data(),
                h_state_.size(),
                hcShape.data(),
                hcShape.size()
            );
            cTensor = Ort::Value::CreateTensor<float>(
                memInfo,
                c_state_.data(),
                c_state_.size(),
                hcShape.data(),
                hcShape.size()
            );

            inputNames.push_back(input_h_name_.c_str());
            inputValues.push_back(std::move(hTensor));
            inputNames.push_back(input_c_name_.c_str());
            inputValues.push_back(std::move(cTensor));
        }

        std::vector<const char*> outputNames;
        outputNames.push_back(output_prob_name_.c_str());
        if (has_hc_outputs_) {
            outputNames.push_back(output_h_name_.c_str());
            outputNames.push_back(output_c_name_.c_str());
        }

        auto outputs = session_->Run(
            Ort::RunOptions{nullptr},
            inputNames.data(),
            inputValues.data(),
            inputValues.size(),
            outputNames.data(),
            outputNames.size()
        );

        if (outputs.empty()) {
            return out;
        }

        float prob = 0.0f;
        {
            float* probData = outputs[0].GetTensorMutableData<float>();
            prob = probData ? probData[0] : 0.0f;
        }

        if (has_hc_outputs_ && outputs.size() >= 3) {
            const float* hOut = outputs[1].GetTensorData<float>();
            const float* cOut = outputs[2].GetTensorData<float>();

            if (hOut) {
                std::memcpy(h_state_.data(), hOut, h_state_.size() * sizeof(float));
            }
            if (cOut) {
                std::memcpy(c_state_.data(), cOut, c_state_.size() * sizeof(float));
            }
        }

        out.ok = true;
        out.speech_prob = prob;
        out.is_speech = prob >= cfg_.speech_threshold;

        if (cfg_.debug) {
            std::cerr << "[SileroVAD] prob=" << prob
                      << " speech=" << (out.is_speech ? 1 : 0) << "\n";
        }

        return out;
    } catch (const std::exception& e) {
        if (cfg_.debug) {
            std::cerr << "[SileroVAD] inference failed: " << e.what() << "\n";
        }
        return out;
    }
}

bool SileroVadEngine::inspectModelIO(std::string* error)
{
    if (!session_) {
        if (error) {
            *error = "Session is null";
        }
        return false;
    }

    Ort::AllocatorWithDefaultOptions allocator;

    const size_t inputCount = session_->GetInputCount();
    const size_t outputCount = session_->GetOutputCount();

    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;

    inputNames.reserve(inputCount);
    outputNames.reserve(outputCount);

    for (size_t i = 0; i < inputCount; ++i) {
        auto name = session_->GetInputNameAllocated(i, allocator);
        inputNames.emplace_back(name.get() ? name.get() : "");
    }

    for (size_t i = 0; i < outputCount; ++i) {
        auto name = session_->GetOutputNameAllocated(i, allocator);
        outputNames.emplace_back(name.get() ? name.get() : "");
    }

    for (const auto& n : inputNames) {
        const std::string ln = toLower(n);
        if (input_audio_name_.empty() &&
            (ln == "input" || ln == "x" || containsInsensitive(ln, "input"))) {
            input_audio_name_ = n;
        } else if (input_sr_name_.empty() &&
                   (ln == "sr" || containsInsensitive(ln, "rate"))) {
            input_sr_name_ = n;
            has_sr_input_ = true;
        } else if (input_h_name_.empty() &&
                   (ln == "h" || containsInsensitive(ln, "state"))) {
            input_h_name_ = n;
            has_hc_inputs_ = true;
        } else if (input_c_name_.empty() &&
                   (ln == "c" || containsInsensitive(ln, "cell"))) {
            input_c_name_ = n;
            has_hc_inputs_ = true;
        }
    }

    if (input_audio_name_.empty() && !inputNames.empty()) {
        input_audio_name_ = inputNames[0];
    }
    if (has_sr_input_ == false && inputNames.size() >= 2) {
        const auto maybe = toLower(inputNames[1]);
        if (maybe == "sr" || containsInsensitive(maybe, "rate")) {
            input_sr_name_ = inputNames[1];
            has_sr_input_ = true;
        }
    }
    if (input_h_name_.empty() && inputNames.size() >= 3) {
        input_h_name_ = inputNames[2];
        has_hc_inputs_ = true;
    }
    if (input_c_name_.empty() && inputNames.size() >= 4) {
        input_c_name_ = inputNames[3];
        has_hc_inputs_ = true;
    }

    for (const auto& n : outputNames) {
        const std::string ln = toLower(n);
        if (output_prob_name_.empty() &&
            (ln == "output" || containsInsensitive(ln, "output") || containsInsensitive(ln, "speech"))) {
            output_prob_name_ = n;
        } else if (output_h_name_.empty() &&
                   (ln == "hn" || ln == "h" || containsInsensitive(ln, "hn"))) {
            output_h_name_ = n;
            has_hc_outputs_ = true;
        } else if (output_c_name_.empty() &&
                   (ln == "cn" || ln == "c" || containsInsensitive(ln, "cn"))) {
            output_c_name_ = n;
            has_hc_outputs_ = true;
        }
    }

    if (output_prob_name_.empty() && !outputNames.empty()) {
        output_prob_name_ = outputNames[0];
    }
    if (output_h_name_.empty() && outputNames.size() >= 2) {
        output_h_name_ = outputNames[1];
        has_hc_outputs_ = true;
    }
    if (output_c_name_.empty() && outputNames.size() >= 3) {
        output_c_name_ = outputNames[2];
        has_hc_outputs_ = true;
    }

    if (input_audio_name_.empty() || output_prob_name_.empty()) {
        if (error) {
            *error = "Failed to resolve Silero VAD model I/O names";
        }
        return false;
    }

    return true;
}

std::string SileroVadEngine::toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool SileroVadEngine::containsInsensitive(const std::string& s, const std::string& needle)
{
    return toLower(s).find(toLower(needle)) != std::string::npos;
}

std::vector<float> SileroVadEngine::pcm16ToFloat(const std::vector<int16_t>& in)
{
    std::vector<float> out;
    out.reserve(in.size());

    for (int16_t v : in) {
        out.push_back(static_cast<float>(v) / 32768.0f);
    }

    return out;
}