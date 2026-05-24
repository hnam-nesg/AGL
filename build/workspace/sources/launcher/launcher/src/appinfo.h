// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2016, 2017 Mentor Graphics Development (Deutschland) GmbH
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (c) 2018 TOYOTA MOTOR CORPORATION
 */

#ifndef APPINFO_H
#define APPINFO_H

#include <QObject>
#include <QtCore/QSharedDataPointer>

class AppInfo
{
    Q_GADGET
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString iconPath READ iconPath)
public:
    AppInfo();
    AppInfo(const QString &icon, const QString &name, const QString &id);
    AppInfo(const AppInfo &other);
    virtual ~AppInfo();
    AppInfo &operator =(const AppInfo &other);
    void swap(AppInfo &other) { qSwap(d, other.d); }

    QString id() const;
    QString name() const;
    QString iconPath() const;

    void read(const QJsonObject &json);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_SHARED(AppInfo)
Q_DECLARE_METATYPE(AppInfo)
Q_DECLARE_METATYPE(QList<AppInfo>)

#endif // APPINFO_H
