#include "AuthController.h"

#include "ui/LoginWindow.h"

AuthController::AuthController(AuthService *service, QObject *parent) : QObject(parent), m_authService(service) {
    connect(m_authService, &AuthService::codeSent, this, &AuthController::codeSent);
    connect(m_authService, &AuthService::loginSuccess, this, &AuthController::loginSuccess);
    connect(m_authService, &AuthService::loginError, this, &AuthController::loginFailed);
}

void AuthController::login(const QString &phone) {
    m_authService->loginBegin(phone);
}

void AuthController::verify(const QString &requestId, const QString &phone, const QString &code) {
    m_authService->verifyCode(requestId, phone, code);
}
