#include "PhoneNavigationBridge.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>
#include <QUrlQuery>
#include <QDebug>
#include <QSharedPointer>
#include <QtMath>

namespace {

constexpr const char *kDefaultResponseHeader =
        "HTTP/1.1 %1 %2\r\n"
        "Content-Type: %3\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Connection: close\r\n"
        "Content-Length: %4\r\n"
        "\r\n";

QString statusText(int code)
{
    switch (code) {
    case 200: return QStringLiteral("OK");
    case 400: return QStringLiteral("Bad Request");
    case 404: return QStringLiteral("Not Found");
    case 500: return QStringLiteral("Internal Server Error");
    default: return QStringLiteral("OK");
    }
}

QString envFirst(const char *a, const char *b)
{
    const QString va = qEnvironmentVariable(a).trimmed();
    if (!va.isEmpty())
        return va;

    return qEnvironmentVariable(b).trimmed();
}

bool validLatLon(double lat, double lon)
{
    return lat >= -90.0 && lat <= 90.0
        && lon >= -180.0 && lon <= 180.0;
}

} // namespace

PhoneNavigationBridge::PhoneNavigationBridge(QObject *parent)
    : QObject(parent),
      m_goongApiKey(envFirst("GOONG_REST_API_KEY", "GOONG_API_KEY"))
{
    connect(&m_server, &QTcpServer::newConnection,
            this, &PhoneNavigationBridge::onNewConnection);

    QTimer::singleShot(0, this, [this]() {
        start();
    });
}

bool PhoneNavigationBridge::isListening() const
{
    return m_server.isListening();
}

quint16 PhoneNavigationBridge::port() const
{
    return m_port;
}

void PhoneNavigationBridge::setPort(quint16 value)
{
    if (m_port == value)
        return;

    const bool wasListening = m_server.isListening();
    if (wasListening)
        m_server.close();

    m_port = value;
    emit portChanged();

    if (wasListening)
        start();
}

QString PhoneNavigationBridge::goongApiKey() const
{
    return m_goongApiKey;
}

void PhoneNavigationBridge::setGoongApiKey(const QString &value)
{
    const QString cleaned = value.trimmed();
    if (m_goongApiKey == cleaned)
        return;

    m_goongApiKey = cleaned;
    emit goongApiKeyChanged();

    qWarning() << "PHONE_NAV_GOONG_KEY_UPDATED"
               << (!m_goongApiKey.isEmpty());
}

QString PhoneNavigationBridge::lastError() const
{
    return m_lastError;
}

QString PhoneNavigationBridge::lastRawBody() const
{
    return m_lastRawBody;
}

bool PhoneNavigationBridge::start()
{
    if (m_server.isListening())
        return true;

    if (!m_server.listen(QHostAddress::AnyIPv4, m_port)) {
        m_lastError = QStringLiteral("PhoneNavigationBridge listen failed on port %1: %2")
                              .arg(m_port)
                              .arg(m_server.errorString());
        qWarning().noquote() << m_lastError;
        emit lastErrorChanged();
        emit listeningChanged();
        return false;
    }

    qWarning() << "PHONE_NAV_BRIDGE_LISTENING"
               << m_server.serverAddress().toString()
               << m_server.serverPort()
               << "goongKey" << (!m_goongApiKey.isEmpty());

    emit listeningChanged();
    return true;
}

void PhoneNavigationBridge::stop()
{
    if (!m_server.isListening())
        return;

    m_server.close();
    emit listeningChanged();
}

bool PhoneNavigationBridge::restart()
{
    stop();
    return start();
}

void PhoneNavigationBridge::onNewConnection()
{
    while (QTcpSocket *socket = m_server.nextPendingConnection()) {
        socket->setParent(this);
        handleSocket(socket);
    }
}

void PhoneNavigationBridge::handleSocket(QTcpSocket *socket)
{
    QSharedPointer<QByteArray> buffer = QSharedPointer<QByteArray>::create();
    buffer->reserve(4096);

    connect(socket, &QTcpSocket::readyRead, this, [this, socket, buffer]() {
        buffer->append(socket->readAll());

        const int headerEnd = buffer->indexOf("\r\n\r\n");
        if (headerEnd < 0)
            return;

        const QByteArray headers = buffer->left(headerEnd + 4);
        int contentLength = 0;

        const QList<QByteArray> lines = headers.split('\n');
        for (QByteArray line : lines) {
            line = line.trimmed();
            if (line.toLower().startsWith("content-length:")) {
                contentLength = line.mid(line.indexOf(':') + 1).trimmed().toInt();
                break;
            }
        }

        if (buffer->size() < headerEnd + 4 + contentLength)
            return;

        const QByteArray request = buffer->left(headerEnd + 4 + contentLength);
        buffer->clear();

        processHttpRequest(socket, request);
    });

    connect(socket, &QTcpSocket::disconnected,
            socket, &QTcpSocket::deleteLater);
}

void PhoneNavigationBridge::processHttpRequest(QTcpSocket *socket, const QByteArray &request)
{
    const int headerEnd = request.indexOf("\r\n\r\n");
    if (headerEnd < 0) {
        sendHttpText(socket, 400, QStringLiteral("bad request\n"));
        return;
    }

    const QByteArray headerBlock = request.left(headerEnd);
    const QByteArray body = request.mid(headerEnd + 4);
    const QList<QByteArray> lines = headerBlock.split('\n');

    if (lines.isEmpty()) {
        sendHttpText(socket, 400, QStringLiteral("empty request\n"));
        return;
    }

    const QList<QByteArray> requestLine = lines.first().trimmed().split(' ');
    if (requestLine.size() < 2) {
        sendHttpText(socket, 400, QStringLiteral("bad request line\n"));
        return;
    }

    const QString method = QString::fromLatin1(requestLine.at(0)).toUpper();
    const QString path = QString::fromUtf8(requestLine.at(1));

    if (method == QLatin1String("OPTIONS")) {
        sendHttpJson(socket, 200, QJsonObject{{QStringLiteral("ok"), true}});
        return;
    }

    if (method == QLatin1String("GET")) {
        const QString text = QStringLiteral(
                    "PhoneNavigationBridge OK\n"
                    "POST /navigation or /navigate with JSON body.\n"
                    "Example: {\"link\":\"https://maps.app.goo.gl/...\",\"source\":\"iphone-shortcut\"}\n");
        sendHttpText(socket, 200, text);
        return;
    }

    if (method != QLatin1String("POST")) {
        sendHttpText(socket, 404, QStringLiteral("unsupported method\n"));
        return;
    }

    m_lastRawBody = QString::fromUtf8(body);
    emit lastRawBodyChanged();
    emit requestReceived(path, m_lastRawBody);

    qWarning().noquote() << "PHONE_NAV_HTTP_POST" << path << m_lastRawBody;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(body, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        const QString msg = QStringLiteral("invalid JSON: %1").arg(err.errorString());
        finishParseFailed(msg, m_lastRawBody, QPointer<QTcpSocket>(socket));
        return;
    }

    resolveLinkThenParse(doc.object(), m_lastRawBody, socket);
}

void PhoneNavigationBridge::sendHttpJson(QTcpSocket *socket, int statusCode, const QJsonObject &body)
{
    if (!socket)
        return;

    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact) + '\n';
    const QString header = QString::fromLatin1(kDefaultResponseHeader)
            .arg(statusCode)
            .arg(statusText(statusCode))
            .arg(QStringLiteral("application/json; charset=utf-8"))
            .arg(payload.size());

    socket->write(header.toUtf8());
    socket->write(payload);
    socket->flush();
    socket->disconnectFromHost();
}

void PhoneNavigationBridge::sendHttpText(QTcpSocket *socket, int statusCode, const QString &text)
{
    if (!socket)
        return;

    const QByteArray payload = text.toUtf8();
    const QString header = QString::fromLatin1(kDefaultResponseHeader)
            .arg(statusCode)
            .arg(statusText(statusCode))
            .arg(QStringLiteral("text/plain; charset=utf-8"))
            .arg(payload.size());

    socket->write(header.toUtf8());
    socket->write(payload);
    socket->flush();
    socket->disconnectFromHost();
}

void PhoneNavigationBridge::resolveLinkThenParse(const QJsonObject &payload,
                                                 const QString &rawBody,
                                                 QTcpSocket *replySocket)
{
    Q_UNUSED(rawBody);

    QPointer<QTcpSocket> safeSocket(replySocket);

    const QString link = payload.value(QStringLiteral("link")).toString().trimmed();

    if (link.isEmpty()) {
        ParseResult result = parseJsonPayload(payload);
        resolvePendingGeocodesThenFinish(result, safeSocket);
        return;
    }

    const QUrl url(link);
    if (!url.isValid() || url.scheme().isEmpty()) {
        ParseResult result = parseJsonPayload(payload);

        if (result.hasDestination || result.needDestinationGeocode) {
            resolvePendingGeocodesThenFinish(result, safeSocket);
            return;
        }

        result.error = QStringLiteral("invalid link URL: %1").arg(link);
        finishParseFailed(result.error, m_lastRawBody, safeSocket, &result);
        return;
    }

    resolveUrlRecursive(url, 8, [this, payload, safeSocket](const QUrl &finalUrl) mutable {
        QJsonObject withResolved = payload;
        withResolved.insert(QStringLiteral("_resolvedLink"), finalUrl.toString());

        ParseResult result = parseJsonPayload(withResolved);
        resolvePendingGeocodesThenFinish(result, safeSocket);
    });
}

void PhoneNavigationBridge::resolveUrlRecursive(const QUrl &url,
                                                int redirectsLeft,
                                                const std::function<void(QUrl)> &done)
{
    if (redirectsLeft <= 0 || !url.isValid()) {
        done(url);
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 AGL PhoneNavigationBridge"));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);

    QNetworkReply *reply = m_network.get(request);

    QTimer *timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    timeout->setInterval(12000);

    connect(timeout, &QTimer::timeout, reply, [reply]() {
        reply->abort();
    });

    timeout->start();

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, url, redirectsLeft, done]() {
        const QVariant redirectTarget =
                reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        const QUrl redirected = redirectTarget.toUrl();
        const QUrl finalUrl = reply->url();

        const QString networkError =
                reply->error() == QNetworkReply::NoError
                    ? QString()
                    : reply->errorString();

        reply->deleteLater();

        if (redirected.isValid()) {
            const QUrl next = finalUrl.resolved(redirected);
            resolveUrlRecursive(next, redirectsLeft - 1, done);
            return;
        }

        if (!networkError.isEmpty()) {
            qWarning() << "PHONE_NAV_RESOLVE_LINK_WARNING"
                       << url.toString()
                       << networkError;
        }

        done(finalUrl);
    });
}

PhoneNavigationBridge::ParseResult PhoneNavigationBridge::parseBody(const QByteArray &body)
{
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(body, &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        ParseResult result;
        result.error = QStringLiteral("invalid JSON: %1").arg(err.errorString());
        return result;
    }

    return parseJsonPayload(doc.object());
}

PhoneNavigationBridge::ParseResult PhoneNavigationBridge::parseJsonPayload(const QJsonObject &payload)
{
    ParseResult result;
    result.ok = true;

    result.source = payload.value(QStringLiteral("source")).toString();
    result.name = payload.value(QStringLiteral("name")).toString();
    result.link = payload.value(QStringLiteral("link")).toString().trimmed();
    result.resolvedLink = payload.value(QStringLiteral("_resolvedLink")).toString().trimmed();

    /*
     * Optional direct start fields.
     * Useful if a future shortcut/app sends:
     * {
     *   "lat": ...,
     *   "lon": ...,
     *   "startLat": ...,
     *   "startLon": ...
     * }
     */
    if (payload.contains(QStringLiteral("startLat"))
            && payload.contains(QStringLiteral("startLon"))) {
        const double slat = payload.value(QStringLiteral("startLat")).toDouble(qQNaN());
        const double slon = payload.value(QStringLiteral("startLon")).toDouble(qQNaN());

        if (validLatLon(slat, slon)) {
            result.startLat = slat;
            result.startLon = slon;
            result.hasStart = true;
            result.debug.insert(QStringLiteral("startMethod"),
                                QStringLiteral("direct_start_lat_lon"));
        }
    }

    /*
     * Direct destination fields.
     */
    if (payload.contains(QStringLiteral("lat"))
            && payload.contains(QStringLiteral("lon"))) {
        const double dlat = payload.value(QStringLiteral("lat")).toDouble(qQNaN());
        const double dlon = payload.value(QStringLiteral("lon")).toDouble(qQNaN());

        if (validLatLon(dlat, dlon)) {
            result.lat = dlat;
            result.lon = dlon;
            result.hasDestination = true;
            result.method = QStringLiteral("direct_lat_lon");
        } else {
            result.error = QStringLiteral("direct lat/lon out of range");
            result.method = QStringLiteral("direct_lat_lon_invalid");
        }

        return result;
    }

    /*
     * If the final URL itself contains @lat,lon or !3dlat!4dlon,
     * use that as destination.
     */
    double lat = 0.0;
    double lon = 0.0;

    if (extractLatLonFromText(result.resolvedLink, &lat, &lon)
            || extractLatLonFromText(result.link, &lat, &lon)) {
        result.lat = lat;
        result.lon = lon;
        result.hasDestination = true;
        result.method = QStringLiteral("lat_lon_in_url");
        return result;
    }

    /*
     * Google Maps direction URL usually contains:
     *   saddr=start
     *   daddr=destination
     *
     * Example:
     *   saddr=10.8072010,106.7048970
     *   daddr=Trường Đại học Công nghệ Kỹ thuật...
     */
    const QUrl url(result.resolvedLink.isEmpty() ? result.link : result.resolvedLink);

    result.saddr = queryParam(url, QStringLiteral("saddr"));
    result.daddr = queryParam(url, QStringLiteral("daddr"));

    /*
     * Fallbacks for other Google Maps URL styles.
     */
    if (result.daddr.isEmpty())
        result.daddr = queryParam(url, QStringLiteral("destination"));

    if (result.daddr.isEmpty())
        result.daddr = queryParam(url, QStringLiteral("query"));

    if (result.saddr.isEmpty())
        result.saddr = queryParam(url, QStringLiteral("origin"));

    /*
     * Resolve start without network first:
     *   - lat,lon => use immediately
     *   - "My Location" / "Vị trí của bạn" => skip and let QML use vehicle position
     *   - text address => mark for Goong geocode
     */
    if (!result.saddr.isEmpty()) {
        if (isCurrentLocationText(result.saddr)) {
            result.debug.insert(QStringLiteral("startMethod"),
                                QStringLiteral("current_location_alias_skip"));
        } else if (extractPlainLatLon(result.saddr, &lat, &lon)) {
            result.startLat = lat;
            result.startLon = lon;
            result.hasStart = true;
            result.debug.insert(QStringLiteral("startMethod"),
                                QStringLiteral("saddr_lat_lon"));
        } else {
            result.needStartGeocode = true;
            result.startGeocodeText = result.saddr;
            result.debug.insert(QStringLiteral("startMethod"),
                                QStringLiteral("need_geocode_saddr"));
        }
    }

    /*
     * Resolve destination without network first:
     *   - lat,lon => use immediately
     *   - text address => mark for Goong geocode
     */
    if (!result.daddr.isEmpty()) {
        if (isCurrentLocationText(result.daddr)) {
            result.error = QStringLiteral("destination is current-location alias; no fixed destination");
            result.method = QStringLiteral("daddr_current_location_alias_invalid");
            return result;
        }

        if (extractPlainLatLon(result.daddr, &lat, &lon)) {
            result.lat = lat;
            result.lon = lon;
            result.hasDestination = true;
            result.method = QStringLiteral("daddr_lat_lon");
            return result;
        }

        result.needDestinationGeocode = true;
        result.destinationGeocodeText = result.daddr;
        result.method = QStringLiteral("need_geocode_daddr");
        return result;
    }

    result.error = QStringLiteral("no destination coordinate or daddr in link");
    result.method = QStringLiteral("no_destination_found");
    return result;
}

void PhoneNavigationBridge::resolvePendingGeocodesThenFinish(ParseResult result,
                                                             QPointer<QTcpSocket> replySocket)
{
    /*
     * Start geocode is optional. If it fails, we still continue with destination.
     */
    if (result.needStartGeocode && !result.hasStart) {
        const QString startText = result.startGeocodeText;

        geocodeAddressAsync(startText, QStringLiteral("saddr"),
                            [this, result, replySocket](ResolvedPoint point) mutable {
            result.needStartGeocode = false;

            if (point.ok) {
                result.startLat = point.lat;
                result.startLon = point.lon;
                result.hasStart = true;

                result.debug.insert(QStringLiteral("startMethod"), point.method);
                result.debug.insert(QStringLiteral("startInput"), point.input);
                result.debug.insert(QStringLiteral("startName"), point.name);
                result.debug.insert(QStringLiteral("startDebug"), point.debug);

                qWarning() << "PHONE_NAV_START_READY"
                           << result.startLat << result.startLon
                           << "name" << point.name
                           << "method" << point.method;
            } else {
                result.debug.insert(QStringLiteral("startGeocodeError"), point.error);
                result.debug.insert(QStringLiteral("startInput"), point.input);

                qWarning() << "PHONE_NAV_START_GEOCODE_FAILED"
                           << point.input
                           << point.error;
            }

            resolveDestinationGeocodeThenFinish(result, replySocket);
        });

        return;
    }

    resolveDestinationGeocodeThenFinish(result, replySocket);
}

void PhoneNavigationBridge::resolveDestinationGeocodeThenFinish(ParseResult result,
                                                                QPointer<QTcpSocket> replySocket)
{
    if (result.hasDestination) {
        finishDestination(result, replySocket);
        return;
    }

    if (!result.needDestinationGeocode) {
        const QString msg = result.error.isEmpty()
                ? QStringLiteral("could not extract destination from link")
                : result.error;

        finishParseFailed(msg, m_lastRawBody, replySocket, &result);
        return;
    }

    const QString destText = result.destinationGeocodeText;

    geocodeAddressAsync(destText, QStringLiteral("daddr"),
                        [this, result, replySocket, destText](ResolvedPoint point) mutable {
        result.needDestinationGeocode = false;

        if (!point.ok) {
            result.error = point.error.isEmpty()
                    ? QStringLiteral("destination geocode failed")
                    : point.error;

            result.method = QStringLiteral("goong_geocode_daddr_failed");
            result.debug.insert(QStringLiteral("destinationInput"), point.input);
            result.debug.insert(QStringLiteral("destinationGeocodeError"), point.error);

            finishParseFailed(result.error, m_lastRawBody, replySocket, &result);
            return;
        }

        result.lat = point.lat;
        result.lon = point.lon;
        result.hasDestination = true;
        result.method = point.method;

        if (result.name.isEmpty())
            result.name = point.name.isEmpty() ? destText : point.name;

        result.debug.insert(QStringLiteral("destinationInput"), point.input);
        result.debug.insert(QStringLiteral("destinationName"), point.name);
        result.debug.insert(QStringLiteral("destinationDebug"), point.debug);

        finishDestination(result, replySocket);
    });
}

void PhoneNavigationBridge::geocodeAddressAsync(const QString &address,
                                                const QString &role,
                                                const std::function<void(ResolvedPoint)> &done)
{
    ResolvedPoint early;
    early.input = address;
    early.method = role + QStringLiteral("_goong_geocode");

    const QString cleanAddress = address.trimmed();
    if (cleanAddress.isEmpty()) {
        early.error = role + QStringLiteral(" geocode address is empty");
        done(early);
        return;
    }

    if (m_goongApiKey.isEmpty()) {
        early.error = QStringLiteral("GOONG_REST_API_KEY is empty; cannot geocode %1")
                              .arg(role);
        done(early);
        return;
    }

    QUrl url(QStringLiteral("https://rsapi.goong.io/geocode"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("address"), cleanAddress);
    query.addQueryItem(QStringLiteral("api_key"), m_goongApiKey);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 AGL PhoneNavigationBridge"));

    QNetworkReply *reply = m_network.get(request);

    QTimer *timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    timeout->setInterval(12000);

    connect(timeout, &QTimer::timeout, reply, [reply]() {
        reply->abort();
    });

    timeout->start();

    connect(reply, &QNetworkReply::finished, this,
            [reply, done, cleanAddress, role]() {
        ResolvedPoint point;
        point.input = cleanAddress;
        point.method = role + QStringLiteral("_goong_geocode");

        const QByteArray body = reply->readAll();

        const QString networkError =
                reply->error() == QNetworkReply::NoError
                    ? QString()
                    : reply->errorString();

        reply->deleteLater();

        if (!networkError.isEmpty()) {
            point.error = QStringLiteral("Goong geocode network error for %1: %2")
                                  .arg(role, networkError);
            done(point);
            return;
        }

        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(body, &err);

        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            point.error = QStringLiteral("Goong geocode JSON parse error for %1: %2")
                                  .arg(role, err.errorString());
            point.debug.insert(QStringLiteral("rawBody"),
                               QString::fromUtf8(body.left(300)));
            done(point);
            return;
        }

        const QJsonObject root = doc.object();
        const QJsonArray results = root.value(QStringLiteral("results")).toArray();

        point.debug.insert(QStringLiteral("status"),
                           root.value(QStringLiteral("status")).toString());

        if (results.isEmpty()) {
            point.error = QStringLiteral("Goong geocode returned no results for %1")
                                  .arg(role);
            done(point);
            return;
        }

        const QJsonObject first = results.first().toObject();
        const QJsonObject geometry = first.value(QStringLiteral("geometry")).toObject();
        const QJsonObject location = geometry.value(QStringLiteral("location")).toObject();

        const double lat = location.value(QStringLiteral("lat")).toDouble(qQNaN());
        const double lon = location.value(QStringLiteral("lng")).toDouble(qQNaN());

        if (!validLatLon(lat, lon)) {
            point.error = QStringLiteral("Goong geocode returned invalid location for %1")
                                  .arg(role);
            point.debug.insert(QStringLiteral("rawFirst"),
                               first);
            done(point);
            return;
        }

        point.ok = true;
        point.lat = lat;
        point.lon = lon;
        point.name = first.value(QStringLiteral("formatted_address"))
                          .toString(cleanAddress);

        point.debug.insert(QStringLiteral("formattedAddress"),
                           first.value(QStringLiteral("formatted_address")).toString());
        point.debug.insert(QStringLiteral("placeId"),
                           first.value(QStringLiteral("place_id")).toString());

        done(point);
    });
}

void PhoneNavigationBridge::finishDestination(const ParseResult &result,
                                              QPointer<QTcpSocket> replySocket)
{
    qWarning() << "PHONE_NAV_DESTINATION_READY"
               << result.lat << result.lon
               << "name" << result.name
               << "method" << result.method
               << "hasStart" << result.hasStart
               << result.startLat << result.startLon;

    emit destinationReceived(result.lat,
                             result.lon,
                             result.name,
                             result.startLat,
                             result.startLon,
                             result.hasStart);

    if (replySocket)
        sendHttpJson(replySocket, 200, resultToJson(result));
}

void PhoneNavigationBridge::finishParseFailed(const QString &message,
                                              const QString &rawBody,
                                              QPointer<QTcpSocket> replySocket,
                                              const ParseResult *result)
{
    m_lastError = message;

    qWarning().noquote() << "PHONE_NAV_PARSE_FAILED" << message;

    emit lastErrorChanged();
    emit parseFailed(message, rawBody);

    if (!replySocket)
        return;

    QJsonObject response;
    if (result)
        response = resultToJson(*result);

    response.insert(QStringLiteral("ok"), false);
    response.insert(QStringLiteral("error"), message);

    sendHttpJson(replySocket, 200, response);
}

bool PhoneNavigationBridge::extractLatLonFromText(const QString &text,
                                                  double *lat,
                                                  double *lon)
{
    if (text.trimmed().isEmpty())
        return false;

    const QString decoded = QUrl::fromPercentEncoding(text.toUtf8());

    const QList<QRegularExpression> patterns = {
        QRegularExpression(QStringLiteral("@(-?\\d+(?:\\.\\d+)?),(-?\\d+(?:\\.\\d+)?)")),
        QRegularExpression(QStringLiteral("[?&]q=(-?\\d+(?:\\.\\d+)?),(-?\\d+(?:\\.\\d+)?)")),
        QRegularExpression(QStringLiteral("[?&]query=(-?\\d+(?:\\.\\d+)?),(-?\\d+(?:\\.\\d+)?)")),
        QRegularExpression(QStringLiteral("!3d(-?\\d+(?:\\.\\d+)?)!4d(-?\\d+(?:\\.\\d+)?)")),
        QRegularExpression(QStringLiteral("ll=(-?\\d+(?:\\.\\d+)?),(-?\\d+(?:\\.\\d+)?)"))
    };

    for (const QRegularExpression &re : patterns) {
        const QRegularExpressionMatch match = re.match(decoded);
        if (!match.hasMatch())
            continue;

        const double parsedLat = match.captured(1).toDouble();
        const double parsedLon = match.captured(2).toDouble();

        if (validLatLon(parsedLat, parsedLon)) {
            if (lat)
                *lat = parsedLat;
            if (lon)
                *lon = parsedLon;
            return true;
        }
    }

    return false;
}

bool PhoneNavigationBridge::extractPlainLatLon(const QString &text,
                                               double *lat,
                                               double *lon)
{
    const QString decoded = QUrl::fromPercentEncoding(text.toUtf8()).trimmed();

    const QRegularExpression re(
                QStringLiteral("^\\s*(-?\\d+(?:\\.\\d+)?)\\s*,\\s*(-?\\d+(?:\\.\\d+)?)\\s*$"));

    const QRegularExpressionMatch match = re.match(decoded);
    if (!match.hasMatch())
        return false;

    const double parsedLat = match.captured(1).toDouble();
    const double parsedLon = match.captured(2).toDouble();

    if (!validLatLon(parsedLat, parsedLon))
        return false;

    if (lat)
        *lat = parsedLat;
    if (lon)
        *lon = parsedLon;

    return true;
}

QString PhoneNavigationBridge::queryParam(const QUrl &url, const QString &key)
{
    if (!url.isValid())
        return QString();

    QUrlQuery query(url);
    return query.queryItemValue(key, QUrl::FullyDecoded).trimmed();
}

QString PhoneNavigationBridge::sanitizeHeaderValue(const QString &value)
{
    QString out = value;
    out.remove('\r');
    out.remove('\n');
    return out;
}

bool PhoneNavigationBridge::isCurrentLocationText(const QString &text)
{
    QString s = QUrl::fromPercentEncoding(text.toUtf8()).trimmed().toLower();

    /*
     * Normalize common punctuation/spacing.
     */
    s.replace(QStringLiteral("+"), QStringLiteral(" "));
    s = s.simplified();

    return s == QStringLiteral("my location")
        || s == QStringLiteral("current location")
        || s == QStringLiteral("your location")
        || s == QStringLiteral("vị trí của bạn")
        || s == QStringLiteral("vi tri cua ban")
        || s == QStringLiteral("vị trí hiện tại")
        || s == QStringLiteral("vi tri hien tai")
        || s == QStringLiteral("vị trí của tôi")
        || s == QStringLiteral("vi tri cua toi");
}

QJsonObject PhoneNavigationBridge::resultToJson(const ParseResult &result)
{
    QJsonObject obj;

    obj.insert(QStringLiteral("ok"), result.hasDestination);

    obj.insert(QStringLiteral("link"), result.link);
    obj.insert(QStringLiteral("resolvedLink"), result.resolvedLink);
    obj.insert(QStringLiteral("source"), result.source);

    obj.insert(QStringLiteral("saddr"), result.saddr);
    obj.insert(QStringLiteral("daddr"), result.daddr);

    obj.insert(QStringLiteral("lat"),
               result.hasDestination ? QJsonValue(result.lat) : QJsonValue());

    obj.insert(QStringLiteral("lon"),
               result.hasDestination ? QJsonValue(result.lon) : QJsonValue());

    obj.insert(QStringLiteral("name"), result.name);
    obj.insert(QStringLiteral("method"), result.method);
    obj.insert(QStringLiteral("error"), result.error);

    obj.insert(QStringLiteral("hasStart"), result.hasStart);

    obj.insert(QStringLiteral("startLat"),
               result.hasStart ? QJsonValue(result.startLat) : QJsonValue());

    obj.insert(QStringLiteral("startLon"),
               result.hasStart ? QJsonValue(result.startLon) : QJsonValue());

    obj.insert(QStringLiteral("needStartGeocode"), result.needStartGeocode);
    obj.insert(QStringLiteral("needDestinationGeocode"), result.needDestinationGeocode);

    obj.insert(QStringLiteral("startGeocodeText"), result.startGeocodeText);
    obj.insert(QStringLiteral("destinationGeocodeText"), result.destinationGeocodeText);

    obj.insert(QStringLiteral("debug"), result.debug);

    return obj;
}