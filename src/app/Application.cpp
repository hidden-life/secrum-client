#include "Application.h"
#include "AuthController.h"
#include "ui/LoginWindow.h"

#include <QMainWindow>

Application::Application() {
    m_authController = new AuthController();
    m_loginWindow = new LoginWindow();

    connect(m_loginWindow, &LoginWindow::loginCompleted, this, &Application::onLoginSuccess);
}


void Application::start() {
    m_loginWindow->show();
}

void Application::onLoginSuccess() {
    m_loginWindow->close();
}
