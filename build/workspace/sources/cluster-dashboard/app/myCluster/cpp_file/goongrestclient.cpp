#include "header_file/goongrestclient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <cmath>
#include <limits>

constexpr auto kBaseUrl = "https://rsapi.goong.io";

GoongRestClient::GoongRestClient(QObject *parent)
    : QObject(parent)
{
}

bool GoongRestClient::available() const
{
    return !m_apiKey.trimmed().isEmpty();
}

void GoongRestClient::setApiKey(const QString &apiKey)
{
    const QString trimmed = apiKey.trimmed();
    if (m_apiKey == trimmed)
        return;

    m_apiKey = trimmed;
    emit apiKeyChanged();
}

int GoongRestClient::autocomplete(const QString &input,
                                  double lat,
                                  double lon,
                                  int limit)
{
    const int requestId = nextRequestId();
    QUrl url = restUrl(QStringLiteral("/v2/place/autocomplete"));
    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("input"), input);
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit > 0 ? limit : 8));
    query.addQueryItem(QStringLiteral("language"), QStringLiteral("vi"));
    query.addQueryItem(QStringLiteral("region"), QStringLiteral("vn"));
    query.addQueryItem(QStringLiteral("components"), QStringLiteral("country:vn"));
    if (std::isfinite(lat) && std::isfinite(lon))
        query.addQueryItem(QStringLiteral("location"), coordString(lat, lon));
    url.setQuery(query);
    getJson(QStringLiteral("autocomplete"), url, requestId);
    return requestId;
}

int GoongRestClient::forwardGeocode(double lat,
                                    double lon,
                                    const QString &address)
{
    const int requestId = nextRequestId();
    QUrl url = restUrl(QStringLiteral("/v2/geocode"));
    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("address"), address);
    if (std::isfinite(lat) && std::isfinite(lon))
        query.addQueryItem(QStringLiteral("location"), coordString(lat, lon));
    url.setQuery(query);
    getJson(QStringLiteral("forwardGeocode"), url, requestId);
    return requestId;
}

int GoongRestClient::reverseGeocode(double lat, double lon)
{
    const int requestId = nextRequestId();
    QUrl url = restUrl(QStringLiteral("/v2/geocode"));
    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("latlng"), coordString(lat, lon));
    url.setQuery(query);
    getJson(QStringLiteral("reverseGeocode"), url, requestId);
    return requestId;
}

int GoongRestClient::placeDetail(const QString &placeId)
{
    const int requestId = nextRequestId();
    QUrl url = restUrl(QStringLiteral("/v2/place/detail"));
    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("place_id"), placeId);
    query.addQueryItem(QStringLiteral("language"), QStringLiteral("vi"));
    query.addQueryItem(QStringLiteral("region"), QStringLiteral("vn"));
    url.setQuery(query);
    getJson(QStringLiteral("placeDetail"), url, requestId);
    return requestId;
}

int GoongRestClient::direction(double originLat,
                               double originLon,
                               double destinationLat,
                               double destinationLon)
{
    return direction(originLat, originLon, destinationLat, destinationLon, QStringLiteral("car"));
}

int GoongRestClient::direction(double originLat,
                               double originLon,
                               double destinationLat,
                               double destinationLon,
                               const QString &vehicle)
{
    const int requestId = nextRequestId();
    QUrl url = restUrl(QStringLiteral("/v2/direction"));
    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("origin"), coordString(originLat, originLon));
    query.addQueryItem(QStringLiteral("destination"), coordString(destinationLat, destinationLon));
    query.addQueryItem(QStringLiteral("vehicle"), vehicle.isEmpty() ? QStringLiteral("car") : vehicle);
    query.addQueryItem(QStringLiteral("language"), QStringLiteral("vi"));
    query.addQueryItem(QStringLiteral("alternatives"), QStringLiteral("false"));
    url.setQuery(query);
    getJson(QStringLiteral("direction"), url, requestId);
    return requestId;
}

int GoongRestClient::distanceMatrix(const QVariantList &origins,
                                    const QVariantList &destinations)
{
    return distanceMatrix(origins, destinations, QStringLiteral("car"));
}

int GoongRestClient::distanceMatrix(const QVariantList &origins,
                                    const QVariantList &destinations,
                                    const QString &vehicle)
{
    const int requestId = nextRequestId();
    QUrl url = restUrl(QStringLiteral("/v2/distancematrix"));
    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("origins"), coordListString(origins));
    query.addQueryItem(QStringLiteral("destinations"), coordListString(destinations));
    query.addQueryItem(QStringLiteral("vehicle"), vehicle.isEmpty() ? QStringLiteral("car") : vehicle);
    url.setQuery(query);
    getJson(QStringLiteral("distanceMatrix"), url, requestId);
    return requestId;
}

int GoongRestClient::geolocation(const QVariantList &cells,
                                 const QVariantList &wifiAccessPoints)
{
    const int requestId = nextRequestId();
    QUrl url = restUrl(QStringLiteral("/v2/geolocation"));

    QVariantMap body;
    body.insert(QStringLiteral("cellTowers"), cells);
    body.insert(QStringLiteral("wifiAccessPoints"), wifiAccessPoints);
    postJson(QStringLiteral("geolocation"), url, body, requestId);
    return requestId;
}

QString GoongRestClient::staticMapUrl(double lat,
                                      double lon,
                                      int zoom,
                                      int width,
                                      int height)
{
    Q_UNUSED(lat)
    Q_UNUSED(lon)
    Q_UNUSED(zoom)
    Q_UNUSED(width)
    Q_UNUSED(height)
    return {};
}

QString GoongRestClient::staticMapRouteUrl(double originLat,
                                           double originLon,
                                           double destinationLat,
                                           double destinationLon)
{
    return staticMapRouteUrl(originLat,
                             originLon,
                             destinationLat,
                             destinationLon,
                             640,
                             360,
                             QStringLiteral("car"));
}

QString GoongRestClient::staticMapRouteUrl(double originLat,
                                           double originLon,
                                           double destinationLat,
                                           double destinationLon,
                                           int width,
                                           int height,
                                           const QString &vehicle)
{
    QUrl url = restUrl(QStringLiteral("/staticmap/route"));
    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("origin"), coordString(originLat, originLon));
    query.addQueryItem(QStringLiteral("destination"), coordString(destinationLat, destinationLon));
    query.addQueryItem(QStringLiteral("vehicle"), vehicle.isEmpty() ? QStringLiteral("car") : vehicle);
    query.addQueryItem(QStringLiteral("size"),
                       QStringLiteral("%1x%2").arg(width).arg(height));
    url.setQuery(query);
    return url.toString(QUrl::FullyEncoded);
}

int GoongRestClient::trip(const QVariantList &points,
                          const QString &vehicle)
{
    const int requestId = nextRequestId();
    QUrl url = restUrl(QStringLiteral("/v2/trip"));
    QUrlQuery query(url);
    const QStringList coords = pointStrings(points);
    if (!coords.isEmpty())
        query.addQueryItem(QStringLiteral("origin"), coords.first());
    if (coords.size() > 1)
        query.addQueryItem(QStringLiteral("destination"), coords.last());
    if (coords.size() > 2)
        query.addQueryItem(QStringLiteral("waypoints"), coords.mid(1, coords.size() - 2).join(QLatin1Char(';')));
    query.addQueryItem(QStringLiteral("vehicle"), vehicle.isEmpty() ? QStringLiteral("car") : vehicle);
    url.setQuery(query);
    getJson(QStringLiteral("trip"), url, requestId);
    return requestId;
}

int GoongRestClient::trip(const QVariantList &points)
{
    return trip(points, QStringLiteral("car"));
}

int GoongRestClient::nextRequestId()
{
    return ++m_nextRequestId;
}

void GoongRestClient::getJson(const QString &api,
                              const QUrl &url,
                              int requestId)
{
    if (!available()) {
        emit requestFailed(requestId, api, QStringLiteral("Thiếu GOONG_REST_API_KEY"));
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AGLNavigation/1.0"));
    request.setRawHeader("Accept", "application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_nam.get(request);
    reply->setProperty("requestId", requestId);
    reply->setProperty("api", api);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

void GoongRestClient::postJson(const QString &api,
                               const QUrl &url,
                               const QVariantMap &body,
                               int requestId)
{
    if (!available()) {
        emit requestFailed(requestId, api, QStringLiteral("Thiếu GOONG_REST_API_KEY"));
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AGLNavigation/1.0"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Accept", "application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    const QByteArray payload = QJsonDocument::fromVariant(body).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = m_nam.post(request, payload);
    reply->setProperty("requestId", requestId);
    reply->setProperty("api", api);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

void GoongRestClient::handleReply(QNetworkReply *reply)
{
    reply->deleteLater();

    const int requestId = reply->property("requestId").toInt();
    const QString api = reply->property("api").toString();
    const QByteArray data = reply->readAll();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError) {
        const QString message = status > 0
                ? QStringLiteral("Goong %1 lỗi HTTP %2").arg(api).arg(status)
                : QStringLiteral("Goong %1 lỗi mạng").arg(api);
        emit requestFailed(requestId, api, message);
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit requestFailed(requestId, api, QStringLiteral("Goong %1 trả JSON không hợp lệ").arg(api));
        return;
    }

    emit replyReady(requestId, api, doc.object().toVariantMap());
}

QUrl GoongRestClient::restUrl(const QString &path) const
{
    QUrl url(QString::fromLatin1(kBaseUrl) + path);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("api_key"), m_apiKey);
    url.setQuery(query);
    return url;
}

QString GoongRestClient::coordString(double lat, double lon) const
{
    return QStringLiteral("%1,%2")
            .arg(QString::number(lat, 'f', 7),
                 QString::number(lon, 'f', 7));
}

QString GoongRestClient::coordListString(const QVariantList &points) const
{
    return pointStrings(points).join(QLatin1Char('|'));
}

QStringList GoongRestClient::pointStrings(const QVariantList &points) const
{
    QStringList out;
    for (const QVariant &point : points) {
        const QVariantMap map = point.toMap();
        bool latOk = false;
        bool lonOk = false;
        const double lat = map.value(QStringLiteral("lat")).toDouble(&latOk);
        const double lon = map.contains(QStringLiteral("lon"))
                ? map.value(QStringLiteral("lon")).toDouble(&lonOk)
                : map.value(QStringLiteral("lng")).toDouble(&lonOk);
        if (latOk && lonOk && std::isfinite(lat) && std::isfinite(lon))
            out.push_back(coordString(lat, lon));
    }
    return out;
}
