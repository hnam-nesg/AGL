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

#ifndef PBAP_H
#define PBAP_H

#include <QObject>
#include <QJsonArray>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlListProperty>

class PhoneNumber : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString number READ number NOTIFY numberChanged)
    Q_PROPERTY(int type READ type NOTIFY typeChanged)

    public:
        explicit PhoneNumber(QString number, QString type);
        virtual ~PhoneNumber();

        QString number() {return m_number;};
        int type() {return m_type;};

        enum PhoneNumberType {
            UNKNOWN,
            VOICE,
            CELL,
            HOME,
            WORK,
            FAX,
        };
        Q_ENUM(PhoneNumberType)

    signals:
        void numberChanged();
        void typeChanged();

    private:
        QString m_number;
        int m_type;
        int stringToEnum(QString);
};

class Contact : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString photo READ photo NOTIFY photoChanged)
    Q_PROPERTY(QQmlListProperty<PhoneNumber> numbers READ numbersList NOTIFY numbersListChanged)

    public:
        explicit Contact(QString name, QString photo, QList<PhoneNumber *>numbers);
        virtual ~Contact();

        bool operator<(Contact& c) {return ((this->m_name < c.m_name));};
        QString name() {return m_name;};
        QString photo() {return m_photo;};
        QList<PhoneNumber *>numbers() {return m_numbers;};
        QQmlListProperty<PhoneNumber>numbersList() {
            return QQmlListProperty<PhoneNumber>(this, 0, &Contact::countNumbers, &Contact::atNumbers);
        }
        static qsizetype countNumbers(QQmlListProperty<PhoneNumber> *property) {
            Contact *contact = qobject_cast<Contact *>(property->object);
            return contact->m_numbers.size();
        }
        static PhoneNumber *atNumbers(QQmlListProperty<PhoneNumber> *property, qsizetype index) {
            Contact *contact = qobject_cast<Contact *>(property->object);
            return contact->m_numbers[index];
        }

    signals:
        void nameChanged();
        void photoChanged();
        void numbersListChanged();

    private:
        QString m_name;
        QString m_photo;
        QList<PhoneNumber *>m_numbers;
};

class RecentCall : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString number READ number NOTIFY numberChanged)
    Q_PROPERTY(QString datetime READ datetime NOTIFY datetimeChanged)
    Q_PROPERTY(int type READ type NOTIFY typeChanged)

    public:
        explicit RecentCall(QString name, QString number, QString datetime, QString type);
        virtual ~RecentCall();

        QString name() {return m_name;};
        QString number() {return m_number;};
        QString datetime() {return m_datetime;};
        int type() {return m_type;};

        enum RecentCallType {
            UNKNOWN,
            MISSED,
            RECEIVED,
            DIALED,
        };
        Q_ENUM(RecentCallType)

    signals:
        void nameChanged();
        void numberChanged();
        void datetimeChanged();
        void typeChanged();

    private:
        QString m_name;
        QString m_number;
        QString m_datetime;
        int m_type;
        int stringToEnum(QString);
};

class Pbap : public QObject
{
    Q_OBJECT

    public:
        explicit Pbap(QQmlContext *context, QObject * parent = Q_NULLPTR);
        virtual ~Pbap();

        Q_INVOKABLE void importContacts(int max_entries);
        Q_INVOKABLE void refreshContacts(int max_entries);
        Q_INVOKABLE void refreshCalls(int max_entries);
        Q_INVOKABLE void search(QString number);

    signals:
        void searchResults(QString name);
        void statusChanged(bool connected);

    private:
        QQmlContext *m_context;
        QList<QObject *>m_contacts;
        QList<QObject *>m_calls;
        void updateContacts(QJsonArray);
        void updateCalls(QJsonArray);
        void sendSearchResults(QJsonArray);

        const QStringList events {
            "status",
        };
};

#endif // PBAP_H
