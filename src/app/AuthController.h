#ifndef APP_AUTH_CONTROLLER_H
#define APP_AUTH_CONTROLLER_H

#include <QObject>
#include <QString>

#include "core/auth/AuthService.h"

class AuthService;

class AuthController final : public QObject {
    Q_OBJECT
public:
    explicit AuthController(AuthService *service, QObject *parent = nullptr);
    // login start
    void login(const QString &phone);
    // code verification
    void verify(const QString &requestId, const QString &phone, const QString &code);

signals:
    void codeSent(const QString &requestId, const QString &phone);
    void loginSuccess();
    void loginFailed(const QString &err);

private:
    AuthService *m_authService;
};

#endif //APP_AUTH_CONTROLLER_H
