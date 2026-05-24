#include "MediaService.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QStringList>
#include <QVariantList>

namespace {

struct DbusEndpoint {
    const char* service;
    const char* path;
    const char* interface;
};

constexpr DbusEndpoint kMediaEndpoint = {
    "org.agl.media",
    "/org/agl/media",
    "org.agl.media"
};

constexpr DbusEndpoint kSocialEndpoint = {
    "org.agl.social",
    "/org/agl/social",
    "org.agl.social"
};

bool callBoolMethod(const char* logPrefix,
                    const DbusEndpoint& endpoint,
                    const QString& method,
                    const QVariantList& args = {})
{
    struct CandidateBus {
        const char* name;
        QDBusConnection connection;
    };

    const CandidateBus buses[] = {
        {"system", QDBusConnection::systemBus()},
        {"session", QDBusConnection::sessionBus()}
    };

    QStringList errors;

    for (const CandidateBus& bus : buses) {
        if (!bus.connection.isConnected()) {
            errors.push_back(QStringLiteral("%1 bus unavailable: %2")
                                 .arg(QString::fromLatin1(bus.name),
                                      bus.connection.lastError().message()));
            continue;
        }

        QDBusInterface iface(
            QString::fromLatin1(endpoint.service),
            QString::fromLatin1(endpoint.path),
            QString::fromLatin1(endpoint.interface),
            bus.connection);

        if (!iface.isValid()) {
            errors.push_back(QStringLiteral("%1 bus invalid interface: %2")
                                 .arg(QString::fromLatin1(bus.name),
                                      iface.lastError().message()));
            continue;
        }

        const QDBusMessage reply = iface.callWithArgumentList(QDBus::Block, method, args);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            errors.push_back(QStringLiteral("%1 bus %2 failed: %3")
                                 .arg(QString::fromLatin1(bus.name),
                                      method,
                                      reply.errorMessage()));
            continue;
        }

        if (reply.arguments().isEmpty() || !reply.arguments().first().canConvert<bool>()) {
            qWarning() << logPrefix << method
                       << "returned unexpected D-Bus reply:"
                       << reply.arguments();
            return false;
        }

        const bool ok = reply.arguments().first().toBool();
        qInfo() << logPrefix << method
                << "via" << bus.name << "bus result =" << ok;
        return ok;
    }

    qWarning() << logPrefix << method
               << "D-Bus call failed for" << endpoint.service
               << errors;
    return false;
}

} // namespace

bool MediaService::executeAction(const QString& action, const nlu::SlotMap& slotValues)
{
    qInfo() << "[MediaService] EXECUTE action=" << action << "slotValues=" << slotValues;
    if (action == "MEDIA_PLAY") {
        return callBoolMethod("[MediaService]", kMediaEndpoint, QStringLiteral("Play"));
    }
    if (action == "MEDIA_STOP") {
        return callBoolMethod("[MediaService]", kMediaEndpoint, QStringLiteral("Stop"));
    }
    if (action == "MEDIA_NEXT_TRACK") {
        return callBoolMethod("[MediaService]", kMediaEndpoint, QStringLiteral("Next"));
    }
    if (action == "MEDIA_PREVIOUS_TRACK") {
        return callBoolMethod("[MediaService]", kMediaEndpoint, QStringLiteral("Previous"));
    }
    if (action == "MEDIA_VOLUME_UP") {
        return callBoolMethod("[MediaService]", kMediaEndpoint, QStringLiteral("VolumeUp"));
    }
    if (action == "MEDIA_VOLUME_DOWN") {
        return callBoolMethod("[MediaService]", kMediaEndpoint, QStringLiteral("VolumeDown"));
    }
    if (action == "MEDIA_SET_VOLUME") {
        const QVariant volumeValue = slotValues.value("volume_level");
        if (!volumeValue.isValid() || volumeValue.toString().isEmpty()) {
            qWarning() << "[MediaService] missing volume_level slot";
            return false;
        }

        const int volume = volumeValue.toInt();
        if (volume < 0 || volume > 100) {
            qWarning() << "[MediaService] volume_level out of range:" << volume;
            return false;
        }

        return callBoolMethod("[MediaService]",
                              kMediaEndpoint,
                              QStringLiteral("SetVolume"),
                              {volume});
    }
    if (action == "MEDIA_MUTE") {
        return callBoolMethod("[MediaService]", kMediaEndpoint, QStringLiteral("Mute"));
    }
    if (action == "MEDIA_UNMUTE") {
        return callBoolMethod("[MediaService]", kMediaEndpoint, QStringLiteral("Unmute"));
    }
    if (action == "MEDIA_PLAY_SONG") {
        const QString song = slotValues.value("song_name").toString().trimmed();
        if (song.isEmpty()) {
            qWarning() << "[MediaService] missing song_name slot";
            return false;
        }

        return callBoolMethod("[MediaService]",
                              kSocialEndpoint,
                              QStringLiteral("SearchMusicAndPlayFirst"),
                              {song});
    }
    if (action == "MEDIA_PLAY_ARTIST" || action == "MEDIA_LIST_BY_ARTIST") {
        const QString artist = slotValues.value("artist_name").toString().trimmed();
        if (artist.isEmpty()) {
            qWarning() << "[MediaService] missing artist_name slot";
            return false;
        }

        const QString method = action == "MEDIA_PLAY_ARTIST"
            ? QStringLiteral("SearchArtistWorksAndPlayFirst")
            : QStringLiteral("SearchArtistWorks");
        return callBoolMethod("[MediaService]", kSocialEndpoint, method, {artist});
    }
    qWarning() << "[MediaService] unknown action:" << action;
    return false;
}
