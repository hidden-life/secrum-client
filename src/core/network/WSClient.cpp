#include "WSClient.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QUrlQuery>

#include "core/auth/AuthSession.h"
#include "core/config/ClientConfiguration.h"

WSClient::WSClient(QObject *parent) : QObject(parent) {
    connect(&m_ws, &QWebSocket::connected, this, &WSClient::connected);
    connect(&m_ws, &QWebSocket::disconnected, this, &WSClient::disconnected);

    connect(&m_ws, &QWebSocket::textMessageReceived, this, [this](const QString &message) {
        QJsonParseError err{};
        const auto doc = QJsonDocument::fromJson(message.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            return;
        }

        const auto obj = doc.object();
        const QString type = obj.value("type").toString();
        const QJsonObject data = obj.value("data").toObject();

        if (!type.isEmpty()) {
            emit eventReceived(type, data);
        }
    });

    connect(&m_ws, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        emit errorOccurred(m_ws.errorString());
    });
}

void WSClient::connectToHost(QUrl url) {
    const QString accessToken = AuthSession::instance().accessToken();
    if (accessToken.isEmpty()) {
        emit errorOccurred("No access token for WS.");
        return;
    }

    QUrlQuery q(url);
    q.addQueryItem("access_token", accessToken);
    url.setQuery(q);

    m_ws.open(url);
}

void WSClient::disconnectFromHost() {
    m_ws.close();
}

void WSClient::send(const QString &type, const QJsonObject &json) {
    QJsonObject env;
    env["type"] = type;
    if (!json.isEmpty()) {
        env["data"] = json;
    }

    m_ws.sendTextMessage(QJsonDocument(env).toJson(QJsonDocument::Compact));
}
