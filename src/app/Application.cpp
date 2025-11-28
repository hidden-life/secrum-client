#include "Application.h"
#include "AuthController.h"
#include "ui/LoginWindow.h"

#include <QMainWindow>

Application::Application() {
    m_authController = new AuthController();
    m_loginWindow = new LoginWindow(m_authController);
}


void Application::start() {
    m_loginWindow->show();
}