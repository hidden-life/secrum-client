#include "AuthService.h"

#include "AuthSession.h"
#include "core/device/DeviceService.h"
#include "core/storage/SQLiteStorage.h"

AuthService::AuthService(QObject *parent) : QObject(parent), m_httpClient(new HttpClient(this)) {
    // success HTTP-response
    connect(m_httpClient, &HttpClient::success, this, [this](const QJsonDocument &doc) {
        QJsonObject json = doc.object();
        // if exists 'access_token' - it's '/auth/verify' response
        if (json.contains("access_token")) {
            const QString accessToken = json.value("access_token").toString();
            const QString refreshToken = json.value("refresh_token").toString();
            const QString deviceId = json.value("device_id").toString();
            const QString userId = json.value("user_id").toString();

            auto &session = AuthSession::instance();
            session.setAccessToken(accessToken);
            session.setRefreshToken(refreshToken);
            session.setUserId(userId);

            // device_id from server is more important than local
            if (!deviceId.isEmpty()) {
                session.setDeviceId(deviceId);
            }

            emit loginSuccess();
            return;
        }

        // ...it's '/auth/begin'
        if (json.contains("request_id")) {
            const QString requestId = json.value("request_id").toString();
            emit codeSent(requestId, m_lastPhone);
            return;
        }

        emit loginError("Unexpected auth response.");
    });

    // HTTP error
    connect(m_httpClient, &HttpClient::error, this, [this](const QString &err) {
        emit loginError(err);
    });
}

void AuthService::loginBegin(const QString &phone) {
    m_lastPhone = phone;
    QJsonObject body;
    body["phone"] = phone;

    m_httpClient->post("/auth/begin", body);
}

void AuthService::verifyCode(const QString &requestId, const QString &phone, const QString &code) {
    QJsonObject body;
    body["request_id"] = requestId;
    body["code"] = code;
    body["phone"] = phone;

    // device identity
    body["device_id"] = DeviceService::deviceId();
    body["device_name"] = QSysInfo::machineHostName();
    body["platform"] = QSysInfo::prettyProductName();

    m_httpClient->post("/auth/verify", body);
}
