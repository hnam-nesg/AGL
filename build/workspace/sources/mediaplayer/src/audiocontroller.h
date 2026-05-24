#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#include <QObject>
#include <QString>

class AudioController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setVolumeMute NOTIFY mutedChanged)
    Q_PROPERTY(QString appNameHint READ appNameHint WRITE setAppNameHint NOTIFY appNameHintChanged)
    Q_PROPERTY(QString deviceAddress READ deviceAddress WRITE setDeviceAddress NOTIFY deviceAddressChanged)

public:
    explicit AudioController(QObject *parent = nullptr);

    double volume() const;
    bool muted() const;
    QString appNameHint() const;
    QString deviceAddress() const;

    Q_INVOKABLE void setVolume(double value);
    Q_INVOKABLE void setVolumeMute(bool mute);
    Q_INVOKABLE void setAppNameHint(const QString &name);
    Q_INVOKABLE void setDeviceAddress(const QString &address);
    Q_INVOKABLE QString resolveTargetId();
    Q_INVOKABLE void refreshTarget();

signals:
    void volumeChanged();
    void mutedChanged();
    void appNameHintChanged();
    void deviceAddressChanged();
    void targetChanged();

private:
    QString runCommand(const QString &program, const QStringList &args) const;
    QString findBluetoothNodeIdByAddress() const;
    QString findStreamIdByHint() const;
    bool inspectMatches(const QString &id) const;

private:
    double m_volume = 0.5;                  // 0.0 -> 1.0
    bool m_muted = false;
    QString m_appNameHint = "mediaplayer";  // đổi nếu app name khác
    QString m_deviceAddress;                // MAC dạng 88:A4:...
    QString m_targetId;                     // stream id runtime
};

#endif // AUDIOCONTROLLER_H
