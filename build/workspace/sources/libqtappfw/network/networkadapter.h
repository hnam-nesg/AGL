/*
 * Copyright (C) 2019,2022 Konsulko Group
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

#ifndef ADAPTER_H
#define ADAPTER_H

#include <QDebug>
#include <QObject>
#include <QtQml/QQmlContext>

class Network;
class WifiNetworkModel;
class WiredNetworkModel;

class AdapterIf
{
    public:
        virtual ~AdapterIf() {};

        virtual bool addService(QString const &id, QVariantMap const &properties) = 0;
        virtual void removeService(QString const &id) = 0;
        virtual void updateProperties(QString const &service, QVariantMap const &properties) = 0;

        virtual QString getType() = 0;
        virtual void updateStatus(QVariantMap const &properties) = 0;
};
Q_DECLARE_INTERFACE(AdapterIf, "AdapterIf")

#endif // ADAPTER_H
