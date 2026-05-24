/*
 * Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef STATUSBARSERVER_H
#define STATUSBARSERVER_H

#include <QtCore/QObject>

class StatusBarServer : public QObject
{
    Q_OBJECT
public:
    enum {
        StatusWifi = 0,
        StatusBluetooth = 1,
        StatusCellular = 2,
        SupportedCount = 3,
    };
    explicit StatusBarServer(QObject *parent = NULL);
    ~StatusBarServer();

    Q_INVOKABLE QList<int> getAvailablePlaceholders() const;
    Q_INVOKABLE QString getStatusIcon(int placeholderIndex) const;
    Q_INVOKABLE QString getStatusText(int placeholderIndex) const;

public slots:
    void setStatusIcon(int placeholderIndex, const QString &icon);
    void setStatusText(int placeholderIndex, const QString &text);

signals:
    void statusIconChanged(int placeholderIndex, const QString &icon);
    void statusTextChanged(int placeholderIndex, const QString &text);

private:
    class Private;
    Private *d;
};

#endif // STATUSBARSERVER_H
