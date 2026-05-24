#pragma once

#include <QObject>
#include <QHostAddress>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>

#include <functional>

class PhoneNavigationBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool listening READ isListening NOTIFY listeningChanged)
    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString goongApiKey READ goongApiKey WRITE setGoongApiKey NOTIFY goongApiKeyChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString lastRawBody READ lastRawBody NOTIFY lastRawBodyChanged)

public:
    explicit PhoneNavigationBridge(QObject *parent = nullptr);
    ~PhoneNavigationBridge() override = default;

    bool isListening() const;
    quint16 port() const;
    void setPort(quint16 value);

    QString goongApiKey() const;
    void setGoongApiKey(const QString &value);

    QString lastError() const;
    QString lastRawBody() const;

    Q_INVOKABLE bool start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE bool restart();

signals:
    void listeningChanged();
    void portChanged();
    void goongApiKeyChanged();
    void lastErrorChanged();
    void lastRawBodyChanged();

    void requestReceived(QString path, QString body);

    /*
     * lat/lon        : destination coordinate
     * name           : destination display name
     * startLat/startLon/hasStart:
     *   - hasStart=true  => Google Maps link supplied/derived a valid start coordinate
     *   - hasStart=false => QML should keep using current/default vehicle start coordinate
     */
    void destinationReceived(double lat, double lon, QString name,
                             double startLat, double startLon, bool hasStart);

    void parseFailed(QString message, QString rawBody);

private slots:
    void onNewConnection();

private:
    struct ResolvedPoint {
        bool ok = false;
        double lat = 0.0;
        double lon = 0.0;
        QString input;
        QString name;
        QString method;
        QString error;
        QJsonObject debug;
    };

    struct ParseResult {
        bool ok = false;
        QString error;

        QString link;
        QString resolvedLink;
        QString source;
        QString name;

        QString saddr;
        QString daddr;

        double lat = 0.0;
        double lon = 0.0;
        bool hasDestination = false;

        double startLat = 0.0;
        double startLon = 0.0;
        bool hasStart = false;

        bool needStartGeocode = false;
        bool needDestinationGeocode = false;
        QString startGeocodeText;
        QString destinationGeocodeText;

        QString method;
        QJsonObject debug;
    };

    void handleSocket(QTcpSocket *socket);
    void processHttpRequest(QTcpSocket *socket, const QByteArray &request);
    void sendHttpJson(QTcpSocket *socket, int statusCode, const QJsonObject &body);
    void sendHttpText(QTcpSocket *socket, int statusCode, const QString &text);

    ParseResult parseBody(const QByteArray &body);
    ParseResult parseJsonPayload(const QJsonObject &payload);

    void resolveLinkThenParse(const QJsonObject &payload,
                              const QString &rawBody,
                              QTcpSocket *replySocket = nullptr);

    void resolveUrlRecursive(const QUrl &url,
                             int redirectsLeft,
                             const std::function<void(QUrl)> &done);

    void resolvePendingGeocodesThenFinish(ParseResult result,
                                          QPointer<QTcpSocket> replySocket);

    void resolveDestinationGeocodeThenFinish(ParseResult result,
                                             QPointer<QTcpSocket> replySocket);

    void geocodeAddressAsync(const QString &address,
                             const QString &role,
                             const std::function<void(ResolvedPoint)> &done);

    void finishDestination(const ParseResult &result,
                           QPointer<QTcpSocket> replySocket = nullptr);

    void finishParseFailed(const QString &message,
                           const QString &rawBody,
                           QPointer<QTcpSocket> replySocket = nullptr,
                           const ParseResult *result = nullptr);

    static bool extractLatLonFromText(const QString &text, double *lat, double *lon);
    static bool extractPlainLatLon(const QString &text, double *lat, double *lon);
    static QString queryParam(const QUrl &url, const QString &key);
    static QString sanitizeHeaderValue(const QString &value);
    static bool isCurrentLocationText(const QString &text);
    static QJsonObject resultToJson(const ParseResult &result);

    QTcpServer m_server;
    QNetworkAccessManager m_network;
    quint16 m_port = 8765;
    QString m_goongApiKey;
    QString m_lastError;
    QString m_lastRawBody;
};