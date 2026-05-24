#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

class QNetworkReply;

class GoongRestClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY apiKeyChanged)
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)

public:
    explicit GoongRestClient(QObject *parent = nullptr);

    bool available() const;
    QString apiKey() const { return m_apiKey; }
    void setApiKey(const QString &apiKey);

    Q_INVOKABLE int autocomplete(const QString &input,
                                 double lat,
                                 double lon,
                                 int limit = 8);
    Q_INVOKABLE int forwardGeocode(double lat,
                                   double lon,
                                   const QString &address);
    Q_INVOKABLE int reverseGeocode(double lat,
                                   double lon);
    Q_INVOKABLE int placeDetail(const QString &placeId);
    Q_INVOKABLE int direction(double originLat,
                              double originLon,
                              double destinationLat,
                              double destinationLon);
    Q_INVOKABLE int direction(double originLat,
                              double originLon,
                              double destinationLat,
                              double destinationLon,
                              const QString &vehicle);
    Q_INVOKABLE int distanceMatrix(const QVariantList &origins,
                                   const QVariantList &destinations);
    Q_INVOKABLE int distanceMatrix(const QVariantList &origins,
                                   const QVariantList &destinations,
                                   const QString &vehicle);
    Q_INVOKABLE int geolocation(const QVariantList &cells,
                                const QVariantList &wifiAccessPoints);
    Q_INVOKABLE QString staticMapUrl(double lat,
                                     double lon,
                                     int zoom = 15,
                                     int width = 640,
                                     int height = 360);
    Q_INVOKABLE QString staticMapRouteUrl(double originLat,
                                          double originLon,
                                          double destinationLat,
                                          double destinationLon);
    Q_INVOKABLE QString staticMapRouteUrl(double originLat,
                                          double originLon,
                                          double destinationLat,
                                          double destinationLon,
                                          int width,
                                          int height,
                                          const QString &vehicle);
    Q_INVOKABLE int trip(const QVariantList &points,
                         const QString &vehicle);
    Q_INVOKABLE int trip(const QVariantList &points);

signals:
    void apiKeyChanged();
    void replyReady(int requestId,
                    const QString &api,
                    const QVariantMap &payload);
    void requestFailed(int requestId,
                       const QString &api,
                       const QString &message);

private:
    int nextRequestId();
    void getJson(const QString &api,
                 const QUrl &url,
                 int requestId);
    void postJson(const QString &api,
                  const QUrl &url,
                  const QVariantMap &body,
                  int requestId);
    void handleReply(QNetworkReply *reply);
    QUrl restUrl(const QString &path) const;
    QString coordString(double lat, double lon) const;
    QString coordListString(const QVariantList &points) const;
    QStringList pointStrings(const QVariantList &points) const;

    QNetworkAccessManager m_nam;
    QString m_apiKey;
    int m_nextRequestId = 0;
};
