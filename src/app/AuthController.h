#ifndef APP_AUTH_CONTROLLER_H
#define APP_AUTH_CONTROLLER_H

#include <QString>

class AuthController {
public:
    bool sendCode(const QString &phone);
    bool login(const QString &phone, const QString &code);
};

#endif //APP_AUTH_CONTROLLER_H
