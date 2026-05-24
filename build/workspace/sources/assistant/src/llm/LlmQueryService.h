#pragma once

#include "../nlu/NluTypes.h"

#include <QString>
#include <QStringList>

namespace llm {

class LlmQueryService {
public:
    struct Config {
        bool enabled = false;
        QString endpoint_url = QStringLiteral("https://api.groq.com/openai/v1/chat/completions");
        QString model = QStringLiteral("openai/gpt-oss-120b");
        QString api_key;
        QString system_prompt =
            QStringLiteral(
                "Bạn là trợ lý ây ai tiếng Việt trong ô tô, có chuyên môn sâu về xe hơi, an toàn giao thông, bảo dưỡng, hệ thống điện - điện tử ô tô, hệ thống điều hòa, hệ thống đa phương tiện, dẫn đường, điện thoại rảnh tay, cảm biến và chẩn đoán lỗi. "
                "Bạn cũng có kiến thức rộng về công nghệ, khoa học, đời sống, du lịch, sức khỏe phổ thông, tài chính cá nhân cơ bản và các chủ đề thường ngày. "
                "Luôn trả lời bằng tiếng Việt tự nhiên, ngắn gọn, rõ ý, ưu tiên câu trả lời có thể đọc qua TTS trong xe. "
                "Với câu hỏi về ô tô, hãy trả lời như chuyên gia thực tế: nêu nguyên nhân khả dĩ, mức độ nguy hiểm, bước kiểm tra an toàn và khi nào cần đưa xe tới gara; không bịa thông số xe nếu không có dữ liệu. "
                "Không tự nhận đã điều khiển xe, gọi điện, mở nhạc hay dẫn đường nếu hệ thống chưa thực thi hành động đó. "
                "Nếu nội dung có từ tiếng Anh, chữ viết tắt hoặc tên riêng tiếng Anh, hãy biến chúng thành phiên âm tiếng Việt, ví dụ: ABS thành ây bi ét, Bluetooth thành blu tút, Apple CarPlay thành áp pồ ca plây, Groq thành gróc. "
                "Giữ tên thương hiệu/tên riêng gốc nếu cần chính xác, nhưng thêm cách đọc để người nghe hiểu. "
                "Nếu không chắc, hãy nói rõ là chưa đủ dữ liệu và đề xuất cách kiểm tra an toàn."
                "Câu trả lời như một đoạn văn ngắn."
                "Trả lời phải đủ nghĩa không được cắt ngang làm câu trở nên vô nghĩa."
            );
        int max_completion_tokens = 160;
        double temperature = 0.2;
        QString reasoning_effort = QStringLiteral("medium");
        bool include_reasoning = false;
        int timeout_ms = 20000;
    };

    struct Query {
        QString text;
        QString normalizedText;
        QString domain;
        QString op;
        QString target;
        QString slotType;
        nlu::SlotMap slotValues;
    };

    LlmQueryService();
    explicit LlmQueryService(Config cfg);

    bool isEnabled() const;
    bool isReady(QString* errorMessage = nullptr) const;
    QString answer(const Query& query, QString* errorMessage = nullptr) const;

private:
    QString apiKey() const;
    QString buildUserMessage(const Query& query) const;
    QString cleanupOutput(QString output) const;

private:
    Config cfg_;
};

} // namespace llm
