#pragma once

#include "NluTypes.h"

#include <onnxruntime_cxx_api.h>

#include <QHash>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QStringList>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace nlu {

class PhoBertMultiHeadClassifier {
public:
    struct Config {
        bool enabled = false;

        std::string model_path = "/usr/share/assistant/models/phobert_multihead_int8.onnx";
        std::string vocab_path = "/usr/share/assistant/models/phobert_vocab.txt";
        std::string bpe_codes_path = "/usr/share/assistant/models/phobert_bpe.codes";
        QString label_map_path = QStringLiteral("/usr/share/assistant/data/nlu/multihead_label_map_v4.json");

        int max_seq_len = 64;
        int intra_threads = 2;
        bool debug = false;
    };

    struct HeadResult {
        QString label;
        int index = -1;
        double confidence = 0.0;
    };

    struct Prediction {
        bool ok = false;
        QString error;
        QString normalizedText;

        HeadResult action;
        HeadResult domain;
        HeadResult op;
        HeadResult target;
        HeadResult execute;
        HeadResult slotType;
    };

public:
    PhoBertMultiHeadClassifier();
    explicit PhoBertMultiHeadClassifier(const Config& cfg);

    bool initialize(QString* errorMessage = nullptr);
    bool isEnabled() const { return cfg_.enabled; }
    bool isInitialized() const { return session_ != nullptr; }

    Prediction predict(const QString& text);

    enum class Head {
        Action = 0,
        Domain,
        Op,
        Target,
        Execute,
        SlotType,
        Count
    };

private:
    struct EncodedInput {
        std::vector<int64_t> inputIds;
        std::vector<int64_t> attentionMask;
        std::vector<int64_t> tokenTypeIds;
    };

private:
    bool loadVocab(QString* errorMessage);
    bool loadBpeCodes(QString* errorMessage);
    bool loadLabelMap(QString* errorMessage);
    bool inspectModelIO(QString* errorMessage);

    EncodedInput encode(const QString& text) const;
    QStringList tokenize(const QString& text) const;
    QStringList bpeTokenizeWord(const QString& word) const;

    HeadResult decodeHead(Head head, Ort::Value& output) const;
    const QStringList& labelsFor(Head head) const;

    static QString normalizeText(QString text);
    static QString headName(Head head);
    static double softmaxConfidence(const std::vector<double>& logits, int index);
    static bool readJsonStringArray(const QJsonObject& object,
                                    const QStringList& candidateKeys,
                                    QStringList* out);

private:
    Config cfg_;
    int maxSeqLen_ = 128;

    Ort::Env env_{ORT_LOGGING_LEVEL_WARNING, "phobert_multihead"};
    Ort::SessionOptions sessionOptions_;
    std::unique_ptr<Ort::Session> session_;

    std::vector<std::string> inputNames_;
    std::vector<std::string> outputNames_;
    std::array<int, static_cast<size_t>(Head::Count)> outputIndexByHead_{};

    QString inputIdsName_;
    QString attentionMaskName_;
    QString tokenTypeIdsName_;
    bool hasAttentionMask_ = false;
    bool hasTokenTypeIds_ = false;

    QHash<QString, int64_t> vocab_;
    QHash<QString, int> bpeRanks_;

    int64_t clsId_ = 0;
    int64_t sepId_ = 2;
    int64_t padId_ = 1;
    int64_t unkId_ = 3;
    int64_t maxVocabId_ = 0;

    std::array<QStringList, static_cast<size_t>(Head::Count)> labels_;
};

} // namespace nlu
