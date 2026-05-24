/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2016, 2017 Mentor Graphics Development (Deutschland) GmbH
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

#include "statusbarserver.h"

class StatusBarServer::Private
{
public:
    Private(StatusBarServer *parent);
    QString texts[SupportedCount];
    QString icons[SupportedCount];
};

StatusBarServer::Private::Private(StatusBarServer *parent)
{
    icons[0] = QStringLiteral("qrc:/images/Status/HMI_Status_Wifi_NoBars-01.png");
    icons[1] = QStringLiteral("qrc:/images/Status/HMI_Status_Bluetooth_Inactive-01.png");
    icons[2] = QStringLiteral("qrc:/images/Status/HMI_Status_Signal_NoBars-01.png");
}

StatusBarServer::StatusBarServer(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

StatusBarServer::~StatusBarServer()
{
    delete d;
}

QList<int> StatusBarServer::getAvailablePlaceholders() const
{
    QList<int> ret;
    for (int i = 0; i < SupportedCount; ++i) {
        ret.append(i);
    }
    return ret;
}

QString StatusBarServer::getStatusIcon(int placeholderIndex) const
{
    QString ret;
    if (-1 < placeholderIndex && placeholderIndex < SupportedCount)
        ret = d->icons[placeholderIndex];
    return ret;
}

void StatusBarServer::setStatusIcon(int placeholderIndex, const QString &icon)
{
    if (-1 < placeholderIndex && placeholderIndex < SupportedCount) {
        if (d->icons[placeholderIndex] == icon) return;
        d->icons[placeholderIndex] = icon;
        emit statusIconChanged(placeholderIndex, icon);
    }
}

QString StatusBarServer::getStatusText(int placeholderIndex) const
{
    QString ret;
    if (-1 < placeholderIndex && placeholderIndex < SupportedCount) {
        ret = d->texts[placeholderIndex];
    }
    return ret;
}

void StatusBarServer::setStatusText(int placeholderIndex, const QString &text)
{
    if (-1 < placeholderIndex && placeholderIndex < SupportedCount) {
        if (d->texts[placeholderIndex] == text) return;
        d->texts[placeholderIndex] = text;
        emit statusTextChanged(placeholderIndex, text);
    }
}
