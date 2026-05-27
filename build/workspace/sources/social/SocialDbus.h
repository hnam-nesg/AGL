#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class SocialDbus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.agl.social")

public:
    explicit SocialDbus(QObject *parent = nullptr);

public slots:
    QString Ping() const;
    bool Search(const QString &type, const QString &query, bool autoClickFirst);
    bool SearchMusic(const QString &query);
    bool SearchMovie(const QString &query);
    bool SearchArtistWorks(const QString &artist);
    bool SearchMusicAndPlayFirst(const QString &query);
    bool SearchMovieAndPlayFirst(const QString &query);
    bool SearchArtistWorksAndPlayFirst(const QString &artist);
    bool SetYoutubePlaying(bool playing);
    bool Pause();
    bool Stop();
    bool PauseYoutube();
    bool PauseMediaPlayer();

signals:
    void searchRequested(const QString &type, const QString &query, bool autoClickFirst);
    void pauseYoutubeRequested();
    void youtubePlaybackChanged(bool playing);

private:
    bool requestSearch(const QString &type, const QString &query, bool autoClickFirst);
    void handleMediaState(const QVariantMap &state);

private slots:
    void onMediaStateChanged(const QVariantMap &state);

private:
    bool m_youtubePlaying = false;
};
