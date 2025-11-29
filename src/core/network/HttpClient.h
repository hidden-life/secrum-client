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
    void post(const QString &path, const QJsonObject &body);

signals:
    void success(QJsonObject data);
    void error(QString message);

private:
    QNetworkAccessManager m_manager;
    QString m_baseUrl = "http://localhost:8080";
};

#endif //CORE_NETWORK_HTTP_CLIENT_H
