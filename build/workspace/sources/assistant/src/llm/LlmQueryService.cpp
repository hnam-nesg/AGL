#include "LlmQueryService.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

#include <cmath>
#include <memory>
#include <utility>

namespace llm {
namespace {

bool isValidReasoningEffort(const QString& value)
{
    return value == QStringLiteral("low") ||
           value == QStringLiteral("medium") ||
           value == QStringLiteral("high");
}

} // namespace

LlmQueryService::LlmQueryService()
    : LlmQueryService(Config{})
{
}

LlmQueryService::LlmQueryService(Config cfg)
    : cfg_(std::move(cfg))
{
}

bool LlmQueryService::isEnabled() const
{
    return cfg_.enabled;
}

QString LlmQueryService::apiKey() const
{
    return cfg_.api_key.trimmed();
}

bool LlmQueryService::isReady(QString* errorMessage) const
{
    if (!cfg_.enabled) {
        return true;
    }

    if (!QUrl(cfg_.endpoint_url).isValid()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid Groq endpoint URL: %1").arg(cfg_.endpoint_url);
        }
        return false;
    }

    if (cfg_.model.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Groq model is empty");
        }
        return false;
    }

    if (apiKey().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Missing Groq API key. Set cfg.llm_cfg.api_key.");
        }
        return false;
    }

    return true;
}

QString LlmQueryService::buildUserMessage(const Query& query) const
{
    QString message;
    message += QStringLiteral("Câu người dùng: ");
    message += query.text.trimmed();

    message += QStringLiteral("\n\nNgữ cảnh NLU:");
    if (!query.normalizedText.isEmpty()) {
        message += QStringLiteral("\n- normalized_text: %1").arg(query.normalizedText);
    }
    if (!query.domain.isEmpty()) message += QStringLiteral("\n- domain: %1").arg(query.domain);
    if (!query.op.isEmpty()) message += QStringLiteral("\n- op: %1").arg(query.op);
    if (!query.target.isEmpty()) message += QStringLiteral("\n- target: %1").arg(query.target);
    if (!query.slotType.isEmpty()) message += QStringLiteral("\n- slot_type: %1").arg(query.slotType);

    if (!query.slotValues.isEmpty()) {
        message += QStringLiteral("\n- slot_values:");
        for (auto it = query.slotValues.constBegin(); it != query.slotValues.constEnd(); ++it) {
            message += QStringLiteral(" %1=%2;").arg(it.key(), it.value().toString());
        }
    }

    // message += QStringLiteral(
    //     "\n\nYêu cầu trả lời: dùng tiếng Việt tự nhiên; nếu có thuật ngữ, tên riêng hoặc chữ viết tắt tiếng Anh thì thêm cách đọc phiên âm tiếng Việt trong ngoặc ở lần đầu; trả lời ngắn gọn để đọc qua loa trong xe."
    // );
    return message;
}

QString LlmQueryService::cleanupOutput(QString output) const
{
    output = output.trimmed();

    const QStringList stopMarkers = {
        QStringLiteral("<|user|>"),
        QStringLiteral("<|system|>"),
        QStringLiteral("<|assistant|>"),
        QStringLiteral("Người dùng:"),
        QStringLiteral("User:"),
        QStringLiteral("Assistant:"),
        QStringLiteral("Trợ lý:")
    };

    for (const QString& marker : stopMarkers) {
        const int idx = output.indexOf(marker, 1, Qt::CaseInsensitive);
        if (idx > 0) {
            output = output.left(idx).trimmed();
        }
    }

    return output.trimmed();
}

QString LlmQueryService::answer(const Query& query, QString* errorMessage) const
{
    if (!cfg_.enabled) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("LLM is disabled");
        }
        return {};
    }

    QString readyError;
    if (!isReady(&readyError)) {
        if (errorMessage) {
            *errorMessage = readyError;
        }
        return {};
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), cfg_.model);
    payload.insert(QStringLiteral("temperature"), cfg_.temperature);
    payload.insert(QStringLiteral("max_completion_tokens"), cfg_.max_completion_tokens);
    payload.insert(QStringLiteral("include_reasoning"), cfg_.include_reasoning);
    payload.insert(QStringLiteral("stream"), false);

    if (isValidReasoningEffort(cfg_.reasoning_effort)) {
        payload.insert(QStringLiteral("reasoning_effort"), cfg_.reasoning_effort);
    }

    QJsonArray messages;
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("system")},
        {QStringLiteral("content"), cfg_.system_prompt}
    });
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("user")},
        {QStringLiteral("content"), buildUserMessage(query)}
    });
    payload.insert(QStringLiteral("messages"), messages);

    QNetworkRequest request(QUrl(cfg_.endpoint_url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(apiKey()).toUtf8());

    QNetworkAccessManager manager;
    std::unique_ptr<QNetworkReply> reply(manager.post(
        request,
        QJsonDocument(payload).toJson(QJsonDocument::Compact)
    ));

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply.get(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(cfg_.timeout_ms);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
    } else {
        reply->abort();
        if (errorMessage) {
            *errorMessage = QStringLiteral("Groq request timeout after %1 ms").arg(cfg_.timeout_ms);
        }
        return {};
    }

    const QByteArray body = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Groq request failed: %1; body=%2")
                                .arg(reply->errorString(), QString::fromUtf8(body).trimmed());
        }
        return {};
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid Groq JSON response: %1").arg(parseError.errorString());
        }
        return {};
    }

    const QJsonObject root = doc.object();
    if (root.contains(QStringLiteral("error"))) {
        const QJsonObject errorObj = root.value(QStringLiteral("error")).toObject();
        const QString message = errorObj.value(QStringLiteral("message")).toString();
        if (errorMessage) {
            *errorMessage = message.isEmpty()
                ? QStringLiteral("Groq returned an error")
                : message;
        }
        return {};
    }

    const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Groq response has no choices");
        }
        return {};
    }

    const QJsonObject message = choices.first().toObject()
        .value(QStringLiteral("message")).toObject();
    const QString content = message.value(QStringLiteral("content")).toString();
    const QString answerText = cleanupOutput(content);
    if (answerText.isEmpty() && errorMessage) {
        *errorMessage = QStringLiteral("Groq returned empty content");
    }
    return answerText;
}

} // namespace llm
