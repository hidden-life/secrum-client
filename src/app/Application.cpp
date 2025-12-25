#include "Application.h"
#include "AuthController.h"
#include "ui/LoginWindow.h"

#include <QMainWindow>
#include <QApplication>
#include <QPointer>

#include "core/auth/AuthSession.h"
#include "core/auth/TokenRefresher.h"
#include "ui/MainWindow.h"
#include "core/config/ClientConfiguration.h"
#include "core/crypto/CryptoService.h"
#include "core/network/ConnectivityService.h"
#include "core/model/UserSearchResult.h"

Application::Application(QObject *parent) : QObject(parent) {
}

Application::~Application() = default;

int Application::start(int argc, char **argv) {
    QApplication app(argc, argv);

    ClientConfiguration::instance().load();
    // create WS client
    m_wsClient = new WSClient(nullptr);

    qRegisterMetaType<UserSearchResult>("UserSearchResult");
    qRegisterMetaType<QVector<UserSearchResult>>("QVector<UserSearchResult>");

    if (!CryptoService::instance().init()) {
        qWarning() << "[FATAL] Crypto initialization failed.";
        return EXIT_FAILURE;
    }

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
    m_deviceService = new DeviceService(this);

    auto refresher = new TokenRefresher(m_authService->httpClient(), this);
    m_authService->httpClient()->setRefresher(refresher);

    // check for active session
    const auto &session = AuthSession::instance();
    connect(m_authService, &AuthService::loginSuccess, this, [this]() {
        auto &sess = AuthSession::instance();
        m_deviceService->setAccessToken(sess.accessToken());
        if (!m_mainWindow) {
            startRealtime();
            showMainWindow();
        }
    });

    connect(m_authService, &AuthService::loginError, this, [this](const QString &err) {
        qWarning() << "[AUTH] startup auth error: " << err;
        if (!m_loginWindow) {
            showLoginWindow();
        }
    });

    if (!session.refreshToken().isEmpty()) {
        m_authService->refreshSession();
    } else {
        showLoginWindow();
    }

    m_keyService = new KeyService(this);

    connect(m_keyService->httpClient(), &HttpClient::success, this, [this](const QJsonDocument &doc) {
        emit m_keyService->bundleLoaded(doc);
    });

    connect(m_keyService->httpClient(), &HttpClient::error, this, [this](const QString &err) {
        emit m_keyService->bundleFailed(err);
    });

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
        if (!m_wsClient) {
            return;
        }

        m_mainWindow = new MainWindow(m_wsClient);

        connect(m_mainWindow, &MainWindow::logoutRequested, this, &Application::logout);

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

void Application::startRealtime() {
    if (!m_wsClient || m_isRealtimeStarted) {
        return;
    }

    m_isRealtimeStarted = true;

    connect(m_wsClient, &WSClient::connected, this, []() {
        qDebug() << "[WS] connected.";
    });

    connect(m_wsClient, &WSClient::disconnected, this, []() {
        qDebug() << "[WS] disconnected.";
    });

    connect(m_wsClient, &WSClient::errorOccurred, this, [](const QString &err) {
        qWarning() << "[WS] error: " << err;
    });

    connect(m_wsClient, &WSClient::eventReceived, this, [](const QString &type, const QJsonObject &json) {
        qDebug() << "[WS] eventReceived: " << type << "=" << json;
    });

    QUrl wsUrl = ClientConfiguration::instance().wsUrl();
    qDebug() << "[WS] base url=" << ClientConfiguration::instance().wsUrl();
    qDebug() << "[WS] access token length=" << AuthSession::instance().accessToken().size();
    m_wsClient->connectToHost(wsUrl);
}

void Application::stopRealtime() {
    if (m_wsClient) {
        m_wsClient->disconnectFromHost();
    }
}

void Application::logout() {
    stopRealtime();

    AuthSession::instance().clear();

    if (m_mainWindow) {
        m_mainWindow->close();
        m_mainWindow->deleteLater();
        m_mainWindow = nullptr;
    }

    showLoginWindow();
}
