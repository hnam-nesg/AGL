#include "PhoBertMultiHeadClassifier.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPair>
#include <QRegularExpression>
#include <QTextStream>
#include <QVector>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace nlu {
namespace {

constexpr int kHeadCount = static_cast<int>(PhoBertMultiHeadClassifier::Head::Count);

std::string toLowerAscii(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

bool containsInsensitive(const std::string& haystack, const std::string& needle)
{
    return toLowerAscii(haystack).find(toLowerAscii(needle)) != std::string::npos;
}

QString stripBom(QString value)
{
    if (!value.isEmpty() && value.front().unicode() == 0xFEFF) {
        value.remove(0, 1);
    }
    return value;
}

bool isMetadataKey(const QString& key)
{
    const QString lower = key.toLower();
    return lower == QStringLiteral("num_labels") ||
           lower == QStringLiteral("num_classes") ||
           lower == QStringLiteral("label_count") ||
           lower == QStringLiteral("count") ||
           lower == QStringLiteral("name") ||
           lower == QStringLiteral("head") ||
           lower == QStringLiteral("type");
}

bool labelsFromJsonValue(const QJsonValue& value, QStringList* out, int depth = 0)
{
    if (!out || depth > 4) {
        return false;
    }

    if (value.isArray()) {
        QStringList labels;
        const QJsonArray array = value.toArray();
        for (const QJsonValue& item : array) {
            if (item.isString()) {
                labels.push_back(item.toString());
            }
        }

        if (!labels.isEmpty()) {
            *out = labels;
            return true;
        }
        return false;
    }

    if (!value.isObject()) {
        return false;
    }

    const QJsonObject objectValue = value.toObject();
    QVector<QPair<int, QString>> indexedLabels;

    for (auto it = objectValue.begin(); it != objectValue.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok && it.value().isString()) {
            indexedLabels.push_back({index, it.value().toString()});
            continue;
        }

        if (isMetadataKey(it.key())) {
            continue;
        }

        int labelIndex = -1;
        if (it.value().isDouble()) {
            labelIndex = it.value().toInt(-1);
        } else if (it.value().isString()) {
            bool valueOk = false;
            labelIndex = it.value().toString().toInt(&valueOk);
            if (!valueOk) {
                labelIndex = -1;
            }
        }

        if (labelIndex >= 0) {
            indexedLabels.push_back({labelIndex, it.key()});
        }
    }

    std::sort(indexedLabels.begin(), indexedLabels.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    QStringList labels;
    for (const auto& item : indexedLabels) {
        labels.push_back(item.second);
    }
    if (!labels.isEmpty()) {
        *out = labels;
        return true;
    }

    const QStringList nestedPriority = {
        QStringLiteral("labels"),
        QStringLiteral("classes"),
        QStringLiteral("id2label"),
        QStringLiteral("idx2label"),
        QStringLiteral("index_to_label"),
        QStringLiteral("label_list"),
        QStringLiteral("label2id"),
        QStringLiteral("label_to_id")
    };

    for (const QString& nestedKey : nestedPriority) {
        if (objectValue.contains(nestedKey) &&
            labelsFromJsonValue(objectValue.value(nestedKey), out, depth + 1)) {
            return true;
        }
    }

    for (auto it = objectValue.begin(); it != objectValue.end(); ++it) {
        if (labelsFromJsonValue(it.value(), out, depth + 1)) {
            return true;
        }
    }

    return false;
}

} // namespace

PhoBertMultiHeadClassifier::PhoBertMultiHeadClassifier()
    : PhoBertMultiHeadClassifier(Config{})
{
}

PhoBertMultiHeadClassifier::PhoBertMultiHeadClassifier(const Config& cfg)
    : cfg_(cfg),
      maxSeqLen_(std::max(8, cfg.max_seq_len))
{
    outputIndexByHead_.fill(-1);
}

bool PhoBertMultiHeadClassifier::initialize(QString* errorMessage)
{
    session_.reset();
    inputNames_.clear();
    outputNames_.clear();
    outputIndexByHead_.fill(-1);

    if (!cfg_.enabled) {
        return true;
    }

    if (!loadVocab(errorMessage)) return false;
    if (!loadBpeCodes(errorMessage)) return false;
    if (!loadLabelMap(errorMessage)) return false;

    try {
        sessionOptions_.SetIntraOpNumThreads(std::max(1, cfg_.intra_threads));
        sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        session_ = std::make_unique<Ort::Session>(
            env_,
            cfg_.model_path.c_str(),
            sessionOptions_
        );
    } catch (const std::exception& e) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("PhoBERT ONNX load failed: %1")
                                .arg(QString::fromUtf8(e.what()));
        }
        session_.reset();
        return false;
    }

    return inspectModelIO(errorMessage);
}

bool PhoBertMultiHeadClassifier::loadVocab(QString* errorMessage)
{
    QFile file(QString::fromStdString(cfg_.vocab_path));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Cannot open PhoBERT vocab: %1")
                                .arg(QString::fromStdString(cfg_.vocab_path));
        }
        return false;
    }

    vocab_.clear();

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    vocab_.insert(QStringLiteral("<s>"), 0);
    vocab_.insert(QStringLiteral("<pad>"), 1);
    vocab_.insert(QStringLiteral("</s>"), 2);
    vocab_.insert(QStringLiteral("<unk>"), 3);
    vocab_.insert(QStringLiteral("<mask>"), 64000);
    maxVocabId_ = 64000;

    int64_t row = 4;
    bool formatDetected = false;
    bool fairseqFrequencyFormat = false;
    while (!in.atEnd()) {
        QString line = stripBom(in.readLine()).trimmed();
        if (line.isEmpty()) {
            continue;
        }

        const QStringList parts = line.split(QRegularExpression(QStringLiteral("\\s+")));
        QString token = parts.first();
        int64_t id = row;

        if (parts.size() >= 2) {
            bool firstIsId = false;
            bool lastIsId = false;
            const int64_t firstId = parts.first().toLongLong(&firstIsId);
            const int64_t lastId = parts.last().toLongLong(&lastIsId);

            if (!formatDetected) {
                // PhoBERT/fairseq vocab is "token frequency"; the second
                // column is not a token id. The first rows have very large
                // counts, which makes this format easy to distinguish.
                fairseqFrequencyFormat = !firstIsId && lastIsId && lastId > 64000;
                formatDetected = true;
            }

            if (!fairseqFrequencyFormat) {
                if (firstIsId && !lastIsId) {
                    id = firstId;
                    token = parts.mid(1).join(QStringLiteral(" "));
                } else if (!firstIsId && lastIsId && lastId <= 64000) {
                    id = lastId;
                    token = parts.mid(0, parts.size() - 1).join(QStringLiteral(" "));
                }
            }
        }

        if (token.isEmpty()) {
            continue;
        }

        if (!vocab_.contains(token)) {
            vocab_.insert(token, id);
        }
        ++row;
    }

    auto resolve = [this](const QStringList& candidates, int64_t fallback) {
        for (const QString& token : candidates) {
            if (vocab_.contains(token)) {
                return vocab_.value(token);
            }
        }
        return fallback;
    };

    clsId_ = resolve({QStringLiteral("<s>"), QStringLiteral("[CLS]")}, 0);
    sepId_ = resolve({QStringLiteral("</s>"), QStringLiteral("[SEP]")}, 2);
    padId_ = resolve({QStringLiteral("<pad>"), QStringLiteral("[PAD]")}, 1);
    unkId_ = resolve({QStringLiteral("<unk>"), QStringLiteral("[UNK]")}, 3);

    if (vocab_.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("PhoBERT vocab is empty: %1")
                                .arg(QString::fromStdString(cfg_.vocab_path));
        }
        return false;
    }

    if (cfg_.debug) {
        std::cout << "[PhoBERT] vocab size=" << vocab_.size()
                  << " max_id=" << maxVocabId_
                  << " unk_id=" << unkId_
                  << "\n";
    }

    return true;
}

bool PhoBertMultiHeadClassifier::loadBpeCodes(QString* errorMessage)
{
    QFile file(QString::fromStdString(cfg_.bpe_codes_path));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Cannot open PhoBERT BPE codes: %1")
                                .arg(QString::fromStdString(cfg_.bpe_codes_path));
        }
        return false;
    }

    bpeRanks_.clear();

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    int rank = 0;
    while (!in.atEnd()) {
        QString line = stripBom(in.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith(QStringLiteral("#"))) {
            continue;
        }

        const QStringList parts = line.split(QRegularExpression(QStringLiteral("\\s+")),
                                             Qt::SkipEmptyParts);
        if (parts.size() < 2) {
            continue;
        }

        bpeRanks_.insert(parts[0] + QStringLiteral("\t") + parts[1], rank++);
    }

    if (bpeRanks_.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("PhoBERT BPE codes are empty: %1")
                                .arg(QString::fromStdString(cfg_.bpe_codes_path));
        }
        return false;
    }

    return true;
}

bool PhoBertMultiHeadClassifier::readJsonStringArray(const QJsonObject& object,
                                                     const QStringList& candidateKeys,
                                                     QStringList* out)
{
    if (!out) return false;

    for (const QString& key : candidateKeys) {
        const QJsonValue value = object.value(key);
        if (labelsFromJsonValue(value, out)) {
            return true;
        }
    }

    return false;
}

bool PhoBertMultiHeadClassifier::loadLabelMap(QString* errorMessage)
{
    QFile file(cfg_.label_map_path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Cannot open PhoBERT label map: %1")
                                .arg(cfg_.label_map_path);
        }
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("PhoBERT label map must be a JSON object: %1")
                                .arg(cfg_.label_map_path);
        }
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonObject heads = root.value(QStringLiteral("heads")).isObject()
                                ? root.value(QStringLiteral("heads")).toObject()
                                : root;

    const std::array<QStringList, kHeadCount> keys = {
        QStringList{
            QStringLiteral("action_label"),
            QStringLiteral("action_labels"),
            QStringLiteral("action"),
            QStringLiteral("actions"),
            QStringLiteral("action_label_map"),
            QStringLiteral("action_label2id"),
            QStringLiteral("action_id2label"),
            QStringLiteral("action2id"),
            QStringLiteral("id2action")
        },
        QStringList{
            QStringLiteral("domain_label"),
            QStringLiteral("domain_labels"),
            QStringLiteral("domain"),
            QStringLiteral("domains"),
            QStringLiteral("domain_label_map"),
            QStringLiteral("domain_label2id"),
            QStringLiteral("domain_id2label"),
            QStringLiteral("domain2id"),
            QStringLiteral("id2domain")
        },
        QStringList{
            QStringLiteral("op_label"),
            QStringLiteral("op_labels"),
            QStringLiteral("operator_label"),
            QStringLiteral("operation_label"),
            QStringLiteral("op"),
            QStringLiteral("ops"),
            QStringLiteral("op_label_map"),
            QStringLiteral("op_label2id"),
            QStringLiteral("op_id2label"),
            QStringLiteral("op2id"),
            QStringLiteral("id2op")
        },
        QStringList{
            QStringLiteral("target_label"),
            QStringLiteral("target_labels"),
            QStringLiteral("target"),
            QStringLiteral("targets"),
            QStringLiteral("target_label_map"),
            QStringLiteral("target_label2id"),
            QStringLiteral("target_id2label"),
            QStringLiteral("target2id"),
            QStringLiteral("id2target")
        },
        QStringList{
            QStringLiteral("execute_label"),
            QStringLiteral("execute_labels"),
            QStringLiteral("exec_label"),
            QStringLiteral("execute"),
            QStringLiteral("executes"),
            QStringLiteral("execute_label_map"),
            QStringLiteral("execute_label2id"),
            QStringLiteral("execute_id2label"),
            QStringLiteral("execute2id"),
            QStringLiteral("id2execute")
        },
        QStringList{
            QStringLiteral("slot_type"),
            QStringLiteral("slot_types"),
            QStringLiteral("slot_label"),
            QStringLiteral("slot_labels"),
            QStringLiteral("slot"),
            QStringLiteral("slotValues"),
            QStringLiteral("slot_type_map"),
            QStringLiteral("slot_type2id"),
            QStringLiteral("slot_id2type"),
            QStringLiteral("slot2id"),
            QStringLiteral("id2slot")
        }
    };

    for (int i = 0; i < kHeadCount; ++i) {
        QStringList labels;
        if (!readJsonStringArray(heads, keys[static_cast<size_t>(i)], &labels)) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Missing label map for head %1 in %2. Available keys: %3")
                                    .arg(headName(static_cast<Head>(i)),
                                         cfg_.label_map_path,
                                         heads.keys().join(QStringLiteral(", ")));
            }
            return false;
        }
        labels_[static_cast<size_t>(i)] = labels;
    }

    return true;
}

bool PhoBertMultiHeadClassifier::inspectModelIO(QString* errorMessage)
{
    if (!session_) {
        if (errorMessage) *errorMessage = QStringLiteral("PhoBERT session is null");
        return false;
    }

    Ort::AllocatorWithDefaultOptions allocator;

    const size_t inputCount = session_->GetInputCount();
    const size_t outputCount = session_->GetOutputCount();

    inputNames_.reserve(inputCount);
    outputNames_.reserve(outputCount);

    for (size_t i = 0; i < inputCount; ++i) {
        auto name = session_->GetInputNameAllocated(i, allocator);
        inputNames_.emplace_back(name.get() ? name.get() : "");
    }

    for (size_t i = 0; i < outputCount; ++i) {
        auto name = session_->GetOutputNameAllocated(i, allocator);
        outputNames_.emplace_back(name.get() ? name.get() : "");
    }

    for (const std::string& name : inputNames_) {
        const std::string lower = toLowerAscii(name);
        if (inputIdsName_.isEmpty() &&
            (lower == "input_ids" || containsInsensitive(lower, "input_ids") ||
             containsInsensitive(lower, "input_ids_"))) {
            inputIdsName_ = QString::fromStdString(name);
        } else if (attentionMaskName_.isEmpty() &&
                   (lower == "attention_mask" || containsInsensitive(lower, "attention"))) {
            attentionMaskName_ = QString::fromStdString(name);
            hasAttentionMask_ = true;
        } else if (tokenTypeIdsName_.isEmpty() &&
                   (lower == "token_type_ids" || containsInsensitive(lower, "token_type") ||
                    containsInsensitive(lower, "segment"))) {
            tokenTypeIdsName_ = QString::fromStdString(name);
            hasTokenTypeIds_ = true;
        }
    }

    if (inputIdsName_.isEmpty() && !inputNames_.empty()) {
        inputIdsName_ = QString::fromStdString(inputNames_[0]);
    }
    if (attentionMaskName_.isEmpty() && inputNames_.size() >= 2) {
        attentionMaskName_ = QString::fromStdString(inputNames_[1]);
        hasAttentionMask_ = true;
    }
    if (tokenTypeIdsName_.isEmpty() && inputNames_.size() >= 3) {
        tokenTypeIdsName_ = QString::fromStdString(inputNames_[2]);
        hasTokenTypeIds_ = true;
    }

    const std::array<QStringList, kHeadCount> outputKeyWords = {
        QStringList{QStringLiteral("action")},
        QStringList{QStringLiteral("domain")},
        QStringList{QStringLiteral("op"), QStringLiteral("operator")},
        QStringList{QStringLiteral("target")},
        QStringList{QStringLiteral("execute"), QStringLiteral("exec")},
        QStringList{QStringLiteral("slot")}
    };

    for (size_t i = 0; i < outputNames_.size(); ++i) {
        const QString name = QString::fromStdString(outputNames_[i]).toLower();
        for (int h = 0; h < kHeadCount; ++h) {
            if (outputIndexByHead_[static_cast<size_t>(h)] >= 0) {
                continue;
            }
            for (const QString& keyword : outputKeyWords[static_cast<size_t>(h)]) {
                if (name.contains(keyword)) {
                    outputIndexByHead_[static_cast<size_t>(h)] = static_cast<int>(i);
                    break;
                }
            }
        }
    }

    for (int h = 0; h < kHeadCount; ++h) {
        if (outputIndexByHead_[static_cast<size_t>(h)] < 0 &&
            static_cast<size_t>(h) < outputNames_.size()) {
            outputIndexByHead_[static_cast<size_t>(h)] = h;
        }
    }

    if (inputIdsName_.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("Cannot resolve PhoBERT input_ids input");
        return false;
    }

    for (int h = 0; h < kHeadCount; ++h) {
        if (outputIndexByHead_[static_cast<size_t>(h)] < 0 ||
            outputIndexByHead_[static_cast<size_t>(h)] >= static_cast<int>(outputNames_.size())) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Cannot resolve PhoBERT output for head %1")
                                    .arg(headName(static_cast<Head>(h)));
            }
            return false;
        }
    }

    if (cfg_.debug) {
        std::cout << "[PhoBERT] inputs:";
        for (const auto& name : inputNames_) std::cout << " " << name;
        std::cout << "\n[PhoBERT] outputs:";
        for (const auto& name : outputNames_) std::cout << " " << name;
        std::cout << "\n";
    }

    return true;
}

QString PhoBertMultiHeadClassifier::normalizeText(QString text)
{
    text = text.toLower().trimmed();
    text.replace(QRegularExpression(QStringLiteral("[\\.,;:!?\\[\\]\\(\\)\\{\\}\\\"']+")),
                 QStringLiteral(" "));
    text.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    return text.trimmed();
}

QStringList PhoBertMultiHeadClassifier::bpeTokenizeWord(const QString& word) const
{
    if (word.isEmpty()) {
        return {};
    }

    QStringList symbols;
    symbols.reserve(word.size());
    for (int i = 0; i < word.size(); ++i) {
        QString symbol(word.at(i));
        if (i == word.size() - 1) {
            symbol += QStringLiteral("</w>");
        }
        symbols.push_back(symbol);
    }

    if (symbols.size() == 1) {
        QString only = symbols.first();
        only.chop(4);
        return {only};
    }

    while (symbols.size() > 1) {
        int bestRank = std::numeric_limits<int>::max();
        int bestIndex = -1;

        for (int i = 0; i < symbols.size() - 1; ++i) {
            const QString key = symbols[i] + QStringLiteral("\t") + symbols[i + 1];
            const auto it = bpeRanks_.find(key);
            if (it != bpeRanks_.end() && it.value() < bestRank) {
                bestRank = it.value();
                bestIndex = i;
            }
        }

        if (bestIndex < 0) {
            break;
        }

        symbols[bestIndex] += symbols[bestIndex + 1];
        symbols.removeAt(bestIndex + 1);
    }

    for (int i = 0; i < symbols.size(); ++i) {
        QString& symbol = symbols[i];
        if (symbol.endsWith(QStringLiteral("</w>"))) {
            symbol.chop(4);
        } else if (i != symbols.size() - 1) {
            symbol += QStringLiteral("@@");
        }
    }

    return symbols;
}

QStringList PhoBertMultiHeadClassifier::tokenize(const QString& text) const
{
    const QString normalized = normalizeText(text);
    const QStringList words = normalized.split(QRegularExpression(QStringLiteral("\\s+")),
                                               Qt::SkipEmptyParts);

    QStringList tokens;
    for (const QString& word : words) {
        const QStringList pieces = bpeTokenizeWord(word);
        if (pieces.isEmpty()) {
            tokens.push_back(QStringLiteral("<unk>"));
        } else {
            tokens.append(pieces);
        }
    }

    return tokens;
}

PhoBertMultiHeadClassifier::EncodedInput PhoBertMultiHeadClassifier::encode(const QString& text) const
{
    EncodedInput encoded;
    encoded.inputIds.reserve(maxSeqLen_);
    encoded.attentionMask.reserve(maxSeqLen_);
    encoded.tokenTypeIds.reserve(maxSeqLen_);

    encoded.inputIds.push_back(clsId_);

    const QStringList tokens = tokenize(text);
    for (const QString& token : tokens) {
        if (static_cast<int>(encoded.inputIds.size()) >= maxSeqLen_ - 1) {
            break;
        }
        const int64_t id = vocab_.value(token, unkId_);
        encoded.inputIds.push_back((id >= 0 && id <= maxVocabId_) ? id : unkId_);
    }

    encoded.inputIds.push_back(sepId_);

    encoded.attentionMask.assign(encoded.inputIds.size(), 1);
    encoded.tokenTypeIds.assign(encoded.inputIds.size(), 0);

    while (static_cast<int>(encoded.inputIds.size()) < maxSeqLen_) {
        encoded.inputIds.push_back(padId_);
        encoded.attentionMask.push_back(0);
        encoded.tokenTypeIds.push_back(0);
    }

    return encoded;
}

double PhoBertMultiHeadClassifier::softmaxConfidence(const std::vector<double>& logits, int index)
{
    if (logits.empty() || index < 0 || index >= static_cast<int>(logits.size())) {
        return 0.0;
    }

    const double maxValue = *std::max_element(logits.begin(), logits.end());
    double denom = 0.0;
    for (double value : logits) {
        denom += std::exp(value - maxValue);
    }
    if (denom <= 0.0) {
        return 0.0;
    }
    return std::exp(logits[static_cast<size_t>(index)] - maxValue) / denom;
}

PhoBertMultiHeadClassifier::HeadResult
PhoBertMultiHeadClassifier::decodeHead(Head head, Ort::Value& output) const
{
    HeadResult result;

    const auto info = output.GetTensorTypeAndShapeInfo();
    const size_t elementCount = info.GetElementCount();
    if (elementCount == 0) {
        return result;
    }

    std::vector<double> logits;
    logits.reserve(elementCount);

    const auto type = info.GetElementType();
    if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
        const float* data = output.GetTensorData<float>();
        for (size_t i = 0; i < elementCount; ++i) {
            logits.push_back(data[i]);
        }
    } else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE) {
        const double* data = output.GetTensorData<double>();
        for (size_t i = 0; i < elementCount; ++i) {
            logits.push_back(data[i]);
        }
    } else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
        const int64_t* data = output.GetTensorData<int64_t>();
        for (size_t i = 0; i < elementCount; ++i) {
            logits.push_back(static_cast<double>(data[i]));
        }
    } else {
        return result;
    }

    const auto maxIt = std::max_element(logits.begin(), logits.end());
    result.index = static_cast<int>(std::distance(logits.begin(), maxIt));
    result.confidence = softmaxConfidence(logits, result.index);

    const QStringList& labels = labelsFor(head);
    if (result.index >= 0 && result.index < labels.size()) {
        result.label = labels[result.index];
    }

    return result;
}

const QStringList& PhoBertMultiHeadClassifier::labelsFor(Head head) const
{
    return labels_[static_cast<size_t>(head)];
}

PhoBertMultiHeadClassifier::Prediction PhoBertMultiHeadClassifier::predict(const QString& text)
{
    Prediction prediction;
    prediction.normalizedText = normalizeText(text);

    if (!cfg_.enabled) {
        prediction.error = QStringLiteral("PhoBERT classifier is disabled");
        return prediction;
    }

    if (!session_) {
        prediction.error = QStringLiteral("PhoBERT classifier is not initialized");
        return prediction;
    }

    try {
        const EncodedInput encoded = encode(text);

        Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator, OrtMemTypeDefault
        );

        std::array<int64_t, 2> shape = {1, static_cast<int64_t>(maxSeqLen_)};

        Ort::Value inputIds = Ort::Value::CreateTensor<int64_t>(
            memInfo,
            const_cast<int64_t*>(encoded.inputIds.data()),
            encoded.inputIds.size(),
            shape.data(),
            shape.size()
        );

        Ort::Value attentionMask(nullptr);
        Ort::Value tokenTypeIds(nullptr);

        std::vector<const char*> inputNamePtrs;
        std::vector<Ort::Value> inputValues;

        const std::string inputIdsName = inputIdsName_.toStdString();
        inputNamePtrs.push_back(inputIdsName.c_str());
        inputValues.push_back(std::move(inputIds));

        std::string attentionMaskName;
        if (hasAttentionMask_) {
            attentionMask = Ort::Value::CreateTensor<int64_t>(
                memInfo,
                const_cast<int64_t*>(encoded.attentionMask.data()),
                encoded.attentionMask.size(),
                shape.data(),
                shape.size()
            );
            attentionMaskName = attentionMaskName_.toStdString();
            inputNamePtrs.push_back(attentionMaskName.c_str());
            inputValues.push_back(std::move(attentionMask));
        }

        std::string tokenTypeIdsName;
        if (hasTokenTypeIds_) {
            tokenTypeIds = Ort::Value::CreateTensor<int64_t>(
                memInfo,
                const_cast<int64_t*>(encoded.tokenTypeIds.data()),
                encoded.tokenTypeIds.size(),
                shape.data(),
                shape.size()
            );
            tokenTypeIdsName = tokenTypeIdsName_.toStdString();
            inputNamePtrs.push_back(tokenTypeIdsName.c_str());
            inputValues.push_back(std::move(tokenTypeIds));
        }

        std::array<const char*, kHeadCount> outputNamePtrs{};
        std::array<std::string, kHeadCount> selectedOutputNames{};
        for (int h = 0; h < kHeadCount; ++h) {
            const int outputIndex = outputIndexByHead_[static_cast<size_t>(h)];
            selectedOutputNames[static_cast<size_t>(h)] = outputNames_[static_cast<size_t>(outputIndex)];
            outputNamePtrs[static_cast<size_t>(h)] = selectedOutputNames[static_cast<size_t>(h)].c_str();
        }

        auto outputs = session_->Run(
            Ort::RunOptions{nullptr},
            inputNamePtrs.data(),
            inputValues.data(),
            inputValues.size(),
            outputNamePtrs.data(),
            outputNamePtrs.size()
        );

        if (outputs.size() < static_cast<size_t>(kHeadCount)) {
            prediction.error = QStringLiteral("PhoBERT output count mismatch");
            return prediction;
        }

        prediction.action = decodeHead(Head::Action, outputs[0]);
        prediction.domain = decodeHead(Head::Domain, outputs[1]);
        prediction.op = decodeHead(Head::Op, outputs[2]);
        prediction.target = decodeHead(Head::Target, outputs[3]);
        prediction.execute = decodeHead(Head::Execute, outputs[4]);
        prediction.slotType = decodeHead(Head::SlotType, outputs[5]);
        prediction.ok = !prediction.action.label.isEmpty();

        if (cfg_.debug) {
            std::cout << "[PhoBERT] action=" << prediction.action.label.toStdString()
                      << " domain=" << prediction.domain.label.toStdString()
                      << " op=" << prediction.op.label.toStdString()
                      << " target=" << prediction.target.label.toStdString()
                      << " execute=" << prediction.execute.label.toStdString()
                      << " slot=" << prediction.slotType.label.toStdString()
                      << " conf=" << prediction.action.confidence
                      << "\n";
        }

        return prediction;
    } catch (const std::exception& e) {
        prediction.error = QStringLiteral("PhoBERT inference failed: %1")
                               .arg(QString::fromUtf8(e.what()));
        return prediction;
    }
}

QString PhoBertMultiHeadClassifier::headName(Head head)
{
    switch (head) {
    case Head::Action: return QStringLiteral("action_label");
    case Head::Domain: return QStringLiteral("domain_label");
    case Head::Op: return QStringLiteral("op_label");
    case Head::Target: return QStringLiteral("target_label");
    case Head::Execute: return QStringLiteral("execute_label");
    case Head::SlotType: return QStringLiteral("slot_type");
    case Head::Count: break;
    }
    return QStringLiteral("unknown");
}

} // namespace nlu
