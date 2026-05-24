// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2016, 2017 Mentor Graphics Development (Deutschland) GmbH
 * Copyright (c) 2018,2019 TOYOTA MOTOR CORPORATION
 * Copyright (C) 2022 Konsulko Group
 */

#include "applicationmodel.h"
#include "appinfo.h"

#include <QtCore/QFile>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>

#include "hmi-debug.h"

class ApplicationModel::Private
{
public:
    Private();

    void addApp(QString icon, QString name, QString id);
    void removeApp(QString id);

    QList<AppInfo> data;
};

namespace {
    QString get_icon_name(QJsonObject const &i)
    {
        QString icon = i["icon"].toString();
        fprintf(stderr, "Looking for icon %s\n", icon.toLocal8Bit().data());
        if ( !QFile::exists(icon) )
            icon = "blank";
        return icon;
    }
}

ApplicationModel::Private::Private()
{
}

void ApplicationModel::Private::addApp(QString icon, QString name, QString id)
{
    HMI_DEBUG("addApp","name: %s icon: %s id: %s.", name.toStdString().c_str(), icon.toStdString().c_str(), id.toStdString().c_str());
    for(int i = 0; i < this->data.size(); ++i) {
        if(this->data[i].id() == id)
            return;
    }

    QString _icon;
    if ( QFile::exists(icon) )
    {
        _icon = QString("file:%1").arg(icon);
        fprintf(stderr, "using icon '%s'\n", _icon.toLocal8Bit().data());
    }
    else
    {
        _icon = "blank";
    }

    int pos = 0;
    for (pos = 0; pos < this->data.size(); ++pos) {
        if (QString::compare(this->data.at(pos).name(), name, Qt::CaseInsensitive) > 0)
            break;
    }
    this->data.insert(pos, AppInfo(_icon, name, id));
}

void ApplicationModel::Private::removeApp(QString id)
{
    HMI_DEBUG("removeApp","id: %s.",id.toStdString().c_str());
    for (int i = 0; i < this->data.size(); ++i) {
          if (this->data.at(i).id() == id) {
              this->data.removeAt(i);
              break;
          }
    }
}

ApplicationModel::ApplicationModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private())
{
}

ApplicationModel::~ApplicationModel()
{
    delete this->d;
}

int ApplicationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return this->d->data.count();
}

QVariant ApplicationModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;
    if (!index.isValid())
        return ret;

    switch (role) {
    case Qt::DecorationRole:
        ret = this->d->data[index.row()].iconPath();
        break;
    case Qt::DisplayRole:
        ret = this->d->data[index.row()].name();
        break;
    case Qt::UserRole:
        ret = this->d->data[index.row()].id();
        break;
    default:
        break;
    }

    return ret;
}

QHash<int, QByteArray> ApplicationModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DecorationRole] = "icon";
    roles[Qt::DisplayRole] = "name";
    roles[Qt::UserRole] = "id";
    return roles;
}

QString ApplicationModel::id(int i) const
{
    return data(index(i), Qt::UserRole).toString();
}

QString ApplicationModel::appid(int i) const
{
    QString id = data(index(i), Qt::UserRole).toString();
    return id.split("@")[0];
}

QString ApplicationModel::name(int i) const
{
    return data(index(i), Qt::DisplayRole).toString();
}

void ApplicationModel::move(int from, int to)
{
    QModelIndex parent;
    if (to < 0 || to > rowCount()) return;
    if (from < to) {
        if (!beginMoveRows(parent, from, from, parent, to + 1)) {
            HMI_NOTICE("launcher","from : %d, to : %d. false.", from, to);
            return;
        }
        d->data.move(from, to);
        endMoveRows();
    } else if (from > to) {
        if (!beginMoveRows(parent, from, from, parent, to)) {
            HMI_NOTICE("launcher","from : %d, to : %d. false.", from, to);
            return;
        }
        d->data.move(from, to);
        endMoveRows();
    } else {
        HMI_NOTICE("launcher","from : %d, to : %d. false.", from, to);
    }
}

void ApplicationModel::updateApplist(QStringList info)
{
    QString icon = info.at(0);
    QString name = info.at(1);
    QString id = info.at(2);

    beginResetModel();
    if(icon == "") { // uninstall
        d->removeApp(id);
    }
    else {
        // new app
        d->addApp(icon, name, id);
    }
    endResetModel();
}

void ApplicationModel::initAppList(QList<QMap<QString, QString>> &apps)
{
    HMI_DEBUG("launcher","init application list.");
    beginResetModel();
    qDebug() << "ApplicationModel::initAppList: got " << apps.size() << " apps";
    for (int i = 0; i < apps.size(); i++) {
        d->addApp(apps[i]["icon_path"], apps[i]["name"], apps[i]["id"]);
    }
    endResetModel();
}
