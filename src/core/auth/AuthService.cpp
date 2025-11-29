#include "AuthService.h"

AuthService::AuthService(QObject *parent) : QObject(parent) {
    m_httpClient = new HttpClient(this);

    connect(m_httpClient, &HttpClient::success, this, [this](QJsonObject json) {
        if (json.contains("request_id")) {
            m_requestId = json["request_id"].toString(); // save request_id
            emit loginCodeSent();
            return;
        }

        if (json.contains("access_token")) {
            emit loginSuccess(json["access_token"].toString());
            return;
        }
    });

    connect(m_httpClient, &HttpClient::error, this, [this](const QString &err) {
        emit loginError(err);
    });
}

void AuthService::loginBegin(const QString &phone) {
    QJsonObject body;
    body["phone"] = phone;

    m_httpClient->post("/auth/begin", body);
}

void AuthService::verifyCode(const QString &code) {
    QJsonObject body;
    body["request_id"] = m_requestId;
    body["code"] = code;

    m_httpClient->post("/auth/verify", body);
}
