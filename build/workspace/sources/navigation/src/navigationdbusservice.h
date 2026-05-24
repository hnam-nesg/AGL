#pragma once

#include <QObject>
#include <QString>

class NavigationDBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.agl.navigation")

public:
    explicit NavigationDBusService(QObject *parent = nullptr);

    bool registerService();

public slots:
    Q_SCRIPTABLE bool NavigateTo(const QString &destination);
    Q_SCRIPTABLE bool SearchDestination(const QString &destination);
    Q_SCRIPTABLE bool navigateTo(const QString &destination);
    Q_SCRIPTABLE bool searchDestination(const QString &destination);
    Q_SCRIPTABLE QString Ping() const;
    Q_INVOKABLE void publishRouteState(bool active,
                                       double currentLatitude,
                                       double currentLongitude,
                                       double startLatitude,
                                       double startLongitude,
                                       double destinationLatitude,
                                       double destinationLongitude,
                                       double heading,
                                       const QString &instruction,
                                       const QString &nextInstruction,
                                       const QString &distanceText,
                                       const QString &timeText,
                                       const QString &routePathJson,
                                       const QString &destinationName);

signals:
    Q_SCRIPTABLE void destinationRequested(const QString &destination);
    Q_SCRIPTABLE void routeStateChanged(bool active,
                                        double currentLatitude,
                                        double currentLongitude,
                                        double startLatitude,
                                        double startLongitude,
                                        double destinationLatitude,
                                        double destinationLongitude,
                                        double heading,
                                        const QString &instruction,
                                        const QString &nextInstruction,
                                        const QString &distanceText,
                                        const QString &timeText,
                                        const QString &routePathJson,
                                        const QString &destinationName);

private:
    bool requestDestination(const QString &destination);
};
