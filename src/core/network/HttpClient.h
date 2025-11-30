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
    void setBaseUrl(const QString &baseUrl);
    void setAccessToken(const QString &accessToken);

    void get(const QString &path);
    void post(const QString &path, const QJsonObject &body);

signals:
    void success(const QJsonDocument &doc);
    void error(const QString &message);

private:
    QNetworkAccessManager m_manager;
    QString m_baseUrl;
    QString m_accessToken;

    QNetworkRequest makeRequest(const QString &path);
    void handleReply(QNetworkReply *reply);
};

#endif //CORE_NETWORK_HTTP_CLIENT_H
