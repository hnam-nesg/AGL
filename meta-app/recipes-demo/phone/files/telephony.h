#ifndef TELEPHONY_H
#define TELEPHONY_H

#include <QObject>
#include <QDBusObjectPath>
#include <QVariantMap>

class Telephony : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString callState READ callState WRITE setCallState NOTIFY callStateChanged)
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)
    Q_PROPERTY(QString callClip READ callClip NOTIFY callClipChanged)
    Q_PROPERTY(QString callColp READ callColp NOTIFY callColpChanged)

public:
    explicit Telephony(QObject *parent = nullptr);
    virtual ~Telephony();

    Q_INVOKABLE void dial(QString number);
    Q_INVOKABLE void answer();
    Q_INVOKABLE void hangup();

    void setConnected(bool state);
    void setCallState(const QString &callState);
    void setOnlineState(bool state);
    void setCallClip(const QString &clip);
    void setCallColp(const QString &colp);

    bool connected() const { return m_connected; }
    bool online() const { return m_online; }
    QString callState() const { return m_call_state; }
    QString callClip() const { return m_clip; }
    QString callColp() const { return m_colp; }

signals:
    void connectedChanged(bool);
    void callStateChanged(QString);
    void onlineChanged(bool);
    void callClipChanged(QString);
    void callColpChanged(QString);

private slots:
    void onCallAdded(const QDBusObjectPath &path, const QVariantMap &properties);
    void onCallRemoved(const QDBusObjectPath &path);

private:
    bool m_connected = false;
    bool m_online = false;
    QString m_call_state = "disconnected";
    QString m_clip;
    QString m_colp;
    QString m_currentCallPath;
    QString m_voiceCallManagerPath;
};

#endif // TELEPHONY_H