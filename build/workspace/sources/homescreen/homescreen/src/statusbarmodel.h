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

#ifndef STATUSBARMODEL_H
#define STATUSBARMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtQml/QQmlContext>

class StatusBarModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit StatusBarModel(QObject *parent = NULL);
    ~StatusBarModel();

    void init(QQmlContext *context);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // slots
    void onWifiConnectedChanged(bool connected);
    void onWifiEnabledChanged(bool enabled);
    void onWifiStrengthChanged(int strength);

private:
    class Private;
    Private *d;
    void setWifiStatus(bool connected, bool enabled, int strength);
};

#endif // STATUSBARMODEL_H
