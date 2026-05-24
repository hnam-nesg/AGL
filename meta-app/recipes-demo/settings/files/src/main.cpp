#include <QDebug>
#include <QCommandLineParser>
#include <QUrlQuery>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <bluetooth.h>
#include <network.h>

int main(int argc, char *argv[])
{
    QString graphic_role = QString("settings"); // defined in layers.json in window manager

    QGuiApplication app(argc, argv);

    app.setApplicationName(graphic_role);
    app.setApplicationVersion(QStringLiteral("0.1.1"));
    app.setOrganizationDomain(QStringLiteral("automotivelinux.org"));
    app.setOrganizationName(QStringLiteral("AutomotiveGradeLinux"));
    
    app.setDesktopFileName(graphic_role);


    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();

    QFile version("/proc/version");
    if (version.open(QFile::ReadOnly)) {
        QStringList data = QString::fromLocal8Bit(version.readAll()).split(QLatin1Char(' '));
        context->setContextProperty("kernel", data.at(2));
        version.close();
    } else {
        qWarning() << version.errorString();
    }

    QFile aglversion("/etc/os-release");
    if (aglversion.open(QFile::ReadOnly)) {
        QStringList data = QString::fromLocal8Bit(aglversion.readAll()).split(QLatin1Char('\n'));
        QStringList data2 = data.at(2).split(QLatin1Char('"'));
        context->setContextProperty("ucb", data2.at(1));
        aglversion.close();
    } else {
        qWarning() << aglversion.errorString();
    }

    context->setContextProperty("bluetooth", new Bluetooth(true, context));

    Network *network = new Network(true, context);
    network->power(true, QString("bluetooth"));
    context->setContextProperty("network", network);    

    engine.load(QUrl(QStringLiteral("qrc:/Settings.qml")));

    return app.exec();
}
