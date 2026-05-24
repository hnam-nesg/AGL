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

#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QJsonArray>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlListProperty>

class Map : public QObject
{
    Q_OBJECT

    public:
        explicit Map(QObject * parent = Q_NULLPTR);
        virtual ~Map();

        Q_INVOKABLE void compose(QString recipient, QString message);
        Q_INVOKABLE void message(QString handle);
        Q_INVOKABLE void listMessages(QString folder = "inbox");

    signals:
        void notificationEvent(QVariantMap message);
        void listMessagesResult(QString folder, QVariantMap listing);
        void messageResult(QString handle, QVariantMap message);

    private:
#if 0
        std::shared_ptr<MessageEngine> m_mloop;

        // slots
        void onConnected();
        void onDisconnected();
        void onMessageReceived(std::shared_ptr<Message>);
#endif
};

#endif // MAP_H
