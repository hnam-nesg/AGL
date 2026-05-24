#ifndef SPEECHRECOGNIZER_H
#define SPEECHRECOGNIZER_H

#include <QObject>
#include <QString>
#include <whisper.h>

class SpeechRecognizer : public QObject
{
    Q_OBJECT
    // Q_PROPERTY giúp QML tự động cập nhật UI khi biến thay đổi
    Q_PROPERTY(QString recognizedText READ recognizedText NOTIFY textChanged)
    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY processingChanged)

public:
    explicit SpeechRecognizer(QObject *parent = nullptr);
    ~SpeechRecognizer();

    QString recognizedText() const;
    bool isProcessing() const;

    // Hàm gọi từ QML khi bấm nút
    Q_INVOKABLE void processAudioFile(const QString &filePath);

signals:
    void textChanged();
    void processingChanged();

private:
    struct whisper_context * m_ctx = nullptr;
    QString m_recognizedText;
    bool m_isProcessing = false;

    void setRecognizedText(const QString &text);
    void setIsProcessing(bool processing);
};

#endif // SPEECHRECOGNIZER_H