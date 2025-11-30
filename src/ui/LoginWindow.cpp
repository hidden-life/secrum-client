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
    setStatusMessage(QString(), false);

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
        setStatusMessage("Enter phone number", true);
        return;
    }

    setStatusMessage("Sending code...", false);
    setState(State::SendingCode);

    m_authController->login(m_phone);
}

void LoginWindow::onLoginClicked() {
    if (m_requestId.isEmpty() || m_phone.isEmpty()) {
        setStatusMessage("First, request a code", true);
        return;
    }

    const QString code = m_ui->codeLineEdit->text().trimmed();
    if (code.isEmpty()) {
        setStatusMessage("Enter a verification code", true);
        return;
    }

    setStatusMessage("Verifying a code...", false);
    setState(State::Verifying);

    m_authController->verify(m_requestId, m_phone, code);
}

void LoginWindow::onCodeSent(const QString &requestId, const QString &phone) {
    m_requestId = requestId;
    m_phone = phone;

    setStatusMessage("Code sent. Check your SMS.", false);
    setState(State::CodeSent);

    m_ui->codeLineEdit->setFocus();
}

void LoginWindow::onLoginSuccess() {
    setStatusMessage("Login successful", false);
    emit loginCompleted();
    close();
}

void LoginWindow::onLoginFailed(const QString &err) {
    setStatusMessage(err, true);

    if (m_state == State::SendingCode) {
        setState(State::IDLE);
    } else if (m_state == State::Verifying) {
        setState(State::CodeSent);
    }
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

void LoginWindow::setStatusMessage(const QString &message, bool isError) {
    m_ui->statusLabel->setText(message);
    if (isError) {
        m_ui->statusLabel->setStyleSheet("color: red;");
    } else {
        m_ui->statusLabel->setStyleSheet("color: green;");
    }
}
