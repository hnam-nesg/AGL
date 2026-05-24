#pragma once

#include <QObject>
#include <QString>

class HvacUiDbus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.agl.hvac")

public:
    explicit HvacUiDbus(QObject *parent = nullptr);

    bool registerService();

public slots:
    Q_SCRIPTABLE QString Ping() const;
    Q_SCRIPTABLE bool ShowPage(const QString &page);
    Q_SCRIPTABLE bool ShowSeatPage();
    Q_SCRIPTABLE bool ShowClimatePage();

signals:
    Q_SCRIPTABLE void pageRequested(int pageIndex);
};
