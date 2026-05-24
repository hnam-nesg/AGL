#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QJsonArray>
#include <QJsonObject>

namespace vshl {
const QString API = QLatin1String("vshl-core");
const QString VOICE_AGENT_ENUMERATION_VERB = QLatin1String("enumerateVoiceAgents");
const QString SUBSCRIBE_VERB = QLatin1String("subscribe");
const QString TAP_TO_TALK_VERB = QLatin1String("startListening");

const QString ALEXA_AGENT_NAME = QLatin1String("Alexa");

const QString DATA_TAG = QLatin1String("data");
const QString RESPONSE_TAG = QLatin1String("response");
const QString AGENTS_TAG = QLatin1String("agents");
const QString DEFAULT_TAG = QLatin1String("default");
const QString NAME_TAG = QLatin1String("name");
const QString ID_TAG = QLatin1String("id");
const QString STATE_TAG = QLatin1String("state");

const QString VOICE_AGENT_ID_ARG = QLatin1String("va_id");
const QString VOICE_AGENT_EVENTS_ARG = QLatin1String("events");
const QString VOICE_AGENT_ACTIONS_ARG = QLatin1String("actions");

const QJsonArray VOICE_AGENT_EVENTS_ARRAY = {
    QLatin1String("voice_authstate_event"),
    QLatin1String("voice_dialogstate_event"),
    QLatin1String("voice_connectionstate_event")
};

const QString VOICE_DIALOG_STATE_EVENT = QLatin1String("vshl-core/voice_dialogstate_event#");
const QString VOICE_DIALOG_IDLE = QLatin1String("IDLE");
const QString VOICE_DIALOG_LISTENING = QLatin1String("LISTENING");
const QString VOICE_DIALOG_THINKING = QLatin1String("THINKING");
const QString VOICE_DIALOG_SPEAKING = QLatin1String("SPEAKING");
const QString VOICE_DIALOG_MICROPHONEOFF = QLatin1String("MICROPHONEOFF");
}

#endif // CONSTANTS_H
