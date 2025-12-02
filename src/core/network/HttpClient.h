#ifndef CORE_NETWORK_HTTP_CLIENT_H
#define CORE_NETWORK_HTTP_CLIENT_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QJsonObject>

class HttpClient final : public QObject{
Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr);
    void setAccessToken(const QString &accessToken);
    void clearAccessToken();

    void get(const QString &path);
    void post(const QString &path, const QJsonObject &body);
    void del(const QString &path);

signals:
    // success JSON-response
    void success(const QJsonDocument &doc);
    // any HTTP error, instead of 401
    void error(const QString &message);
    // signal that catches 401 error
    void unauthorized();

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_manager;
    QString m_baseUrl;
    QString m_accessToken;

    QNetworkRequest makeRequest(const QString &path);
    void handleReply(QNetworkReply *reply);
};

#endif //CORE_NETWORK_HTTP_CLIENT_H
