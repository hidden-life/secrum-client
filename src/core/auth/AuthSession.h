#ifndef CORE_AUTH_AUTH_SESSION_H
#define CORE_AUTH_AUTH_SESSION_H

#include <QString>

class AuthSession {
public:
    static AuthSession &instance();

    // user
    QString userId() const;
    void setUserId(const QString &id);

    // device
    QString deviceId() const;
    void setDeviceId(const QString &id);

    // tokens
    QString accessToken() const;
    void setAccessToken(const QString &token);

    QString refreshToken() const;
    void setRefreshToken(const QString &token);

    // clear
    void clear();

private:
    AuthSession() = default;
};

#endif //CORE_AUTH_AUTH_SESSION_H
