/*
 * Copyright (C) 2019,2020,2022 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QDebug>

#include "map.h"

Map::Map(QObject * parent) :
    QObject(parent)
{
#if 0
    m_mloop = MessageEngineFactory::getInstance().getMessageEngine(url);
    QObject::connect(m_mloop.get(), &MessageEngine::connected, this, &Map::onConnected);
    QObject::connect(m_mloop.get(), &MessageEngine::disconnected, this, &Map::onDisconnected);
    QObject::connect(m_mloop.get(), &MessageEngine::messageReceived, this, &Map::onMessageReceived);
#endif
}

Map::~Map()
{
}

void Map::compose(QString recipient, QString message)
{
#if 0
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    CallMessage* btmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    parameter.insert("recipient", recipient);
    parameter.insert("message", message);
    btmsg->createRequest("bluetooth-map", "compose", parameter);
    m_mloop->sendMessage(std::move(msg));
#endif
}

void Map::message(QString handle)
{
#if 0
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    CallMessage* btmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    parameter.insert("handle", handle);
    btmsg->createRequest("bluetooth-map", "message", parameter);
    m_mloop->sendMessage(std::move(msg));
#endif
}

void Map::listMessages(QString folder)
{
#if 0
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    CallMessage* btmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    parameter.insert("folder", folder);
    btmsg->createRequest("bluetooth-map", "list_messages", parameter);
    m_mloop->sendMessage(std::move(msg));
#endif
}

#if 0
void Map::onConnected()
{

    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    CallMessage* btmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    parameter.insert("value", "notification");
    btmsg->createRequest("bluetooth-map", "subscribe", parameter);
    m_mloop->sendMessage(std::move(msg));

    listMessages();
}

void Map::onDisconnected()
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    CallMessage* btmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    parameter.insert("value", "notification");
    btmsg->createRequest("bluetooth-map", "unsubscribe", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Map::onMessageReceived(std::shared_ptr<Message> msg)
{
    if (msg->isEvent()) {
        std::shared_ptr<EventMessage> tmsg = std::static_pointer_cast<EventMessage>(msg);

        if (tmsg->eventApi() != "bluetooth-map")
            return;
        if (tmsg->eventName() == "notification") {
            emit notificationEvent(tmsg->eventData().toVariantMap());
        }
    } else if (msg->isReply()) {
        auto rmsg = std::static_pointer_cast<ResponseMessage>(msg);
        if (rmsg->requestVerb() == "list_messages") {
            QString folder = rmsg->requestParameters().value("folder").toString();
            QVariantMap listing = rmsg->replyData().value("messages").toObject().toVariantMap();
            emit listMessagesResult(folder, listing);
        } else if (rmsg->requestVerb() == "message") {
            QString handle = rmsg->requestParameters().value("handle").toString();
            emit messageResult(handle, rmsg->replyData().toVariantMap());
        }
    }
}
#endif
