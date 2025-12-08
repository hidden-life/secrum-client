#include "TokenRefresher.h"
#include "AuthSession.h"
#include "core/network/HttpClient.h"

#include <QJsonObject>

TokenRefresher::TokenRefresher(HttpClient *client, QObject *parent) : QObject(parent), m_httpClient(client) {}

void TokenRefresher::refresh() {
    if (m_refreshInProgress) return;
    m_refreshInProgress = true;

    QJsonObject body;
    body["refresh_token"] = AuthSession::instance().refreshToken();

    auto conn = connect(m_httpClient, &HttpClient::success, this, [this](const QJsonDocument &doc) {
        m_refreshInProgress = false;
        disconnect(m_httpClient, nullptr, this, nullptr);

        auto json = doc.object();
        AuthSession::instance().setAccessToken(json["access_token"].toString());
        AuthSession::instance().setRefreshToken(json["refresh_token"].toString());

        emit refreshSuccess();
    });

    connect(m_httpClient, &HttpClient::error, this, [this](const QString &err) {
        m_refreshInProgress = false;
        emit refreshFailed(err);
    });

    qDebug() << "[REFRESH TOKEN] token = " << AuthSession::instance().refreshToken();
    qDebug() << "[REFRESH REQUEST sending = " << body;
    m_httpClient->post("/auth/refresh", body);
}