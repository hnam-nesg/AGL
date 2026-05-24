#ifndef ASRMANAGER_H
#define ASRMANAGER_H

#include <QObject>
#include <QThread>
#include <QStringList>
#include <QAudioSource>
#include <QIODevice>

class AsrWorker;

class AsrManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString partialText READ partialText NOTIFY partialTextChanged)
    Q_PROPERTY(QString finalText READ finalText NOTIFY finalTextChanged)
    Q_PROPERTY(QStringList transcriptLines READ transcriptLines NOTIFY transcriptLinesChanged)
    Q_PROPERTY(bool listening READ listening NOTIFY listeningChanged)
    Q_PROPERTY(bool engineReady READ engineReady NOTIFY engineReadyChanged)
    Q_PROPERTY(QString modelPath READ modelPath WRITE setModelPath NOTIFY modelPathChanged)
    Q_PROPERTY(QString audioDeviceName READ audioDeviceName NOTIFY audioDeviceNameChanged)

public:
    explicit AsrManager(QObject *parent = nullptr);
    ~AsrManager() override;

    QString state() const;
    QString partialText() const;
    QString finalText() const;
    QStringList transcriptLines() const;
    bool listening() const;
    bool engineReady() const;
    QString modelPath() const;
    QString audioDeviceName() const;

    void setModelPath(const QString &path);

    Q_INVOKABLE void initializeEngine();
    Q_INVOKABLE void startListening();
    Q_INVOKABLE void stopListening();
    Q_INVOKABLE void clearTranscript();

signals:
    void stateChanged();
    void partialTextChanged();
    void finalTextChanged();
    void transcriptLinesChanged();
    void listeningChanged();
    void engineReadyChanged();
    void modelPathChanged();
    void audioDeviceNameChanged();
    void errorOccurred(const QString &message);

    void initializeWorker(const QString &modelPath);
    void audioChunkReady(const QByteArray &pcmChunk);
    void resetWorker();
    void stopWorker();

private slots:
    void onAudioReadyRead();
    void onWorkerInitializedChanged(bool ready);
    void onPartialResultReady(const QString &text);
    void onFinalResultReady(const QString &text);
    void onWorkerError(const QString &message);

private:
    void setupWorker();
    bool setupAudioInput();
    void setState(const QString &newState);

    AsrWorker *m_worker = nullptr;
    QThread m_workerThread;

    QAudioSource *m_audioSource = nullptr;
    QIODevice *m_audioDevice = nullptr;
    QByteArray m_pendingAudio;

    QString m_state = QStringLiteral("Idle");
    QString m_partialText;
    QString m_finalText;
    QStringList m_transcriptLines;
    bool m_listening = false;
    bool m_engineReady = false;
    QString m_modelPath = QStringLiteral("/usr/share/models/phowhisper-base-q5_0.bin");
    QString m_audioDeviceName;
    bool m_pendingStartRequested = false;

    static constexpr int kSampleRate = 16000;
    static constexpr int kChannels = 1;
    static constexpr int kBytesPerSample = 2;
    static constexpr int kChunkMilliseconds = 500;
};

#endif // ASRMANAGER_H
