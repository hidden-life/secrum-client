#include "AuthSession.h"
#include "core/storage/SQLiteStorage.h"

AuthSession &AuthSession::instance() {
    static AuthSession instance;
    return instance;
}

QString AuthSession::userId() const {
    return SQLiteStorage::instance().get("user_id");
}

void AuthSession::setUserId(const QString &id) {
    SQLiteStorage::instance().set("user_id", id);
}

QString AuthSession::deviceId() const {
    return SQLiteStorage::instance().get("device_id");
}

void AuthSession::setDeviceId(const QString &id) {
    SQLiteStorage::instance().set("device_id", id);
}

QString AuthSession::accessToken() const {
    return SQLiteStorage::instance().get("access_token");
}

void AuthSession::setAccessToken(const QString &token) {
    SQLiteStorage::instance().set("access_token", token);
}

QString AuthSession::refreshToken() const {
    return SQLiteStorage::instance().get("refresh_token");
}

void AuthSession::setRefreshToken(const QString &token) {
    SQLiteStorage::instance().set("refresh_token", token);
}

void AuthSession::clear() {
    auto &s = SQLiteStorage::instance();

    s.remove("user_id");
    s.remove("device_id");
    s.remove("access_token");
    s.remove("refresh_token");
}

bool AuthSession::isAuthenticated() const {
    return !accessToken().isEmpty() && !userId().isEmpty();
}
