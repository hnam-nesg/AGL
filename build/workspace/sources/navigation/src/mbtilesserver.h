#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QTcpServer>
#include <QString>

struct sqlite3;

class QTcpSocket;

class MbTilesServer : public QObject
{
public:
    explicit MbTilesServer(QObject *parent = nullptr);
    ~MbTilesServer() override;

    bool start(quint16 port = 8080);
    bool isListening() const;
    QString mbtilesPath() const;
    quint16 port() const;

private:
    void acceptConnections();
    void handleSocket(QTcpSocket *socket);
    void handleRequest(QTcpSocket *socket, const QByteArray &request);

    bool ensureDatabaseOpen();
    bool openDatabase(const QString &path);
    QString resolveMbtilesPath() const;
    bool fetchTile(int z, int x, int y, QByteArray *tile);
    bool fetchTileRow(int z, int x, int tileRow, QByteArray *tile);
    QByteArray metadataJson();

    void sendResponse(QTcpSocket *socket,
                      int statusCode,
                      const QByteArray &reason,
                      const QByteArray &contentType,
                      const QByteArray &body,
                      const QList<QByteArray> &extraHeaders = {});

    QTcpServer m_server;
    sqlite3 *m_db = nullptr;
    QString m_mbtilesPath;
    quint16 m_port = 0;
    bool m_warnedMissing = false;
};
