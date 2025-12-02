#include "Application.h"
#include "AuthController.h"
#include "ui/LoginWindow.h"

#include <QMainWindow>
#include <QApplication>

#include "core/auth/AuthSession.h"
#include "ui/MainWindow.h"
#include "core/config/ClientConfiguration.h"
#include "core/network/ConnectivityService.h"

Application::Application(QObject *parent) : QObject(parent) {
}

Application::~Application() = default;

int Application::start(int argc, char **argv) {
    QApplication app(argc, argv);
    ClientConfiguration::instance().load();

    // connectivity check
    m_connectivity = new ConnectivityService(this);
    m_connectivity->start();
    connect(m_connectivity, &ConnectivityService::serverOnline, this, []() {
        qDebug() << "[CONNECTIVITY] Server online.";
    });

    connect(m_connectivity, &ConnectivityService::serverOffline, this, []() {
        qDebug() << "[CONNECTIVITY] Server offline.";
    });

    // config/HTTP/services
    m_authService = new AuthService(&app);
    m_authController = new AuthController(m_authService, this);

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
        m_loginWindow->deleteLater();
        m_loginWindow = nullptr;
    }

    showMainWindow();
}

void Application::showLoginWindow() {
    if (!m_loginWindow) {
        m_loginWindow = new LoginWindow(m_authController);
        if (m_connectivity) {
            m_loginWindow->setConnectivity(m_connectivity);
        }

        connect(m_loginWindow, &LoginWindow::loginCompleted, this, &Application::onLoginSuccess);
        connect(m_loginWindow, &QObject::destroyed, this, [this]() {
            m_loginWindow = nullptr;
        });
    }

    m_loginWindow->show();
    m_loginWindow->raise();
    m_loginWindow->activateWindow();
}

void Application::showMainWindow() {
    if (!m_mainWindow) {
        m_mainWindow = new MainWindow();
        if (m_connectivity) {
            m_mainWindow->setConnectivity(m_connectivity);
        }

        connect(m_mainWindow, &QObject::destroyed, this, [this]() {
            m_mainWindow = nullptr;
        });
    }

    m_mainWindow->show();
    m_mainWindow->raise();
    m_mainWindow->activateWindow();
}
