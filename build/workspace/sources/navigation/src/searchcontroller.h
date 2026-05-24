#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QHash>
#include <QStringList>

class SearchController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList results READ results NOTIFY resultsChanged)
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit SearchController(QObject *parent = nullptr);

    void setGoongRestApiKey(const QString &apiKey);

    // QML gọi hàm này mỗi khi text thay đổi
    Q_INVOKABLE void scheduleSearch(const QString &query,
                                    double currentLat,
                                    double currentLon);
    Q_INVOKABLE void clear();

    QVariantList results() const { return m_results; }
    bool searching() const { return m_searching; }
    QString statusMessage() const { return m_statusMessage; }

signals:
    void resultsChanged();
    void searchingChanged();
    void statusMessageChanged();

private slots:
    void doSearch();

private:
    void sendSearchRequest(const QString &query,
                           const QString &originalQuery,
                           double currentLat,
                           double currentLon,
                           int seq,
                           int attempt);
    void sendPhotonSearchRequest(const QString &query,
                                 const QString &originalQuery,
                                 double currentLat,
                                 double currentLon,
                                 int seq,
                                 int attempt);
    void sendGoongPlaceDetailRequest(const QString &placeId,
                                     const QVariantMap &prediction,
                                     const QString &originalQuery,
                                     const QString &cacheKey,
                                     double currentLat,
                                     double currentLon,
                                     int seq,
                                     int rankSeed);
    void handlePhotonSearchReply(QNetworkReply *reply);
    void handleGoongAutocompleteReply(QNetworkReply *reply);
    void handleGoongPlaceDetailReply(QNetworkReply *reply);
    void cancelActiveRequest();
    void setResults(const QVariantList &results);
    void setSearching(bool searching);
    void setStatusMessage(const QString &message);
    int textScore(const QString &query, const QString &name) const;

    QNetworkAccessManager m_nam;
    QPointer<QNetworkReply> m_activeReply;
    QTimer m_debounce;

    int m_requestSeq = 0;         // id tăng dần cho mỗi request
    QString m_pendingQuery;
    double m_pendingLat = 0.0;
    double m_pendingLon = 0.0;

    QVariantList m_results;
    QHash<QString, QVariantList> m_cache;
    QStringList m_cacheOrder;
    QString m_goongRestApiKey;
    bool m_searching = false;
    QString m_statusMessage;
    qint64 m_rateLimitedUntilMs = 0;
};
