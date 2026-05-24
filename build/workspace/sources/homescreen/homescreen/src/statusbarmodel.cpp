/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2017, 2018 TOYOTA MOTOR CORPORATION
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

#include "statusbarmodel.h"
#include "statusbarserver.h"
#include <network.h>
#include <wifiadapter.h>

class StatusBarModel::Private
{
public:
    Private(StatusBarModel *parent);

private:
    StatusBarModel *q;
public:
    StatusBarServer server;
    QString iconList[StatusBarServer::SupportedCount];
    Network *network;
    WifiAdapter *wifi_a;
};

StatusBarModel::Private::Private(StatusBarModel *parent)
    : q(parent)
{
    connect(&server, &StatusBarServer::statusIconChanged, [&](int placeholderIndex, const QString &icon) {
        if (placeholderIndex < 0 || StatusBarServer::SupportedCount <= placeholderIndex) return;
        if (iconList[placeholderIndex] == icon) return;
        iconList[placeholderIndex] = icon;
        emit q->dataChanged(q->index(placeholderIndex), q->index(placeholderIndex));
    });
    for (int i = 0; i < StatusBarServer::SupportedCount; i++) {
        iconList[i] = server.getStatusIcon(i);
    }
}

StatusBarModel::StatusBarModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
}

StatusBarModel::~StatusBarModel()
{
    delete d;
}

void StatusBarModel::init(QQmlContext *context)
{
    d->network = new Network(false, context);
    context->setContextProperty("network", d->network);
    d->wifi_a = static_cast<WifiAdapter*>(d->network->findAdapter("wifi"));
    Q_CHECK_PTR(d->wifi_a);

    QObject::connect(d->wifi_a, &WifiAdapter::wifiConnectedChanged,
		     this, &StatusBarModel::onWifiConnectedChanged);
    QObject::connect(d->wifi_a, &WifiAdapter::wifiEnabledChanged,
		     this, &StatusBarModel::onWifiEnabledChanged);
    QObject::connect(d->wifi_a, &WifiAdapter::wifiStrengthChanged,
		     this, &StatusBarModel::onWifiStrengthChanged);

    setWifiStatus(d->wifi_a->wifiConnected(), d->wifi_a->wifiEnabled(), d->wifi_a->wifiStrength());
}

void StatusBarModel::setWifiStatus(bool connected, bool enabled, int strength)
{
    if (enabled && connected)
        if (strength < 30)
            d->server.setStatusIcon(0, QStringLiteral("qrc:/images/Status/HMI_Status_Wifi_1Bar-01.png"));
        else if (strength < 50)
            d->server.setStatusIcon(0, QStringLiteral("qrc:/images/Status/HMI_Status_Wifi_2Bars-01.png"));
        else if (strength < 70)
            d->server.setStatusIcon(0, QStringLiteral("qrc:/images/Status/HMI_Status_Wifi_3Bars-01.png"));
        else
            d->server.setStatusIcon(0, QStringLiteral("qrc:/images/Status/HMI_Status_Wifi_Full-01.png"));
    else
        d->server.setStatusIcon(0, QStringLiteral("qrc:/images/Status/HMI_Status_Wifi_NoBars-01.png"));
}

void StatusBarModel::onWifiConnectedChanged(bool connected)
{
    setWifiStatus(connected, d->wifi_a->wifiEnabled(), d->wifi_a->wifiStrength());
}

void StatusBarModel::onWifiEnabledChanged(bool enabled)
{
    setWifiStatus(d->wifi_a->wifiConnected(), enabled, d->wifi_a->wifiStrength());
}

void StatusBarModel::onWifiStrengthChanged(int strength)
{
    qInfo() << "Strength changed: " << strength;
    setWifiStatus(d->wifi_a->wifiConnected(), d->wifi_a->wifiEnabled(), strength);
}

int StatusBarModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // Delete bluetooth because use agl-service-bluetooth.
    return StatusBarServer::SupportedCount - 1;
}

QVariant StatusBarModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;
    if (!index.isValid())
        return ret;

    switch (role) {
    case Qt::DisplayRole:
        if (index.row() == 0){
            ret = d->iconList[StatusBarServer::StatusWifi];
        }else if (index.row() == 1){
            ret = d->iconList[StatusBarServer::StatusCellular];
        }
        break;
    default:
        break;
    }

    return ret;
}

QHash<int, QByteArray> StatusBarModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "icon";
    return roles;
}
