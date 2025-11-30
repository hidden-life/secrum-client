#include "HttpClient.h"

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QNetworkReply>

#include "core/auth/AuthSession.h"
#include "core/config/ClientConfiguration.h"

HttpClient::HttpClient(QObject *parent) : QObject(parent) {
    m_baseUrl = ClientConfiguration::instance().baseURL();
}

void HttpClient::setAccessToken(const QString &accessToken) {
    m_accessToken = accessToken;
}

void HttpClient::get(const QString &path) {
    const QNetworkRequest request = makeRequest(path);
    QNetworkReply *reply = m_manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

void HttpClient::post(const QString &path, const QJsonObject &body) {
    const QNetworkRequest request = makeRequest(path);
    const QJsonDocument doc(body);
    const QByteArray payload = doc.toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_manager.post(request, payload);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

QNetworkRequest HttpClient::makeRequest(const QString &path) {
    const QUrl url(m_baseUrl + path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (const QString token = AuthSession::instance().accessToken(); !token.isEmpty()) {
        req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }

    return req;
}

void HttpClient::handleReply(QNetworkReply *reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        return;
    }

    const QByteArray data = reply->readAll();
    if (data.isEmpty()) {
        emit success(QJsonDocument());
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit error(QStringLiteral("Failed to parse JSON: %1").arg(parseError.errorString()));
        return;
    }

    emit success(doc);
}
