#include "HttpClient.h"

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QNetworkReply>

HttpClient::HttpClient(QObject *parent) : QObject(parent) {}

void HttpClient::post(const QString &path, const QJsonObject &body) {
    QNetworkRequest req(QUrl(m_baseUrl + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto payload = QJsonDocument(body).toJson();
    auto * reply = m_manager.post(req, payload);

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        QByteArray data = reply->readAll();

        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString());
            reply->deleteLater();
            return;
        }

        auto json = QJsonDocument::fromJson(data);
        emit success(json.object());

        reply->deleteLater();
    });
}
