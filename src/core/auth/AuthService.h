#ifndef CORE_AUTH_AUTH_SERVICE_H
#define CORE_AUTH_AUTH_SERVICE_H

#include <QObject>

#include "core/network/HttpClient.h"

class HttpClient;

class AuthService final : public QObject {
    Q_OBJECT
public:
    explicit AuthService(QObject *parent = nullptr);
    // 1. code request
    void loginBegin(const QString &phone);
    // 2. code confirmation
    void verifyCode(const QString &requestId, const QString &phone, const QString &code);
    // 3. refresh session
    void refreshSession();

signals:
    // code sent. need a field to enter it
    void codeSent(const QString &requestId, const QString &phone);
    // login success (tokens are saved inside AuthSession)
    void loginSuccess();
    // any login error
    void loginError(const QString &err);

private:
    HttpClient *m_httpClient;
    QString m_lastPhone;
};

#endif //CORE_AUTH_AUTH_SERVICE_H
