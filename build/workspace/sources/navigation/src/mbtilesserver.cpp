#include "mbtilesserver.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QStringList>
#include <QTcpSocket>
#include <QUrl>

#include <sqlite3.h>

namespace {

constexpr auto kDefaultMbtilesPath = "/usr/share/navigation/maps/vietnam.mbtiles";
constexpr auto kVectorTilePathPrefix = "/data/v3";

bool isGzip(const QByteArray &data)
{
    return data.size() >= 2
        && static_cast<unsigned char>(data.at(0)) == 0x1f
        && static_cast<unsigned char>(data.at(1)) == 0x8b;
}

QByteArray statusText(int statusCode, const QByteArray &fallback)
{
    switch (statusCode) {
    case 200: return "OK";
    case 204: return "No Content";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 503: return "Service Unavailable";
    default: return fallback;
    }
}

bool insertNumber(QJsonObject *object, const QString &key, const QString &value)
{
    bool ok = false;
    const double number = value.toDouble(&ok);
    if (!ok)
        return false;

    object->insert(key, number);
    return true;
}

bool insertNumberArray(QJsonObject *object, const QString &key, const QString &value, qsizetype minimumSize)
{
    const QStringList parts = value.split(',', Qt::SkipEmptyParts);
    if (parts.size() < minimumSize)
        return false;

    QJsonArray array;
    for (const QString &part : parts) {
        bool ok = false;
        const double number = part.trimmed().toDouble(&ok);
        if (!ok)
            return false;
        array.append(number);
    }

    object->insert(key, array);
    return true;
}

void applyMbtilesMetadata(QJsonObject *metadata, const QString &name, const QString &value)
{
    if (name == QStringLiteral("name")
            || name == QStringLiteral("description")
            || name == QStringLiteral("attribution")
            || name == QStringLiteral("version")
            || name == QStringLiteral("id")
            || name == QStringLiteral("format")) {
        metadata->insert(name, value);
        return;
    }

    if (name == QStringLiteral("minzoom") || name == QStringLiteral("maxzoom")) {
        insertNumber(metadata, name, value);
        return;
    }

    if (name == QStringLiteral("bounds")) {
        insertNumberArray(metadata, name, value, 4);
        return;
    }

    if (name == QStringLiteral("center")) {
        insertNumberArray(metadata, name, value, 2);
        return;
    }

    if (name == QStringLiteral("json")) {
        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError && document.isObject()) {
            const QJsonObject json = document.object();
            if (json.contains(QStringLiteral("vector_layers")))
                metadata->insert(QStringLiteral("vector_layers"), json.value(QStringLiteral("vector_layers")));
        }
    }
}

} // namespace

MbTilesServer::MbTilesServer(QObject *parent)
    : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, [this]() {
        acceptConnections();
    });
}

MbTilesServer::~MbTilesServer()
{
    if (m_db)
        sqlite3_close(m_db);
}

bool MbTilesServer::start(quint16 port)
{
    m_port = port;
    ensureDatabaseOpen();

    if (m_server.isListening())
        return true;

    if (!m_server.listen(QHostAddress::LocalHost, port)) {
        qWarning() << "Failed to open local tile server 127.0.0.1:" << port
                   << m_server.errorString();
        return false;
    }

    qInfo() << "Local MBTiles server:"
            << QStringLiteral("http://127.0.0.1:%1%2/{z}/{x}/{y}.pbf").arg(port).arg(kVectorTilePathPrefix)
            << "mbtiles=" << m_mbtilesPath;
    return true;
}

bool MbTilesServer::isListening() const
{
    return m_server.isListening();
}

QString MbTilesServer::mbtilesPath() const
{
    return m_mbtilesPath;
}

quint16 MbTilesServer::port() const
{
    return m_server.serverPort();
}

void MbTilesServer::acceptConnections()
{
    while (QTcpSocket *socket = m_server.nextPendingConnection())
        handleSocket(socket);
}

void MbTilesServer::handleSocket(QTcpSocket *socket)
{
    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);

    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        QByteArray buffer = socket->property("httpBuffer").toByteArray();
        buffer += socket->readAll();

        if (!buffer.contains("\r\n\r\n") && buffer.size() < 16384) {
            socket->setProperty("httpBuffer", buffer);
            return;
        }

        socket->setProperty("httpBuffer", QByteArray());
        handleRequest(socket, buffer);
    });
}

void MbTilesServer::handleRequest(QTcpSocket *socket, const QByteArray &request)
{
    const QList<QByteArray> lines = request.split('\n');
    if (lines.isEmpty()) {
        sendResponse(socket, 400, {}, "text/plain", "bad request\n");
        return;
    }

    const QList<QByteArray> parts = lines.first().trimmed().split(' ');
    if (parts.size() < 2) {
        sendResponse(socket, 400, {}, "text/plain", "bad request\n");
        return;
    }

    const QByteArray method = parts.at(0);
    const QUrl url(QString::fromUtf8(parts.at(1)));
    const QString path = url.path();

    if (method == "OPTIONS") {
        sendResponse(socket, 204, {}, "text/plain", {});
        return;
    }

    if (method != "GET" && method != "HEAD") {
        sendResponse(socket, 405, {}, "text/plain", "method not allowed\n");
        return;
    }

    if (path == "/" || path == "/health") {
        const QByteArray body = ensureDatabaseOpen()
            ? QByteArray("ok\n")
            : QByteArray("missing mbtiles: ") + m_mbtilesPath.toUtf8() + QByteArray("\n");
        sendResponse(socket, m_db ? 200 : 503, {}, "text/plain", method == "HEAD" ? QByteArray() : body);
        return;
    }

    if (path == "/metadata.json" || path == "/data/v3.json") {
        const QByteArray body = metadataJson();
        sendResponse(socket, m_db ? 200 : 503, {}, "application/json", method == "HEAD" ? QByteArray() : body);
        return;
    }

    static const QRegularExpression tilePattern(
        QStringLiteral("^/data/v3/(\\d+)/(\\d+)/(\\d+)\\.pbf$"));
    const QRegularExpressionMatch match = tilePattern.match(path);

    if (!match.hasMatch()) {
        sendResponse(socket, 404, {}, "text/plain", "not found\n");
        return;
    }

    const int z = match.captured(1).toInt();
    const int x = match.captured(2).toInt();
    const int y = match.captured(3).toInt();

    if (z < 0 || z > 30 || x < 0 || y < 0) {
        sendResponse(socket, 400, {}, "text/plain", "invalid tile\n");
        return;
    }

    QByteArray tile;
    if (!fetchTile(z, x, y, &tile)) {
        sendResponse(socket, m_db ? 404 : 503, {}, "text/plain", "tile not found\n");
        return;
    }

    QList<QByteArray> extraHeaders = {
        "Cache-Control: public, max-age=604800"
    };
    if (isGzip(tile))
        extraHeaders.append("Content-Encoding: gzip");

    sendResponse(socket,
                 200,
                 {},
                 "application/vnd.mapbox-vector-tile",
                 method == "HEAD" ? QByteArray() : tile,
                 extraHeaders);
}

bool MbTilesServer::ensureDatabaseOpen()
{
    if (m_db)
        return true;

    m_mbtilesPath = resolveMbtilesPath();
    if (!QFileInfo::exists(m_mbtilesPath)) {
        if (!m_warnedMissing) {
            qWarning() << "Missing MBTiles:" << m_mbtilesPath
                       << "set NAVIGATION_MBTILES or copy the file to"
                       << kDefaultMbtilesPath;
            m_warnedMissing = true;
        }
        return false;
    }

    return openDatabase(m_mbtilesPath);
}

bool MbTilesServer::openDatabase(const QString &path)
{
    sqlite3 *db = nullptr;
    const QByteArray localPath = QFileInfo(path).absoluteFilePath().toLocal8Bit();
    const int rc = sqlite3_open_v2(localPath.constData(),
                                   &db,
                                   SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX,
                                   nullptr);

    if (rc != SQLITE_OK) {
        qWarning() << "Failed to open MBTiles" << path << sqlite3_errmsg(db);
        if (db)
            sqlite3_close(db);
        return false;
    }

    sqlite3_busy_timeout(db, 1000);
    sqlite3_exec(db, "PRAGMA query_only=ON; PRAGMA cache_size=-20000;", nullptr, nullptr, nullptr);

    m_db = db;
    m_mbtilesPath = QFileInfo(path).absoluteFilePath();
    qInfo() << "Opened MBTiles:" << m_mbtilesPath;
    return true;
}

QString MbTilesServer::resolveMbtilesPath() const
{
    const QString envPath = qEnvironmentVariable("NAVIGATION_MBTILES");
    if (!envPath.isEmpty())
        return envPath;

    const QString appRelative = QCoreApplication::applicationDirPath()
        + QStringLiteral("/../share/navigation/maps/vietnam.mbtiles");

    const QStringList candidates = {
        kDefaultMbtilesPath,
        QStringLiteral("/usr/share/navigation/vietnam.mbtiles"),
        QStringLiteral("/var/lib/navigation/vietnam.mbtiles"),
        appRelative
    };

    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate))
            return candidate;
    }

    return kDefaultMbtilesPath;
}

bool MbTilesServer::fetchTile(int z, int x, int y, QByteArray *tile)
{
    if (!ensureDatabaseOpen())
        return false;

    const int maxIndex = (1 << z) - 1;
    if (x > maxIndex || y > maxIndex)
        return false;

    const int tmsY = maxIndex - y;
    if (fetchTileRow(z, x, tmsY, tile))
        return true;

    return tmsY != y && fetchTileRow(z, x, y, tile);
}

bool MbTilesServer::fetchTileRow(int z, int x, int tileRow, QByteArray *tile)
{
    static constexpr auto sql =
        "SELECT tile_data FROM tiles "
        "WHERE zoom_level=?1 AND tile_column=?2 AND tile_row=?3 "
        "LIMIT 1";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, z);
    sqlite3_bind_int(stmt, 2, x);
    sqlite3_bind_int(stmt, 3, tileRow);

    const int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    const void *blob = sqlite3_column_blob(stmt, 0);
    const int size = sqlite3_column_bytes(stmt, 0);
    tile->clear();
    if (blob && size > 0)
        tile->append(static_cast<const char *>(blob), size);

    sqlite3_finalize(stmt);
    return true;
}

QByteArray MbTilesServer::metadataJson()
{
    QJsonObject metadata;
    metadata.insert(QStringLiteral("tilejson"), QStringLiteral("2.2.0"));
    metadata.insert(QStringLiteral("name"), QStringLiteral("vietnam-openmaptiles"));
    metadata.insert(QStringLiteral("format"), QStringLiteral("pbf"));
    metadata.insert(QStringLiteral("scheme"), QStringLiteral("xyz"));
    metadata.insert(QStringLiteral("minzoom"), 0);
    metadata.insert(QStringLiteral("maxzoom"), 14);

    QJsonArray tiles;
    tiles.append(QStringLiteral("http://127.0.0.1:%1/data/v3/{z}/{x}/{y}.pbf").arg(m_port ? m_port : 8080));
    metadata.insert(QStringLiteral("tiles"), tiles);

    if (ensureDatabaseOpen()) {
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(m_db, "SELECT name, value FROM metadata", -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const auto *name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                const auto *value = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                if (name && value)
                    applyMbtilesMetadata(&metadata, QString::fromUtf8(name), QString::fromUtf8(value));
            }
        }
        sqlite3_finalize(stmt);
    }

    metadata.insert(QStringLiteral("scheme"), QStringLiteral("xyz"));
    metadata.insert(QStringLiteral("tiles"), tiles);

    return QJsonDocument(metadata).toJson(QJsonDocument::Compact) + '\n';
}

void MbTilesServer::sendResponse(QTcpSocket *socket,
                                 int statusCode,
                                 const QByteArray &reason,
                                 const QByteArray &contentType,
                                 const QByteArray &body,
                                 const QList<QByteArray> &extraHeaders)
{
    QByteArray response;
    response += "HTTP/1.1 " + QByteArray::number(statusCode) + ' '
        + statusText(statusCode, reason) + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: GET, HEAD, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n";
    response += "Connection: close\r\n";

    for (const QByteArray &header : extraHeaders)
        response += header + "\r\n";

    response += "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n";
    response += body;

    socket->write(response);
    socket->disconnectFromHost();
}
