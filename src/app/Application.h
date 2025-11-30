#ifndef APP_APPLICATION_H
#define APP_APPLICATION_H

#include <QObject>
#include <memory>

class AuthController;
class LoginWindow;
class AuthService;
class MainWindow;

class Application final : public QObject {
    Q_OBJECT
public:
    Application(QObject *parent = nullptr);
    ~Application();

    int start(int argc, char **argv);

private slots:
    void onLoginSuccess();

private:
    std::unique_ptr<AuthService> m_authService;
    std::unique_ptr<LoginWindow> m_loginWindow;
    std::unique_ptr<MainWindow> m_mainWindow;
    std::unique_ptr<AuthController> m_authController;

    void showLoginWindow();
    void showMainWindow();
};

#endif //APP_APPLICATION_H
