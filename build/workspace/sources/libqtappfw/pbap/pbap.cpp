/*
 * Copyright (C) 2018-2020,2022 Konsulko Group
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
#include <QMetaEnum>
#include <QMimeDatabase>
#include <QtQml/QQmlEngine>
#include <QJsonObject>

#include "pbap.h"


PhoneNumber::PhoneNumber(QString number, QString type)
{
    m_number = number;
    m_type = stringToEnum(type);
}

PhoneNumber::~PhoneNumber()
{
}

int PhoneNumber::stringToEnum(QString key)
{
    const QMetaObject* metaObject = PhoneNumber::metaObject();
    int enumIndex = metaObject->indexOfEnumerator("PhoneNumberType");
    QMetaEnum mEnum = metaObject->enumerator(enumIndex);

    int value = mEnum.keyToValue(key.toUtf8().data());
    return (value < 0) ? 0 : value;
}

Contact::Contact(QString name, QString photo, QList<PhoneNumber *>numbers)
{
    m_name = name;
    m_photo = photo;
    m_numbers = numbers;
}

Contact::~Contact()
{
}

RecentCall::RecentCall(QString name, QString number, QString datetime, QString type)
{
    m_name = name;
    m_number = number;
    m_datetime = datetime;
    m_type = stringToEnum(type);
}

RecentCall::~RecentCall()
{
}

int RecentCall::stringToEnum(QString key)
{
    const QMetaObject* metaObject = RecentCall::metaObject();
    int enumIndex = metaObject->indexOfEnumerator("RecentCallType");
    QMetaEnum mEnum = metaObject->enumerator(enumIndex);

    int value = mEnum.keyToValue(key.toUtf8().data());
    return (value < 0) ? 0 : value;
}

Pbap::Pbap (QQmlContext *context, QObject * parent) :
    QObject(parent)
{
#if 0
    m_mloop = MessageEngineFactory::getInstance().getMessageEngine(url);
#endif
    m_context = context;
    m_context->setContextProperty("ContactsModel", QVariant::fromValue(m_contacts));
    qmlRegisterUncreatableType<PhoneNumber>("PhoneNumber", 1, 0, "PhoneNumber", "Enum");
    m_context->setContextProperty("RecentCallModel", QVariant::fromValue(m_calls));
    qmlRegisterUncreatableType<RecentCall>("RecentCall", 1, 0, "RecentCall", "Enum");

#if 0
    QObject::connect(m_mloop.get(), &MessageEngine::connected, this, &Pbap::onConnected);
    QObject::connect(m_mloop.get(), &MessageEngine::disconnected, this, &Pbap::onDisconnected);
    QObject::connect(m_mloop.get(), &MessageEngine::messageReceived, this, &Pbap::onMessageReceived);
#endif
}

Pbap::~Pbap()
{
}

void Pbap::importContacts(int max_entries)
{
#if 0
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* pmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    pmsg->createRequest("bluetooth-pbap", "import", parameter);
    m_mloop->sendMessage(std::move(msg));
#endif
}

void Pbap::refreshContacts(int max_entries)
{
#if 0
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* pmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    pmsg->createRequest("bluetooth-pbap", "contacts", parameter);
    m_mloop->sendMessage(std::move(msg));
#endif
}

void Pbap::refreshCalls(int max_entries)
{
#if 0
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* pmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("list", "cch");
    if (max_entries >= 0)
        parameter.insert("max_entries", max_entries);

    pmsg->createRequest("bluetooth-pbap", "history", parameter);
    m_mloop->sendMessage(std::move(msg));
#endif
}

void Pbap::search(QString number)
{
#if 0
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* pmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    if (!number.isEmpty())
        parameter.insert("number", number);
    parameter.insert("max_entries", 1);

    pmsg->createRequest("bluetooth-pbap", "search", parameter);
    m_mloop->sendMessage(std::move(msg));
#endif
}

bool compareContactPtr(QObject *a, QObject *b)
{
    Contact *contactA = qobject_cast<Contact *>(a);
    Contact *contactB = qobject_cast<Contact *>(b);

    return (*contactA < *contactB);
}

void Pbap::updateContacts(QJsonArray vcards)
{
    m_contacts.clear();

    for (auto vcard : vcards) {
        QJsonObject entry = vcard.toObject();
        QString image;

        QString name = entry.value("fn").toString();
        QList<PhoneNumber *> numbers;

        for (auto number: entry.value("telephone").toArray()) {
            QString type = number.toObject().keys().at(0);
            QString tel = number.toObject().value(type).toString();
            numbers.append(new PhoneNumber(tel, type));
        }

        if (entry.contains("photo")) {
            QJsonObject photo = entry.value("photo").toObject();
            QString mimetype = photo.value("mimetype").toString();
            QString mimedata = photo.value("data").toString();
            image = "data:" + mimetype + ";base64," + mimedata;
        }

        if (!numbers.isEmpty())
            m_contacts.append(new Contact(name, image, numbers));
    }

    std::sort(m_contacts.begin(), m_contacts.end(), compareContactPtr);

    // Refresh model
    m_context->setContextProperty("ContactsModel", QVariant::fromValue(m_contacts));

    refreshCalls(100);
}

void Pbap::updateCalls(QJsonArray vcards)
{
    QString name, number, datetime, type;

    m_calls.clear();

    for (auto vcard : vcards) {
        QJsonObject entry = vcard.toObject();
        name = entry.value("fn").toString();
        number = entry.value("telephone").toString();
        // For calls with an empty name, fetch the name from contacts
        if (name.isEmpty()) {
            bool found = false;
            for (auto contact_obj : m_contacts) {
                Contact *contact = qobject_cast<Contact *>(contact_obj);
                QList<PhoneNumber *> numbers = contact->numbers();
                for (auto phone_number : numbers) {
                    if (number.endsWith(phone_number->number())) {
                        name = contact->name();
                        found = true;
                        break;
                    }
                }
                if (found == true)
                    break;
            }
            if (!found)
                name = number;
        }

        QStringList typelist = entry.value("type").toString().split(QRegularExpression("="));
        type = typelist.takeLast();

        datetime = entry.value("timestamp").toString();
        // Convert the PBAP date/time to ISO 8601 format
        datetime.insert(4, '-');
        datetime.insert(7, '-');
        datetime.insert(13, ':');
        datetime.insert(16, ':');

        m_calls.append(new RecentCall(name, number, datetime, type));
    }

    // Refresh model
    m_context->setContextProperty("RecentCallModel", QVariant::fromValue(m_calls));
}

void Pbap::sendSearchResults(QJsonArray results)
{
    QString name;

    if (results.empty())
        name = "Not Found";
    else
        name = results.at(0).toObject().value("name").toString();

    emit searchResults(name);
}

#if 0
void Pbap::onConnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* pmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        pmsg->createRequest("bluetooth-pbap", "subscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }
}

void Pbap::onDisconnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* pmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        pmsg->createRequest("bluetooth-pbap", "unsubscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }
}

void Pbap::onMessageReceived(std::shared_ptr<Message> msg)
{
    if (!msg)
        return;

    if (msg->isEvent()) {
        std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
        QString ename = emsg->eventName();
        QString eapi = emsg->eventApi();
        QVariantMap data = emsg->eventData().toVariantMap();

        if (eapi != "bluetooth-pbap")
            return;

        bool connected = data.find("connected").value().toBool();
        if (ename == "status") {
            emit statusChanged(connected);
            if (connected)
                refreshContacts(-1);
            }
    } else if (msg->isReply()) {
        std::shared_ptr<ResponseMessage> rmsg = std::static_pointer_cast<ResponseMessage>(msg);
        QString api = rmsg->requestApi();
        if (api != "bluetooth-pbap")
            return;

        QString verb = rmsg->requestVerb();
        QJsonObject data = rmsg->replyData();
        if (verb == "contacts" || verb == "import") {
            updateContacts(data.value("vcards").toArray());
        } else if (verb == "history") {
            updateCalls(data.value("vcards").toArray());
        } else if (verb == "search") {
            sendSearchResults(data.value("results").toArray());
        }
    }
}
#endif
