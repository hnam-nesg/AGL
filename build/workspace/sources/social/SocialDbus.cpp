#include "SocialDbus.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDebug>

namespace {
constexpr auto kMediaServiceName = "org.agl.media";
constexpr auto kMediaObjectPath = "/org/agl/media";
constexpr auto kMediaInterfaceName = "org.agl.media";
}

SocialDbus::SocialDbus(QObject *parent)
    : QObject(parent)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qWarning() << "Cannot connect to system D-Bus for media coordination:"
                   << bus.lastError().message();
        return;
    }

    const bool connected = bus.connect(
        QString(),
        QString::fromLatin1(kMediaObjectPath),
        QString::fromLatin1(kMediaInterfaceName),
        QStringLiteral("StateChanged"),
        this,
        SLOT(onMediaStateChanged(QVariantMap)));

    if (!connected) {
        qWarning() << "Cannot subscribe to org.agl.media StateChanged:"
                   << bus.lastError().message();
    }
}

QString SocialDbus::Ping() const
{
    return QStringLiteral("org.agl.social");
}

bool SocialDbus::Search(const QString &type, const QString &query, bool autoClickFirst)
{
    return requestSearch(type, query, autoClickFirst);
}

bool SocialDbus::SearchMusic(const QString &query)
{
    return requestSearch(QStringLiteral("music"), query, false);
}

bool SocialDbus::SearchMovie(const QString &query)
{
    return requestSearch(QStringLiteral("movie"), query, false);
}

bool SocialDbus::SearchArtistWorks(const QString &artist)
{
    return requestSearch(QStringLiteral("artistWorks"), artist, false);
}

bool SocialDbus::SearchMusicAndPlayFirst(const QString &query)
{
    return requestSearch(QStringLiteral("music"), query, true);
}

bool SocialDbus::SearchMovieAndPlayFirst(const QString &query)
{
    return requestSearch(QStringLiteral("movie"), query, true);
}

bool SocialDbus::SearchArtistWorksAndPlayFirst(const QString &artist)
{
    return requestSearch(QStringLiteral("artistWorks"), artist, true);
}

bool SocialDbus::SetYoutubePlaying(bool playing)
{
    if (m_youtubePlaying == playing)
        return true;

    m_youtubePlaying = playing;
    emit youtubePlaybackChanged(playing);

    if (playing)
        return PauseMediaPlayer();

    return true;
}

bool SocialDbus::Pause()
{
    return PauseYoutube();
}

bool SocialDbus::Stop()
{
    return PauseYoutube();
}

bool SocialDbus::PauseYoutube()
{
    if (m_youtubePlaying) {
        m_youtubePlaying = false;
        emit youtubePlaybackChanged(false);
    }

    emit pauseYoutubeRequested();
    return true;
}

bool SocialDbus::PauseMediaPlayer()
{
    const QDBusMessage message = QDBusMessage::createMethodCall(
        QString::fromLatin1(kMediaServiceName),
        QString::fromLatin1(kMediaObjectPath),
        QString::fromLatin1(kMediaInterfaceName),
        QStringLiteral("Pause"));

    auto pauseOnBus = [&](const QDBusConnection &bus, const QString &busName) {
        if (!bus.isConnected()) {
            qWarning() << "Cannot pause org.agl.media on" << busName
                       << "D-Bus:" << bus.lastError().message();
            return false;
        }

        const QDBusMessage reply = bus.call(message, QDBus::Block, 1000);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            if (reply.errorName() != QLatin1String("org.freedesktop.DBus.Error.ServiceUnknown")
                && reply.errorName() != QLatin1String("org.freedesktop.DBus.Error.NameHasNoOwner")) {
                qWarning() << "Failed to pause org.agl.media on" << busName
                           << "D-Bus:" << reply.errorName() << reply.errorMessage();
            }
            return false;
        }

        return true;
    };

    if (pauseOnBus(QDBusConnection::systemBus(), QStringLiteral("system")))
        return true;

    return pauseOnBus(QDBusConnection::sessionBus(), QStringLiteral("session"));
}

bool SocialDbus::requestSearch(const QString &type, const QString &query, bool autoClickFirst)
{
    const QString trimmedType = type.trimmed();
    const QString trimmedQuery = query.trimmed();

    if (trimmedType.isEmpty() || trimmedQuery.isEmpty())
        return false;

    if (autoClickFirst)
        PauseMediaPlayer();

    emit searchRequested(trimmedType, trimmedQuery, autoClickFirst);
    return true;
}

void SocialDbus::handleMediaState(const QVariantMap &state)
{
    const QString status = state.value(QStringLiteral("status")).toString().toLower();
    if (status == QLatin1String("playing"))
        PauseYoutube();
}

void SocialDbus::onMediaStateChanged(const QVariantMap &state)
{
    handleMediaState(state);
}
