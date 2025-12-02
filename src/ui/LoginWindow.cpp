#include "LoginWindow.h"
#include "./ui_loginwindow.h"
#include "app/AuthController.h"
#include "MainWindow.h"
#include "core/config/ClientConfiguration.h"

LoginWindow::LoginWindow(AuthController *authController, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::LoginWindow),
    m_authController(authController) {
    m_ui->setupUi(this);

    // initial state
    setState(State::IDLE);

    connect(m_ui->sendCodeButton, &QPushButton::clicked, this, &LoginWindow::onSendCodeClicked);
    connect(m_ui->loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_authController, &AuthController::codeSent, this, &LoginWindow::onCodeSent);
    connect(m_authController, &AuthController::loginSuccess, this, &LoginWindow::onLoginSuccess);
    connect(m_authController, &AuthController::loginFailed, this, &LoginWindow::onLoginFailed);
}

LoginWindow::~LoginWindow() {
    delete m_ui;
}

void LoginWindow::onSendCodeClicked() {
    m_phone = m_ui->phoneLineEdit->text().trimmed();
    if (m_phone.isEmpty()) {
        return;
    }

    setState(State::SendingCode);

    m_authController->login(m_phone);
}

void LoginWindow::onLoginClicked() {
    if (m_requestId.isEmpty() || m_phone.isEmpty()) {
        return;
    }

    const QString code = m_ui->codeLineEdit->text().trimmed();
    if (code.isEmpty()) {
        return;
    }

    setState(State::Verifying);

    m_authController->verify(m_requestId, m_phone, code);
}

void LoginWindow::onCodeSent(const QString &requestId, const QString &phone) {
    m_requestId = requestId;
    m_phone = phone;

    setState(State::CodeSent);

    m_ui->codeLineEdit->setFocus();
}

void LoginWindow::onLoginSuccess() {
    emit loginCompleted();
    close();
}

void LoginWindow::onLoginFailed(const QString &err) {

    if (m_state == State::SendingCode) {
        setState(State::IDLE);
    } else if (m_state == State::Verifying) {
        setState(State::CodeSent);
    }
}

void LoginWindow::setConnectivity(ConnectivityService *svc) {
    if (!svc) {
        return;
    }
    m_connectivity = svc;

    connect(svc, &ConnectivityService::stateChanged, this, [this](int state) {
        bool isOnline = (state == ConnectivityService::State::Online);

        m_ui->sendCodeButton->setEnabled(isOnline);
        m_ui->loginButton->setEnabled(isOnline);

        if (!isOnline) {
            m_ui->errorLabel->setText("Waiting for connection...");
            m_ui->errorLabel->show();
        } else {
            m_ui->errorLabel->clear();
            m_ui->errorLabel->hide();
        }
    });
}

void LoginWindow::setState(State state) {
    m_state = state;

    bool canEditPhone = (state == State::IDLE || state == State::CodeSent);
    bool canEditCode = (state == State::IDLE || state == State::Verifying);
    bool canSendCode = (state == State::IDLE || state == State::CodeSent);
    bool canLogin = (state == State::CodeSent);

    // block buttons during queries
    if (state == State::SendingCode || state == State::Verifying) {
        canSendCode = false;
        canLogin = false;
    }

    m_ui->phoneLineEdit->setEnabled(canEditPhone);
    m_ui->codeLineEdit->setEnabled(canEditCode);
    m_ui->loginButton->setEnabled(canLogin);
    m_ui->sendCodeButton->setEnabled(canSendCode);
}
