#include "HttpClient.h"

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QNetworkReply>

#include "core/auth/AuthSession.h"
#include "core/auth/TokenRefresher.h"
#include "core/config/ClientConfiguration.h"

HttpClient::HttpClient(QObject *parent) : QObject(parent) {
    m_baseUrl = ClientConfiguration::instance().baseURL();
}

void HttpClient::setAccessToken(const QString &accessToken) {
    m_accessToken = accessToken;
}

void HttpClient::clearAccessToken() {
    m_accessToken.clear();
}

void HttpClient::get(const QString &path) {
    m_pending = {"GET", path, {}};
    const QNetworkRequest request = makeRequest(path);
    QNetworkReply *reply = m_manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

void HttpClient::post(const QString &path, const QJsonObject &body) {
    m_pending = {"POST", path, body};
    const QNetworkRequest request = makeRequest(path);
    const QJsonDocument doc(body);
    const QByteArray payload = doc.toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_manager.post(request, payload);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

void HttpClient::del(const QString &path) {
    m_pending = {"DELETE", path, {}};
    auto req = makeRequest(path);
    auto *reply = m_manager.deleteResource(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

void HttpClient::setRefresher(TokenRefresher *refresher) {
    m_refresher = refresher;

    connect(m_refresher, &TokenRefresher::refreshSuccess, this, [this]() {
        setAccessToken(AuthSession::instance().accessToken());
        if (m_retryCallback) {
            m_retryCallback();
        }
    });

    connect(m_refresher, &TokenRefresher::refreshFailed, this, [this](const QString &err) {
        emit error("Session expired. Please login again.");
    });
}

void HttpClient::onReplyFinished(QNetworkReply *reply) {
    // reply ALWAYS should be deleted
    reply->deleteLater();

    // HTTP status
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // net errors (DNS, timeout etc.)
    if (reply->error() != QNetworkReply::NoError) {
        if (status == 401) {
            emit unauthorized();
            return;
        }

        emit error(reply->errorString());
        return;
    }

    // status 401 without networkError
    if (status == 401) {
        emit unauthorized();
        return;
    }

    const QByteArray data = reply->readAll();
    if (data.isEmpty()) {
        // empty JSON
        emit success(QJsonDocument());
        return;
    }

    QJsonParseError jsonErr{};
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError) {
        emit error(QStringLiteral("Invalid JSON response: %1").arg(jsonErr.errorString()));
        return;
    }

    emit success(doc);
}

QNetworkRequest HttpClient::makeRequest(const QString &path) {
    const QUrl url(m_baseUrl + path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!path.contains("/auth/refresh")) {
        if (const QString token = AuthSession::instance().accessToken(); !token.isEmpty()) {
            req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
        }
    }

    return req;
}

void HttpClient::handleReply(QNetworkReply *reply) {
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 401) {
        reply->deleteLater();

        if (!m_refresher) {
            emit unauthorized();
            return;
        }

        qDebug() << "[HTTP] Access token expired, attempting refresh: " << reply->errorString();

        m_retryCallback = [this, reply]() {
            const auto &p = m_pending;

            if (p.method == "GET") {
                this->get(p.path);
            } else if (p.method == "POST") {
                this->post(p.path, p.body);
            } else if (p.method == "DELETE") {
                this->del(p.path);
            }
        };

        m_refresher->refresh();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        reply->deleteLater();
        return;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();

    if (data.isEmpty()) {
        emit success(QJsonDocument());
        return;
    }

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        emit error("Invalid JSON.");
        return;
    }

    emit success(doc);
}
