#ifndef APP_AUTH_CONTROLLER_H
#define APP_AUTH_CONTROLLER_H

#include <QObject>
#include <QString>

#include "core/auth/AuthService.h"

class AuthController final : public QObject {
    Q_OBJECT
public:
    explicit AuthController(QObject *parent = nullptr);
    AuthService *service();

private:
    AuthService *m_authService;
};

#endif //APP_AUTH_CONTROLLER_H
