#ifndef APP_APPLICATION_H
#define APP_APPLICATION_H

#include <QObject>

class AuthController;
class LoginWindow;

class Application : public QObject {
    Q_OBJECT
public:
    Application();

    void start();

private slots:
    void onLoginSuccess();

private:
    AuthController *m_authController;
    LoginWindow *m_loginWindow;
};

#endif //APP_APPLICATION_H
