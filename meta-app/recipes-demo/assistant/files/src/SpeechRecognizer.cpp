#include "SpeechRecognizer.h"
#include <QtConcurrent>
#include <QDebug>
#include <QFile>
#include <vector>

// Hàm helper để đọc file WAV chuẩn (16-bit PCM, 16kHz)
bool readWavFile(const QString& filePath, std::vector<float>& pcmf32) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    
    // Bỏ qua 44 bytes header của file WAV để lấy raw data (PoC đơn giản)
    file.seek(44); 
    QByteArray rawData = file.readAll();
    file.close();

    const int16_t* pcm16 = reinterpret_cast<const int16_t*>(rawData.data());
    int frames = rawData.size() / sizeof(int16_t);
    
    pcmf32.resize(frames);
    for (int i = 0; i < frames; ++i) {
        pcmf32[i] = static_cast<float>(pcm16[i]) / 32768.0f; // Chuẩn hóa về -1.0 -> 1.0
    }
    return true;
}

SpeechRecognizer::SpeechRecognizer(QObject *parent) : QObject(parent)
{
    // Khởi tạo model từ file .bin trên Raspberry Pi 5
    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = false;
    
    m_ctx = whisper_init_from_file_with_params("/usr/share/models/phowhisper-base-q5_0.bin", cparams);
    if (m_ctx == nullptr) {
        qWarning() << "Lỗi: Không thể tải mô hình PhoWhisper!";
    }
}

SpeechRecognizer::~SpeechRecognizer() {
    if (m_ctx) whisper_free(m_ctx);
}

QString SpeechRecognizer::recognizedText() const { return m_recognizedText; }
bool SpeechRecognizer::isProcessing() const { return m_isProcessing; }

void SpeechRecognizer::setRecognizedText(const QString &text) {
    if (m_recognizedText != text) {
        m_recognizedText = text;
        emit textChanged();
    }
}

void SpeechRecognizer::setIsProcessing(bool processing) {
    if (m_isProcessing != processing) {
        m_isProcessing = processing;
        emit processingChanged();
    }
}

void SpeechRecognizer::processAudioFile(const QString &filePath)
{
    if (m_isProcessing || !m_ctx) return;

    setIsProcessing(true);
    setRecognizedText("Đang xử lý âm thanh...");

    // Chạy tác vụ nhận diện trên luồng nền (Background Thread)
    QtConcurrent::run([this, filePath]() {
        std::vector<float> pcmf32;
        if (!readWavFile(filePath, pcmf32)) {
            setRecognizedText("Lỗi: Không đọc được file audio.");
            setIsProcessing(false);
            return;
        }

        whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        wparams.language = "vi";
        wparams.n_threads = 4; // Ép Pi 5 dùng 4 nhân

        if (whisper_full(m_ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
            setRecognizedText("Lỗi: Suy luận thất bại.");
            setIsProcessing(false);
            return;
        }

        QString result = "";
        const int n_segments = whisper_full_n_segments(m_ctx);
        for (int i = 0; i < n_segments; ++i) {
            result += QString::fromUtf8(whisper_full_get_segment_text(m_ctx, i));
        }

        setRecognizedText(result.trimmed());
        setIsProcessing(false);
    });
}