#ifndef APP_APPLICATION_H
#define APP_APPLICATION_H


class AuthController;
class LoginWindow;

class Application {
public:
    Application();

    void start();

private:
    AuthController *m_authController;
    LoginWindow *m_loginWindow;
};

#endif //APP_APPLICATION_H
