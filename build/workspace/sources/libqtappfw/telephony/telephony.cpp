/*
 * Copyright (C) 2017-2020,2022 Konsulko Group
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

#include "telephony.h"


Telephony::Telephony(QObject * parent) :
	QObject(parent),
	m_connected(false),
	m_call_state("disconnected")
{
#if 0
	m_mloop = MessageEngineFactory::getInstance().getMessageEngine(url);
	QObject::connect(m_mloop.get(), &MessageEngine::connected, this, &Telephony::onConnected);
	QObject::connect(m_mloop.get(), &MessageEngine::disconnected, this, &Telephony::onDisconnected);
	QObject::connect(m_mloop.get(), &MessageEngine::messageReceived, this, &Telephony::onMessageReceived);
#endif
}

Telephony::~Telephony()
{
}

void Telephony::dial(QString number)
{
#if 0
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
	QJsonObject parameter;

	parameter.insert("value", number);
	tmsg->createRequest("telephony", "dial", parameter);
	m_mloop->sendMessage(std::move(msg));
#endif
}

void Telephony::answer()
{
#if 0
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
	tmsg->createRequest("telephony", "answer");
	m_mloop->sendMessage(std::move(msg));
#endif
}

void Telephony::hangup()
{
#if 0
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
	tmsg->createRequest("telephony", "hangup");
	m_mloop->sendMessage(std::move(msg));
#endif
}

#if 0
void Telephony::onConnected()
{
	QStringList events {
		"callStateChanged",
		"dialingCall",
		"incomingCall",
		"terminatedCall",
		"online"};
	QStringListIterator eventIterator(events);

	while (eventIterator.hasNext()) {
		std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
		if (!msg)
			return;

		CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
		QJsonObject parameter;
		parameter.insert("value", eventIterator.next());
		tmsg->createRequest("telephony", "subscribe", parameter);
		m_mloop->sendMessage(std::move(msg));
	}

	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
	tmsg->createRequest("Bluetooth-Manager", "adapter_state", QJsonObject());
	m_mloop->sendMessage(std::move(msg));
	//make ui available, while waiting for connection status,
	//most likely profile is already connected
	setConnected(true);
}

void Telephony::onDisconnected()
{
	setConnected(false);
}

void Telephony::onMessageReceived(std::shared_ptr<Message> msg)
{
	if (!msg)
		return;

	if (msg->isEvent()) {
		std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
		if (emsg->eventApi() != "telephony")
			return;
		QString ename = emsg->eventName();
		QJsonObject data = emsg->eventData();
		if (ename == "callStateChanged") {
			setCallState(data.find("state").value().toString());
		} else if (ename == "dialingCall") {
			m_colp = data.find("colp").value().toString();
				setCallState("dialing");
		} else if (ename == "incomingCall") {
			m_clip = data.find("clip").value().toString();
				setCallState("incoming");
		} else if (ename == "terminatedCall") {
				setCallState("disconnected");
				m_colp = "";
				m_clip = "";
		} else if (ename == "online") {
			setOnlineState(data.find("connected").value().toBool());
		}
	}
	else if (msg->isReply()) {
		std::shared_ptr<ResponseMessage> rmsg = std::static_pointer_cast<ResponseMessage>(msg);
		QString verb = rmsg->requestVerb();
		QJsonObject data = rmsg->replyData();
		if (rmsg->replyStatus() == "failed") {
			qDebug() << "phone failed bt verb:" << verb;
			if ((verb == "adapter_state") &&
			    (rmsg->replyInfo().contains("No adapter")))
				setConnected(false);
			else if (verb == "dial") {
				setCallState("disconnected");
				m_colp = "";
				m_clip = "";
			}
		}
	} else
		qWarning() << "Received invalid inbound message";

}
#endif
