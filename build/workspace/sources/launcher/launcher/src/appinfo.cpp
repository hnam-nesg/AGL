/*
 * Copyright (C) 2016, 2017 Mentor Graphics Development (Deutschland) GmbH
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (c) 2018 TOYOTA MOTOR CORPORATION
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

#include "appinfo.h"

#include <QtCore/QJsonObject>

class AppInfo::Private : public QSharedData
{
public:
    Private();
    Private(const Private &other);

    QString id;
    QString name;
    QString iconPath;
};

AppInfo::Private::Private()
{
}

AppInfo::Private::Private(const Private &other)
    : QSharedData(other)
    , id(other.id)
    , name(other.name)
    , iconPath(other.iconPath)
{
}

AppInfo::AppInfo()
    : d(new Private)
{
}

AppInfo::AppInfo(const QString &icon, const QString &name, const QString &id)
    : d(new Private)
{
    d->iconPath = icon;
    d->name = name;
    d->id = id;
}

AppInfo::AppInfo(const AppInfo &other)
    : d(other.d)
{
}

AppInfo::~AppInfo()
{
}

AppInfo &AppInfo::operator =(const AppInfo &other)
{
    d = other.d;
    return *this;
}

QString AppInfo::id() const
{
    return d->id;
}

QString AppInfo::name() const
{
    return d->name;
}

QString AppInfo::iconPath() const
{
    return d->iconPath;
}

void AppInfo::read(const QJsonObject &json)
{
    d->id = json["id"].toString();
    d->name = json["name"].toString();
    d->iconPath = json["iconPath"].toString();
}
