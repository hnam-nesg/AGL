#include "searchcontroller.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QSet>
#include <QVariantMap>
#include <QVector>
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <limits>

namespace {

constexpr int kMinSearchChars = 2;
constexpr int kMaxCacheEntries = 80;
constexpr qint64 kRateLimitBackoffMs = 15000;
constexpr double kSampleLat = 10.80690;
constexpr double kSampleLon = 106.70290;

double distanceMeters(double lat1, double lon1, double lat2, double lon2);

bool isValidCoordinate(double lat, double lon)
{
    return std::isfinite(lat) && std::isfinite(lon)
            && lat >= -90.0 && lat <= 90.0
            && lon >= -180.0 && lon <= 180.0;
}

QString normalizeText(const QString &value)
{
    const QString decomposed = value.normalized(QString::NormalizationForm_D);
    QString out;
    out.reserve(decomposed.size());

    for (const QChar &raw : decomposed) {
        const QChar ch = raw.toLower();
        const QChar::Category category = ch.category();
        if (category == QChar::Mark_NonSpacing ||
            category == QChar::Mark_SpacingCombining ||
            category == QChar::Mark_Enclosing) {
            continue;
        }

        if (ch == QChar(0x0111)) {
            out += QLatin1Char('d');
        } else if (ch.isLetterOrNumber()) {
            out += ch;
        } else {
            out += QLatin1Char(' ');
        }
    }

    return out.simplified();
}

QString simplifySearchIntent(QString normalized)
{
    const QStringList fillers = {
        QStringLiteral("tim kiem"),
        QStringLiteral("tim"),
        QStringLiteral("gan day"),
        QStringLiteral("gan toi"),
        QStringLiteral("gan nhat"),
        QStringLiteral("quanh day"),
        QStringLiteral("quanh toi"),
        QStringLiteral("xung quanh"),
        QStringLiteral("o day"),
        QStringLiteral("o dau"),
        QStringLiteral("cho toi"),
        QStringLiteral("di den")
    };

    for (const QString &filler : fillers)
        normalized.replace(filler, QLatin1String(" "));
    return normalized.simplified();
}

int editDistanceLimited(const QString &a, const QString &b, int limit)
{
    if (std::abs(static_cast<int>(a.size() - b.size())) > limit)
        return limit + 1;

    QVector<int> previous(b.size() + 1);
    QVector<int> current(b.size() + 1);
    for (int j = 0; j <= b.size(); ++j)
        previous[j] = j;

    for (int i = 1; i <= a.size(); ++i) {
        current[0] = i;
        int rowBest = current[0];
        for (int j = 1; j <= b.size(); ++j) {
            const int cost = a.at(i - 1) == b.at(j - 1) ? 0 : 1;
            current[j] = std::min({ previous[j] + 1,
                                    current[j - 1] + 1,
                                    previous[j - 1] + cost });
            rowBest = std::min(rowBest, current[j]);
        }
        if (rowBest > limit)
            return limit + 1;
        previous.swap(current);
    }

    return previous[b.size()];
}

QString typeLabel(const QString &placeClass, const QString &type)
{
    const QString cls = placeClass.toLower();
    const QString t = type.toLower();

    if (t.contains(QStringLiteral("restaurant"))) return QStringLiteral("Nhà hàng");
    if (t.contains(QStringLiteral("cafe"))) return QStringLiteral("Quán cà phê");
    if (t.contains(QStringLiteral("fuel"))) return QStringLiteral("Trạm xăng");
    if (t.contains(QStringLiteral("hospital"))) return QStringLiteral("Bệnh viện");
    if (t.contains(QStringLiteral("school")) || t.contains(QStringLiteral("university"))) return QStringLiteral("Trường học");
    if (t.contains(QStringLiteral("hotel"))) return QStringLiteral("Khách sạn");
    if (t.contains(QStringLiteral("bank")) || t.contains(QStringLiteral("atm"))) return QStringLiteral("Ngân hàng");
    if (t.contains(QStringLiteral("parking"))) return QStringLiteral("Bãi đỗ xe");
    if (cls == QStringLiteral("shop")) return QStringLiteral("Cửa hàng");
    if (cls == QStringLiteral("tourism")) return QStringLiteral("Du lịch");
    if (cls == QStringLiteral("highway")) return QStringLiteral("Đường");
    if (cls == QStringLiteral("place")) return QStringLiteral("Khu vực");
    if (cls == QStringLiteral("amenity")) return QStringLiteral("Địa điểm");
    return QStringLiteral("Địa điểm");
}

QString smartQuery(const QString &query)
{
    const QString n = normalizeText(query);
    const QString intent = simplifySearchIntent(n);
    const QString trimmed = query.trimmed();

    if (intent == QStringLiteral("cf") ||
        intent == QStringLiteral("cafe") ||
        intent == QStringLiteral("coffee") ||
        intent == QStringLiteral("ca phe") ||
        intent == QStringLiteral("quan ca phe")) {
        return QStringLiteral("cafe");
    }

    if (intent == QStringLiteral("an") ||
        intent == QStringLiteral("an gi") ||
        intent == QStringLiteral("quan an") ||
        intent == QStringLiteral("nha hang") ||
        intent == QStringLiteral("fast food")) {
        return QStringLiteral("restaurant");
    }

    if (intent == QStringLiteral("do xang") ||
        intent == QStringLiteral("cay xang") ||
        intent == QStringLiteral("tram xang") ||
        intent == QStringLiteral("xang")) {
        return QStringLiteral("fuel");
    }

    if (intent == QStringLiteral("gui xe") ||
        intent == QStringLiteral("do xe") ||
        intent == QStringLiteral("bai xe") ||
        intent == QStringLiteral("bai do xe")) {
        return QStringLiteral("parking");
    }

    if (intent == QStringLiteral("rut tien") ||
        intent == QStringLiteral("atm") ||
        intent == QStringLiteral("ngan hang")) {
        return QStringLiteral("atm");
    }

    if (intent == QStringLiteral("nha thuoc") ||
        intent == QStringLiteral("hieu thuoc") ||
        intent == QStringLiteral("thuoc tay")) {
        return QStringLiteral("pharmacy");
    }

    if (intent == QStringLiteral("benh vien") ||
        intent == QStringLiteral("phong kham") ||
        intent == QStringLiteral("kham benh")) {
        return QStringLiteral("hospital");
    }

    if (intent == QStringLiteral("bhx") ||
        intent == QStringLiteral("bach hoa xanh")) {
        return QStringLiteral("Bách Hóa Xanh");
    }

    if (intent == QStringLiteral("sieu thi") ||
        intent == QStringLiteral("tap hoa") ||
        intent == QStringLiteral("cua hang tien loi")) {
        return QStringLiteral("supermarket");
    }

    if (intent == QStringLiteral("cho")) {
        return QStringLiteral("marketplace");
    }

    if (intent == QStringLiteral("khach san") ||
        intent == QStringLiteral("hotel")) {
        return QStringLiteral("hotel");
    }

    if (intent == QStringLiteral("landmark81") ||
        intent == QStringLiteral("landmark 81") ||
        intent == QStringLiteral("landmark")) {
        return QStringLiteral("Landmark 81");
    }

    return trimmed;
}

QString firstNonEmpty(std::initializer_list<QString> values)
{
    for (const QString &value : values) {
        const QString trimmed = value.trimmed();
        if (!trimmed.isEmpty())
            return trimmed;
    }

    return {};
}

QString leadingDescriptionPart(const QString &description)
{
    const QStringList parts = description.split(QLatin1Char(','), Qt::SkipEmptyParts);
    return parts.isEmpty() ? description.trimmed() : parts.first().trimmed();
}

QString fullDisplayName(const QString &primary, const QString &secondary)
{
    const QString p = primary.trimmed();
    const QString s = secondary.trimmed();
    if (p.isEmpty())
        return s;
    if (s.isEmpty())
        return p;

    const QString np = normalizeText(p);
    const QString ns = normalizeText(s);
    if (ns == np || ns.startsWith(np + QLatin1Char(' ')) ||
        ns.contains(QStringLiteral(" ") + np + QStringLiteral(" "))) {
        return s;
    }

    return p + QStringLiteral(", ") + s;
}

QString localSuffix(double lat, double lon)
{
    if (isValidCoordinate(lat, lon) && distanceMeters(lat, lon, kSampleLat, kSampleLon) < 45000.0)
        return QStringLiteral("Bình Thạnh, Hồ Chí Minh");
    return QStringLiteral("Việt Nam");
}

bool queryHasExplicitLocation(const QString &query)
{
    const QString n = normalizeText(query);
    return n.contains(QStringLiteral("ho chi minh")) ||
            n.contains(QStringLiteral("hcm")) ||
            n.contains(QStringLiteral("sai gon")) ||
            n.contains(QStringLiteral("viet nam")) ||
            n.contains(QStringLiteral("ha noi")) ||
            n.contains(QStringLiteral("da nang")) ||
            n.contains(QStringLiteral("da lat")) ||
            n.contains(QStringLiteral("binh duong")) ||
            n.contains(QStringLiteral("dong nai")) ||
            n.contains(QStringLiteral("vung tau"));
}

QString searchQueryForAttempt(const QString &query, double lat, double lon, int attempt)
{
    const QString base = smartQuery(query);
    const QString normalizedBase = normalizeText(base);
    const QString suffix = localSuffix(lat, lon);
    const QString normalizedSuffix = normalizeText(suffix);

    if (attempt == 0)
        return base;

    if (attempt == 1 && !normalizedBase.contains(normalizedSuffix))
        return base + QStringLiteral(", ") + suffix;

    if (attempt == 2 && !normalizeText(query).contains(QStringLiteral("ho chi minh")))
        return query.trimmed() + QStringLiteral(", Hồ Chí Minh");

    if (attempt == 3 && !normalizeText(query).contains(QStringLiteral("viet nam")))
        return query.trimmed() + QStringLiteral(", Việt Nam");

    const QString normalized = normalizeText(query);
    if (attempt == 4 && !normalized.isEmpty())
        return normalized + QStringLiteral(", Vietnam");

    return {};
}

QStringList osmTagsForQuery(const QString &query)
{
    const QString n = normalizeText(query);
    QStringList tags;

    auto add = [&tags](const QString &tag) {
        if (!tags.contains(tag))
            tags.push_back(tag);
    };

    if (n.contains(QStringLiteral("cafe")) || n.contains(QStringLiteral("coffee")) || n.contains(QStringLiteral("ca phe")) || n == QStringLiteral("cf")) {
        add(QStringLiteral("amenity:cafe"));
    }
    if (n.contains(QStringLiteral("fast food"))) {
        add(QStringLiteral("amenity:fast_food"));
    } else if (n.contains(QStringLiteral("restaurant")) || n.contains(QStringLiteral("nha hang")) || n.contains(QStringLiteral("quan an")) || n == QStringLiteral("an")) {
        add(QStringLiteral("amenity:restaurant"));
    }
    if (n.contains(QStringLiteral("fuel")) || n.contains(QStringLiteral("gas station")) || n.contains(QStringLiteral("xang"))) {
        add(QStringLiteral("amenity:fuel"));
    }
    if (n.contains(QStringLiteral("parking")) || n.contains(QStringLiteral("do xe")) || n.contains(QStringLiteral("gui xe")) || n.contains(QStringLiteral("bai xe"))) {
        add(QStringLiteral("amenity:parking"));
    }
    if (n.contains(QStringLiteral("atm")) || n.contains(QStringLiteral("rut tien"))) {
        add(QStringLiteral("amenity:atm"));
    } else if (n.contains(QStringLiteral("ngan hang"))) {
        add(QStringLiteral("amenity:bank"));
    }
    if (n.contains(QStringLiteral("hospital")) || n.contains(QStringLiteral("benh vien"))) {
        add(QStringLiteral("amenity:hospital"));
    } else if (n.contains(QStringLiteral("phong kham")) || n.contains(QStringLiteral("kham benh"))) {
        add(QStringLiteral("amenity:clinic"));
    }
    if (n.contains(QStringLiteral("pharmacy")) || n.contains(QStringLiteral("nha thuoc")) || n.contains(QStringLiteral("thuoc tay"))) {
        add(QStringLiteral("amenity:pharmacy"));
    }
    if (n.contains(QStringLiteral("supermarket")) || n.contains(QStringLiteral("sieu thi")) || n.contains(QStringLiteral("bach hoa xanh")) || n.contains(QStringLiteral("bhx"))) {
        add(QStringLiteral("shop:supermarket"));
    } else if (n.contains(QStringLiteral("tap hoa")) || n.contains(QStringLiteral("cua hang tien loi"))) {
        add(QStringLiteral("shop:convenience"));
    }
    if (n == QStringLiteral("cho") || n.contains(QStringLiteral("cho ")) || n.contains(QStringLiteral("marketplace"))) {
        add(QStringLiteral("amenity:marketplace"));
    }
    if (n.contains(QStringLiteral("hotel")) || n.contains(QStringLiteral("khach san"))) {
        add(QStringLiteral("tourism:hotel"));
    }

    return tags;
}

int categoryScore(const QString &query, const QString &placeClass, const QString &type)
{
    const QString n = normalizeText(query);
    const QString cls = placeClass.toLower();
    const QString t = type.toLower();

    if ((n.contains(QStringLiteral("cafe")) || n.contains(QStringLiteral("coffee")) || n.contains(QStringLiteral("ca phe")) || n == QStringLiteral("cf")) &&
        t.contains(QStringLiteral("cafe"))) return 260;
    if ((n.contains(QStringLiteral("restaurant")) || n.contains(QStringLiteral("nha hang")) || n.contains(QStringLiteral("quan an")) || n == QStringLiteral("an")) &&
        (t.contains(QStringLiteral("restaurant")) || t.contains(QStringLiteral("fast_food")))) return 260;
    if ((n.contains(QStringLiteral("fuel")) || n.contains(QStringLiteral("xang"))) && t.contains(QStringLiteral("fuel"))) return 260;
    if ((n.contains(QStringLiteral("parking")) || n.contains(QStringLiteral("do xe")) || n.contains(QStringLiteral("gui xe"))) && t.contains(QStringLiteral("parking"))) return 240;
    if ((n.contains(QStringLiteral("atm")) || n.contains(QStringLiteral("rut tien"))) && t.contains(QStringLiteral("atm"))) return 240;
    if (n.contains(QStringLiteral("ngan hang")) && t.contains(QStringLiteral("bank"))) return 220;
    if ((n.contains(QStringLiteral("hospital")) || n.contains(QStringLiteral("benh vien")) || n.contains(QStringLiteral("phong kham"))) && (t.contains(QStringLiteral("hospital")) || t.contains(QStringLiteral("clinic")))) return 240;
    if ((n.contains(QStringLiteral("pharmacy")) || n.contains(QStringLiteral("nha thuoc")) || n.contains(QStringLiteral("thuoc tay"))) && t.contains(QStringLiteral("pharmacy"))) return 240;
    if ((n.contains(QStringLiteral("supermarket")) || n.contains(QStringLiteral("sieu thi")) || n.contains(QStringLiteral("tap hoa")) || n.contains(QStringLiteral("bach hoa xanh")) || n.contains(QStringLiteral("bhx"))) &&
        (cls == QStringLiteral("shop") || t.contains(QStringLiteral("supermarket")) || t.contains(QStringLiteral("convenience")))) return 260;
    if ((n == QStringLiteral("cho") || n.contains(QStringLiteral("cho ")) || n.contains(QStringLiteral("marketplace"))) && t.contains(QStringLiteral("marketplace"))) return 240;
    if ((n.contains(QStringLiteral("hotel")) || n.contains(QStringLiteral("khach san"))) && t.contains(QStringLiteral("hotel"))) return 240;

    return 0;
}

int classPriority(const QString &placeClass, const QString &type)
{
    const QString cls = placeClass.toLower();
    const QString t = type.toLower();

    if (t.contains(QStringLiteral("restaurant")) || t.contains(QStringLiteral("cafe")) ||
        t.contains(QStringLiteral("fuel")) || t.contains(QStringLiteral("hospital")) ||
        t.contains(QStringLiteral("hotel"))) {
        return 140;
    }
    if (cls == QStringLiteral("amenity") || cls == QStringLiteral("shop") || cls == QStringLiteral("tourism"))
        return 120;
    if (cls == QStringLiteral("highway"))
        return 80;
    if (cls == QStringLiteral("place"))
        return 60;
    return 40;
}

int distanceScore(double meters)
{
    if (!std::isfinite(meters))
        return 0;
    if (meters < 300) return 180;
    if (meters < 1000) return 150;
    if (meters < 3000) return 120;
    if (meters < 10000) return 90;
    if (meters < 30000) return 60;
    if (meters < 100000) return 30;
    return 0;
}

int proximityScore(double meters)
{
    if (!std::isfinite(meters))
        return 0;

    return qRound(650000.0 / (1.0 + std::max(0.0, meters) / 1000.0));
}

double distanceMeters(double lat1, double lon1, double lat2, double lon2)
{
    constexpr double kEarthRadiusMeters = 6371000.0;
    constexpr double kPi = 3.14159265358979323846;
    constexpr double kDegToRad = kPi / 180.0;

    const double p1 = lat1 * kDegToRad;
    const double p2 = lat2 * kDegToRad;
    const double dp = (lat2 - lat1) * kDegToRad;
    const double dl = (lon2 - lon1) * kDegToRad;
    const double a = std::sin(dp / 2.0) * std::sin(dp / 2.0)
            + std::cos(p1) * std::cos(p2)
            * std::sin(dl / 2.0) * std::sin(dl / 2.0);
    const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return kEarthRadiusMeters * c;
}

QString distanceText(double meters)
{
    if (!std::isfinite(meters))
        return {};
    if (meters < 100)
        return QStringLiteral("< 100 m");
    if (meters < 1000)
        return QStringLiteral("%1 m").arg(qRound(meters / 10.0) * 10);

    const double km = meters / 1000.0;
    if (km < 100.0)
        return QStringLiteral("%1 km").arg(QString::number(std::round(km * 10.0) / 10.0, 'f', 1));
    return QStringLiteral("%1 km").arg(qRound(km));
}

QString resultKey(const QString &primary, double lat, double lon)
{
    return QStringLiteral("%1|%2|%3")
            .arg(normalizeText(primary))
            .arg(QString::number(lat, 'f', 5))
            .arg(QString::number(lon, 'f', 5));
}

QString searchCacheKey(const QString &query, double lat, double lon)
{
    if (!isValidCoordinate(lat, lon))
        return normalizeText(query) + QStringLiteral("|vn");

    const qint64 latCell = static_cast<qint64>(std::llround(lat * 100.0));
    const qint64 lonCell = static_cast<qint64>(std::llround(lon * 100.0));
    return QStringLiteral("%1|%2|%3")
            .arg(normalizeText(query))
            .arg(latCell)
            .arg(lonCell);
}

QString rateLimitMessage(qint64 untilMs)
{
    const qint64 waitMs = std::max<qint64>(1000, untilMs - QDateTime::currentMSecsSinceEpoch());
    return QStringLiteral("Dịch vụ tìm kiếm đang bị giới hạn. Vui lòng chờ %1 giây.")
            .arg(static_cast<int>(std::ceil(waitMs / 1000.0)));
}

} // namespace

SearchController::SearchController(QObject *parent)
    : QObject(parent)
{
    m_debounce.setInterval(350);
    m_debounce.setSingleShot(true);
    connect(&m_debounce, &QTimer::timeout,
            this, &SearchController::doSearch);
}

void SearchController::setGoongRestApiKey(const QString &apiKey)
{
    m_goongRestApiKey = apiKey.trimmed();
}

void SearchController::scheduleSearch(const QString &query,
                                      double currentLat,
                                      double currentLon)
{
    const QString trimmed = query.trimmed();
    m_pendingQuery = trimmed;
    m_pendingLat   = currentLat;
    m_pendingLon   = currentLon;

    if (trimmed.size() < kMinSearchChars) {
        m_debounce.stop();
        ++m_requestSeq;
        cancelActiveRequest();
    setSearching(false);
    setStatusMessage({});
    setResults(QVariantList{});
        return;
    }

    ++m_requestSeq;
    cancelActiveRequest();

    const QString key = searchCacheKey(trimmed, currentLat, currentLon);
    const auto cached = m_cache.constFind(key);
    if (cached != m_cache.constEnd()) {
        m_debounce.stop();
    setStatusMessage({});
    setSearching(false);
    setResults(cached.value());
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_rateLimitedUntilMs > now) {
        m_debounce.stop();
    setSearching(false);
    setStatusMessage(rateLimitMessage(m_rateLimitedUntilMs));
    setResults(QVariantList{});
        return;
    }

    setStatusMessage({});
    setSearching(true);
    m_debounce.start();
}

void SearchController::clear()
{
    m_debounce.stop();
    ++m_requestSeq;
    cancelActiveRequest();
    m_pendingQuery.clear();
    setSearching(false);
    setStatusMessage({});
    setResults(QVariantList{});
}

void SearchController::doSearch()
{
    const QString q = m_pendingQuery;
    if (q.size() < kMinSearchChars) {
    setSearching(false);
    setStatusMessage({});
    setResults(QVariantList{});
        return;
    }

    const QString key = searchCacheKey(q, m_pendingLat, m_pendingLon);
    const auto cached = m_cache.constFind(key);
    if (cached != m_cache.constEnd()) {
    setSearching(false);
    setStatusMessage({});
    setResults(cached.value());
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_rateLimitedUntilMs > now) {
    setSearching(false);
    setStatusMessage(rateLimitMessage(m_rateLimitedUntilMs));
    setResults(QVariantList{});
        return;
    }

    ++m_requestSeq;
    const int seq = m_requestSeq;
    setResults(QVariantList{});
    sendSearchRequest(searchQueryForAttempt(q, m_pendingLat, m_pendingLon, 0),
                      q,
                      m_pendingLat,
                      m_pendingLon,
                      seq,
                      0);
}

void SearchController::sendSearchRequest(const QString &queryText,
                                         const QString &originalQuery,
                                         double currentLat,
                                         double currentLon,
                                         int seq,
                                         int attempt)
{
    if (m_goongRestApiKey.isEmpty()) {
        sendPhotonSearchRequest(searchQueryForAttempt(originalQuery, currentLat, currentLon, attempt),
                                originalQuery,
                                currentLat,
                                currentLon,
                                seq,
                                attempt);
        return;
    }

    QUrl url(QStringLiteral("https://rsapi.goong.io/v2/place/autocomplete"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("input"), queryText);
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("12"));
    query.addQueryItem(QStringLiteral("language"), QStringLiteral("vi"));
    query.addQueryItem(QStringLiteral("region"), QStringLiteral("vn"));
    query.addQueryItem(QStringLiteral("components"), QStringLiteral("country:vn"));
    query.addQueryItem(QStringLiteral("api_key"), m_goongRestApiKey);
    if (isValidCoordinate(currentLat, currentLon))
        query.addQueryItem(QStringLiteral("location"),
                           QStringLiteral("%1,%2")
                                   .arg(QString::number(currentLat, 'f', 7),
                                        QString::number(currentLon, 'f', 7)));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AGLNavigation/1.0"));
    req.setRawHeader("Accept", "application/json");
    req.setRawHeader("Accept-Language", "vi,en");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_nam.get(req);
    m_activeReply = reply;
    reply->setProperty("seq", seq);
    reply->setProperty("query", originalQuery);
    reply->setProperty("requestQuery", queryText);
    reply->setProperty("cacheKey", searchCacheKey(originalQuery, currentLat, currentLon));
    reply->setProperty("attempt", attempt);
    reply->setProperty("lat", currentLat);
    reply->setProperty("lon", currentLon);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleGoongAutocompleteReply(reply);
    });
}

void SearchController::sendPhotonSearchRequest(const QString &queryText,
                                               const QString &originalQuery,
                                               double currentLat,
                                               double currentLon,
                                               int seq,
                                               int attempt)
{
    QUrl url(QStringLiteral("https://photon.komoot.io/api"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), queryText);
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("15"));
    for (const QString &tag : osmTagsForQuery(originalQuery))
        query.addQueryItem(QStringLiteral("osm_tag"), tag);
    if (isValidCoordinate(currentLat, currentLon)) {
        query.addQueryItem(QStringLiteral("lat"), QString::number(currentLat, 'f', 7));
        query.addQueryItem(QStringLiteral("lon"), QString::number(currentLon, 'f', 7));
        query.addQueryItem(QStringLiteral("location_bias_scale"), QStringLiteral("1.0"));
    }
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AGLNavigation/1.0"));
    req.setRawHeader("Accept-Language", "vi,en");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_nam.get(req);
    m_activeReply = reply;

    // gắn metadata vào reply để xử lý đúng request
    reply->setProperty("seq", seq);
    reply->setProperty("query", originalQuery);
    reply->setProperty("requestQuery", queryText);
    reply->setProperty("cacheKey", searchCacheKey(originalQuery, currentLat, currentLon));
    reply->setProperty("attempt", attempt);
    reply->setProperty("lat", currentLat);
    reply->setProperty("lon", currentLon);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handlePhotonSearchReply(reply);
    });
}

void SearchController::sendGoongPlaceDetailRequest(const QString &placeId,
                                                   const QVariantMap &prediction,
                                                   const QString &originalQuery,
                                                   const QString &cacheKey,
                                                   double currentLat,
                                                   double currentLon,
                                                   int seq,
                                                   int rankSeed)
{
    QUrl url(QStringLiteral("https://rsapi.goong.io/v2/place/detail"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("place_id"), placeId);
    query.addQueryItem(QStringLiteral("language"), QStringLiteral("vi"));
    query.addQueryItem(QStringLiteral("region"), QStringLiteral("vn"));
    query.addQueryItem(QStringLiteral("api_key"), m_goongRestApiKey);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AGLNavigation/1.0"));
    req.setRawHeader("Accept", "application/json");
    req.setRawHeader("Accept-Language", "vi,en");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_nam.get(req);
    reply->setProperty("seq", seq);
    reply->setProperty("query", originalQuery);
    reply->setProperty("cacheKey", cacheKey);
    reply->setProperty("lat", currentLat);
    reply->setProperty("lon", currentLon);
    reply->setProperty("prediction", prediction);
    reply->setProperty("rankSeed", rankSeed);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleGoongPlaceDetailReply(reply);
    });
}

void SearchController::handleGoongAutocompleteReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (m_activeReply == reply)
        m_activeReply = nullptr;

    const int seq = reply->property("seq").toInt();
    if (seq < m_requestSeq)
        return;

    const QString originalQuery = reply->property("query").toString();
    const int attempt = reply->property("attempt").toInt();
    const double curLat = reply->property("lat").toDouble();
    const double curLon = reply->property("lon").toDouble();
    const QString cacheKey = reply->property("cacheKey").toString();

    if (reply->error() != QNetworkReply::NoError) {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray errorBody = reply->readAll();
        if (httpStatus == 429)
            m_rateLimitedUntilMs = QDateTime::currentMSecsSinceEpoch() + kRateLimitBackoffMs;
        qWarning() << "Goong autocomplete failed"
                   << httpStatus
                   << reply->errorString()
                   << reply->url()
                   << errorBody.left(300);
        sendPhotonSearchRequest(searchQueryForAttempt(originalQuery, curLat, curLon, attempt),
                                originalQuery,
                                curLat,
                                curLon,
                                seq,
                                attempt);
        return;
    }

    const QByteArray data = reply->readAll();
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Goong autocomplete JSON parse failed" << data.left(300);
        sendPhotonSearchRequest(searchQueryForAttempt(originalQuery, curLat, curLon, attempt),
                                originalQuery,
                                curLat,
                                curLon,
                                seq,
                                attempt);
        return;
    }

    QJsonArray predictions = doc.object().value(QStringLiteral("predictions")).toArray();
    if (predictions.isEmpty())
        predictions = doc.object().value(QStringLiteral("results")).toArray();
    if (predictions.isEmpty()) {
        const QString nextQuery = searchQueryForAttempt(originalQuery, curLat, curLon, attempt + 1);
        if (!nextQuery.isEmpty() && normalizeText(nextQuery) != normalizeText(reply->property("requestQuery").toString())) {
            sendSearchRequest(nextQuery,
                              originalQuery,
                              curLat,
                              curLon,
                              seq,
                              attempt + 1);
            return;
        }

        sendPhotonSearchRequest(searchQueryForAttempt(originalQuery, curLat, curLon, 1),
                                originalQuery,
                                curLat,
                                curLon,
                                seq,
                                1);
        return;
    }

    int detailCount = 0;
    for (int i = 0; i < predictions.size() && detailCount < 12; ++i) {
        const QJsonObject p = predictions.at(i).toObject();
        const QString placeId = p.value(QStringLiteral("place_id")).toString(
                    p.value(QStringLiteral("id")).toString()).trimmed();
        if (placeId.isEmpty())
            continue;

        sendGoongPlaceDetailRequest(placeId,
                                    p.toVariantMap(),
                                    originalQuery,
                                    cacheKey,
                                    curLat,
                                    curLon,
                                    seq,
                                    i);
        ++detailCount;
    }

    if (detailCount == 0) {
        sendPhotonSearchRequest(searchQueryForAttempt(originalQuery, curLat, curLon, 1),
                                originalQuery,
                                curLat,
                                curLon,
                                seq,
                                1);
    }
}

void SearchController::handleGoongPlaceDetailReply(QNetworkReply *reply)
{
    reply->deleteLater();

    const int seq = reply->property("seq").toInt();
    if (seq < m_requestSeq)
        return;

    const QString cacheKey = reply->property("cacheKey").toString();
    const QString query = reply->property("query").toString();
    const double curLat = reply->property("lat").toDouble();
    const double curLon = reply->property("lon").toDouble();
    const QVariantMap prediction = reply->property("prediction").toMap();
    const int rankSeed = reply->property("rankSeed").toInt();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Goong place detail failed"
                   << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                   << reply->errorString()
                   << reply->url()
                   << reply->readAll().left(300);
        if (m_results.isEmpty())
            setSearching(false);
        return;
    }

    const QByteArray data = reply->readAll();
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Goong place detail JSON parse failed" << data.left(300);
        if (m_results.isEmpty())
            setSearching(false);
        return;
    }

    const QJsonObject root = doc.object();
    QJsonObject result = root.value(QStringLiteral("result")).toObject();
    if (result.isEmpty()) {
        const QJsonArray results = root.value(QStringLiteral("results")).toArray();
        if (!results.isEmpty())
            result = results.at(0).toObject();
    }
    if (result.isEmpty())
        result = root;

    QJsonObject location = result.value(QStringLiteral("geometry")).toObject()
            .value(QStringLiteral("location")).toObject();
    if (location.isEmpty())
        location = result.value(QStringLiteral("location")).toObject();

    const double lat = location.value(QStringLiteral("lat")).toDouble(std::numeric_limits<double>::quiet_NaN());
    const double lon = location.contains(QStringLiteral("lng"))
            ? location.value(QStringLiteral("lng")).toDouble(std::numeric_limits<double>::quiet_NaN())
            : location.value(QStringLiteral("lon")).toDouble(std::numeric_limits<double>::quiet_NaN());
    if (!isValidCoordinate(lat, lon)) {
        if (m_results.isEmpty())
            setSearching(false);
        return;
    }

    const QVariantMap structured = prediction.value(QStringLiteral("structured_formatting")).toMap();
    const QString description = prediction.value(QStringLiteral("description")).toString().trimmed();
    const QString mainText = structured.value(QStringLiteral("main_text")).toString().trimmed();
    const QString secondaryText = structured.value(QStringLiteral("secondary_text")).toString().trimmed();
    const QString primary = firstNonEmpty({
            result.value(QStringLiteral("name")).toString(),
            mainText,
            leadingDescriptionPart(description),
            query
    });
    const QString secondary = firstNonEmpty({
            result.value(QStringLiteral("formatted_address")).toString(),
            result.value(QStringLiteral("address")).toString(),
            secondaryText,
            description
    });
    const QString displayName = fullDisplayName(primary, secondary);
    const double meters = isValidCoordinate(curLat, curLon)
            ? distanceMeters(curLat, curLon, lat, lon)
            : std::numeric_limits<double>::quiet_NaN();

    QVariantMap item;
    item.insert(QStringLiteral("display_name"), displayName);
    item.insert(QStringLiteral("primary"), primary.isEmpty() ? secondary : primary);
    item.insert(QStringLiteral("secondary"), secondary);
    item.insert(QStringLiteral("typeLabel"), QStringLiteral("Địa điểm"));
    item.insert(QStringLiteral("distanceText"), distanceText(meters));
    item.insert(QStringLiteral("lat"), lat);
    item.insert(QStringLiteral("lon"), lon);
    item.insert(QStringLiteral("distance"), meters);
    item.insert(QStringLiteral("place_id"), result.value(QStringLiteral("place_id")).toString(
                    prediction.value(QStringLiteral("place_id")).toString()));
    item.insert(QStringLiteral("rank"), 4000000 + textScore(query, primary + QLatin1Char(' ') + secondary) * 100
                + proximityScore(meters) - rankSeed);

    QVariantList next = m_results;
    const QString key = resultKey(item.value(QStringLiteral("primary")).toString(), lat, lon);
    for (const QVariant &existing : next) {
        const QVariantMap map = existing.toMap();
        if (resultKey(map.value(QStringLiteral("primary")).toString(),
                      map.value(QStringLiteral("lat")).toDouble(),
                      map.value(QStringLiteral("lon")).toDouble()) == key) {
            return;
        }
    }

    next.push_back(item);
    std::sort(next.begin(), next.end(), [](const QVariant &a, const QVariant &b) {
        return a.toMap().value(QStringLiteral("rank")).toInt()
                > b.toMap().value(QStringLiteral("rank")).toInt();
    });
    while (next.size() > 10)
        next.removeLast();

    setSearching(false);
    setStatusMessage({});
    setResults(next);
    if (!cacheKey.isEmpty()) {
        if (!m_cache.contains(cacheKey))
            m_cacheOrder.push_back(cacheKey);
        m_cache.insert(cacheKey, next);
        while (m_cacheOrder.size() > kMaxCacheEntries)
            m_cache.remove(m_cacheOrder.takeFirst());
    }
}

void SearchController::handlePhotonSearchReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (m_activeReply == reply)
        m_activeReply = nullptr;

    const int seq = reply->property("seq").toInt();
    if (seq < m_requestSeq)
        return;

    if (reply->error() != QNetworkReply::NoError) {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray errorBody = reply->readAll();
        QString error;
        if (httpStatus == 429)
            m_rateLimitedUntilMs = QDateTime::currentMSecsSinceEpoch() + kRateLimitBackoffMs;

        if (httpStatus == 429) {
            error = rateLimitMessage(m_rateLimitedUntilMs);
        } else if (httpStatus > 0) {
            error = QStringLiteral("Lỗi dịch vụ tìm kiếm %1").arg(httpStatus);
        } else {
            error = QStringLiteral("Lỗi dịch vụ tìm kiếm");
        }
        qWarning() << error << reply->errorString() << reply->url() << errorBody.left(300);
        setStatusMessage(error);
        setSearching(false);
        setResults(QVariantList{});
        return;
    }

    const QByteArray data = reply->readAll();
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        const QString error = QStringLiteral("Lỗi đọc dữ liệu tìm kiếm");
        qWarning() << error << reply->url() << data.left(200);
        setStatusMessage(error);
        setSearching(false);
        setResults(QVariantList{});
        return;
    }
    const QJsonArray arr = doc.object().value(QStringLiteral("features")).toArray();

    const QString query = reply->property("query").toString();
    const QString requestQuery = reply->property("requestQuery").toString();
    const int attempt = reply->property("attempt").toInt();
    const double curLat = reply->property("lat").toDouble();
    const double curLon = reply->property("lon").toDouble();
    const bool hasCurrentCoord = isValidCoordinate(curLat, curLon);

    if (arr.isEmpty()) {
        const QString nextQuery = searchQueryForAttempt(query, curLat, curLon, attempt + 1);
        if (!nextQuery.isEmpty() && normalizeText(nextQuery) != normalizeText(requestQuery)) {
            sendPhotonSearchRequest(nextQuery, query, curLat, curLon, seq, attempt + 1);
            return;
        }
    }

    struct Result {
        QString displayName;
        QString primary;
        QString secondary;
        QString typeLabel;
        QString distanceText;
        double lat;
        double lon;
        double distance = 0.0;
        double importance = 0.0;
        bool inVietnam = false;
        int tscore = 0;
        int rank = 0;
    };

    QVector<Result> tmp;
    tmp.reserve(arr.size());
    QSet<QString> seen;
    bool hasVietnamResult = false;
    double bestDistance = std::numeric_limits<double>::infinity();

    for (const QJsonValue &v : arr) {
        const QJsonObject feature = v.toObject();
        const QJsonObject o = feature.value(QStringLiteral("properties")).toObject();
        const QJsonObject geometry = feature.value(QStringLiteral("geometry")).toObject();
        const QJsonArray coordinates = geometry.value(QStringLiteral("coordinates")).toArray();
        if (coordinates.size() < 2)
            continue;

        Result r;
        r.lat = coordinates.at(1).toDouble();
        r.lon = coordinates.at(0).toDouble();
        if (!std::isfinite(r.lat) || !std::isfinite(r.lon))
            continue;

        const QString name = o.value(QStringLiteral("name")).toString().trimmed();
        const QString street = o.value(QStringLiteral("street")).toString().trimmed();
        const QString city = o.value(QStringLiteral("city")).toString().trimmed();
        const QString district = o.value(QStringLiteral("district")).toString().trimmed();
        const QString state = o.value(QStringLiteral("state")).toString().trimmed();
        const QString country = o.value(QStringLiteral("country")).toString().trimmed();
        const QString countryCode = o.value(QStringLiteral("countrycode")).toString().trimmed().toUpper();
        const QString osmKey = o.value(QStringLiteral("osm_key")).toString();
        const QString osmValue = o.value(QStringLiteral("osm_value")).toString();

        r.primary = !name.isEmpty() ? name : (!street.isEmpty() ? street : (!city.isEmpty() ? city : query));
        QStringList secondaryParts;
        const QStringList secondaryCandidates = { street, district, city, state, country };
        for (const QString &part : secondaryCandidates) {
            if (!part.isEmpty() && normalizeText(part) != normalizeText(r.primary) &&
                !secondaryParts.contains(part)) {
                secondaryParts.push_back(part);
            }
        }
        r.secondary = secondaryParts.join(QStringLiteral(", "));
        r.displayName = r.secondary.isEmpty()
                ? r.primary
                : r.primary + QStringLiteral(", ") + r.secondary;
        if (r.primary.trimmed().isEmpty())
            continue;

        r.distance = hasCurrentCoord
                ? distanceMeters(curLat, curLon, r.lat, r.lon)
                : std::numeric_limits<double>::quiet_NaN();
        if (std::isfinite(r.distance))
            bestDistance = std::min(bestDistance, r.distance);
        r.importance = o.value(QStringLiteral("extent")).isArray() ? 0.5 : 0.0;
        r.inVietnam = countryCode == QStringLiteral("VN") ||
                normalizeText(country).contains(QStringLiteral("viet nam"));
        hasVietnamResult = hasVietnamResult || r.inVietnam;
        r.typeLabel = typeLabel(osmKey, osmValue);
        r.distanceText = distanceText(r.distance);
        r.tscore = textScore(query, r.primary + QLatin1Char(' ') + r.secondary);
        const QString normalizedSecondary = normalizeText(r.secondary);
        const int localScore = normalizedSecondary.contains(QStringLiteral("binh thanh")) ? 450
                : ((normalizedSecondary.contains(QStringLiteral("ho chi minh")) ||
                    normalizedSecondary.contains(QStringLiteral("sai gon")) ||
                    normalizedSecondary.contains(QStringLiteral("hcm"))) ? 300 : 0);
        r.rank = (r.inVietnam ? 2000000 : -2000000)
                + proximityScore(r.distance)
                + r.tscore * 100
                + categoryScore(query, osmKey, osmValue) * 120
                + localScore
                + classPriority(osmKey, osmValue)
                + distanceScore(r.distance) * 50
                + qRound(r.importance * 200.0);

        const QString key = resultKey(r.primary, r.lat, r.lon);
        if (seen.contains(key))
            continue;
        seen.insert(key);
        tmp.push_back(r);
    }

    if (hasCurrentCoord && attempt == 0 && !queryHasExplicitLocation(query) &&
        (tmp.isEmpty() || bestDistance > 25000.0)) {
        const QString nextQuery = searchQueryForAttempt(query, curLat, curLon, 1);
        if (!nextQuery.isEmpty() && normalizeText(nextQuery) != normalizeText(requestQuery)) {
            sendPhotonSearchRequest(nextQuery, query, curLat, curLon, seq, 1);
            return;
        }
    }

    if (!hasVietnamResult && attempt < 4) {
        const QString nextQuery = searchQueryForAttempt(query, curLat, curLon, attempt + 1);
        if (!nextQuery.isEmpty() && normalizeText(nextQuery) != normalizeText(requestQuery)) {
            sendPhotonSearchRequest(nextQuery, query, curLat, curLon, seq, attempt + 1);
            return;
        }
    }

    std::sort(tmp.begin(), tmp.end(), [](const Result &a, const Result &b) {
        if (a.rank != b.rank)
            return a.rank > b.rank;
        return a.distance < b.distance;
    });

    QVariantList out;
    const qsizetype maxResults = std::min<qsizetype>(tmp.size(), 10);
    out.reserve(maxResults);
    for (qsizetype i = 0; i < tmp.size() && out.size() < maxResults; ++i) {
        const Result &r = tmp.at(i);
        if (hasVietnamResult && !r.inVietnam)
            continue;

        QVariantMap m;
        m.insert("display_name", r.displayName);
        m.insert("primary", r.primary);
        m.insert("secondary", r.secondary);
        m.insert("typeLabel", r.typeLabel);
        m.insert("distanceText", r.distanceText);
        m.insert("lat", r.lat);
        m.insert("lon", r.lon);
        m.insert("distance", r.distance);
        m.insert("tscore", r.tscore);
        m.insert("rank", r.rank);
        out.push_back(m);
    }

    setSearching(false);
    setStatusMessage({});
    const QString cacheKey = reply->property("cacheKey").toString();
    if (!cacheKey.isEmpty()) {
        if (!m_cache.contains(cacheKey))
            m_cacheOrder.push_back(cacheKey);
        m_cache.insert(cacheKey, out);
        while (m_cacheOrder.size() > kMaxCacheEntries)
            m_cache.remove(m_cacheOrder.takeFirst());
    }
    setResults(out);
}

void SearchController::cancelActiveRequest()
{
    if (!m_activeReply)
        return;

    QNetworkReply *reply = m_activeReply;
    m_activeReply = nullptr;
    reply->abort();
}

void SearchController::setResults(const QVariantList &results)
{
    if (m_results == results)
        return;

    m_results = results;
    emit resultsChanged();
}

void SearchController::setSearching(bool searching)
{
    if (m_searching == searching)
        return;

    m_searching = searching;
    emit searchingChanged();
}

void SearchController::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message)
        return;

    m_statusMessage = message;
    emit statusMessageChanged();
}

int SearchController::textScore(const QString &query,
                                const QString &name) const
{
    const QString q = normalizeText(query);
    const QString n = normalizeText(name);
    if (q.isEmpty() || n.isEmpty())
        return 0;

    if (n == q)
        return 1000;
    if (n.startsWith(q))
        return 930;

    const QStringList qWords = q.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    const QStringList nWords = n.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    int score = n.contains(q) ? 620 : 0;
    int matched = 0;

    for (const QString &qw : qWords) {
        int best = 0;
        for (const QString &nw : nWords) {
            if (nw == qw) {
                best = std::max(best, 160);
            } else if (nw.startsWith(qw)) {
                best = std::max(best, 130);
            } else if (nw.contains(qw)) {
                best = std::max(best, 90);
            } else if (qw.size() >= 4 && editDistanceLimited(qw, nw.left(qw.size()), 1) <= 1) {
                best = std::max(best, 70);
            } else if (qw.size() >= 5 && editDistanceLimited(qw, nw, 2) <= 2) {
                best = std::max(best, 55);
            }
        }

        if (best > 0) {
            ++matched;
            score += best;
        }
    }

    if (matched == qWords.size())
        score += 260;
    else if (matched > 0)
        score += 80;

    const int lengthDelta = std::abs(static_cast<int>(n.size() - q.size()));
    score -= std::min(120, lengthDelta * 2);
    return std::max(score, 0);
}
