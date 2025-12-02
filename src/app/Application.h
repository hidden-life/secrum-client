#ifndef APP_APPLICATION_H
#define APP_APPLICATION_H

#include "core/device/DeviceService.h"
#include "core/network/ConnectivityService.h"

class AuthController;
class LoginWindow;
class AuthService;
class MainWindow;
class ConnectivityService;
class DeviceService;

class Application final : public QObject {
    Q_OBJECT
public:
    Application(QObject *parent = nullptr);
    ~Application();

    int start(int argc, char **argv);

private slots:
    void onLoginSuccess();

private:
    AuthService *m_authService;
    LoginWindow *m_loginWindow;
    MainWindow *m_mainWindow;
    AuthController *m_authController;
    DeviceService *m_deviceService;

    ConnectivityService *m_connectivity = nullptr;

    void showLoginWindow();
    void showMainWindow();
};

#endif //APP_APPLICATION_H
