// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (c) 2018,2019 TOYOTA MOTOR CORPORATION
 * Copyright (C) 2022 Konsulko Group
 */

#ifndef APPLICATIONMODEL_H
#define APPLICATIONMODEL_H

#include <QAbstractListModel>

class ApplicationModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ApplicationModel(QObject *parent = nullptr);
    ~ApplicationModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QString appid(int index) const;
    Q_INVOKABLE QString id(int index) const;
    Q_INVOKABLE QString name(int index) const;
    Q_INVOKABLE void move(int from, int to);
    Q_INVOKABLE void initAppList(QList<QMap<QString, QString>> &apps);
    Q_INVOKABLE void updateApplist(QStringList info);

private:
    class Private;
    Private *d;
};

#endif // APPLICATIONMODEL_H
