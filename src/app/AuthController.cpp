#include "AuthController.h"

AuthController::AuthController(QObject *parent) : QObject(parent) {
    m_authService = new AuthService(this);
}

AuthService * AuthController::service() {
    return m_authService;
}
