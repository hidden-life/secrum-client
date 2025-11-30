#include "Application.h"
#include "AuthController.h"
#include "ui/LoginWindow.h"

#include <QMainWindow>
#include <QApplication>

#include "core/auth/AuthSession.h"
#include "ui/MainWindow.h"

Application::Application(QObject *parent) : QObject(parent) {
}

Application::~Application() = default;

int Application::start(int argc, char **argv) {
    QApplication app(argc, argv);

    // config/HTTP/services
    m_authService = std::make_unique<AuthService>(&app);
    m_authController = std::make_unique<AuthController>(m_authService.get(), this);

    // check for active session
    const auto &session = AuthSession::instance();
    if (!session.accessToken().isEmpty() && !session.userId().isEmpty()) {
        showMainWindow();
    } else {
        showLoginWindow();
    }

    return app.exec();
}

void Application::onLoginSuccess() {
    if (m_loginWindow) {
        m_loginWindow->close();
        m_loginWindow.reset();
    }

    showMainWindow();
}

void Application::showLoginWindow() {
    m_loginWindow = std::make_unique<LoginWindow>(m_authController.get());
    connect(m_loginWindow.get(), &LoginWindow::loginCompleted, this, &Application::onLoginSuccess);
    m_loginWindow->show();
}

void Application::showMainWindow() {
    m_mainWindow = std::make_unique<MainWindow>();
    m_mainWindow->show();
}
