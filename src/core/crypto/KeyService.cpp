#include "KeyService.h"

#include "core/auth/AuthSession.h"

KeyService &KeyService::instance() {
    static KeyService instance;
    return instance;
}

void KeyService::fetchBundle(const QString &userId) {
    if (userId.isEmpty()) {
        emit bundleFailed(userId, "Empty user ID.");
        return;
    }

    const QString path = QString("/keys/bundle/%1").arg(userId);
    m_httpClient->get(path);
}

KeyService::KeyService(QObject *parent) : QObject(parent), m_httpClient(new HttpClient(this)) {
    m_httpClient->setAccessToken(AuthSession::instance().accessToken());

    connect(m_httpClient, &HttpClient::success, this, [this](const QJsonDocument &doc) {
        if (!doc.isObject()) {
            emit bundleFailed("", "Invalid bundle response.");
            return;
        }

        const QJsonObject obj = doc.object();
        const QString userId = obj.value("user_id").toString();

        emit bundleLoaded(userId, obj);
    });

    connect(m_httpClient, &HttpClient::error, this, [this](const QString &err) {
        emit bundleFailed("", err);
    });
}
