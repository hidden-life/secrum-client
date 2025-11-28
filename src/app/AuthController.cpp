#include "AuthController.h"

bool AuthController::sendCode(const QString &phone) {
    return !phone.trimmed().isEmpty();
}

bool AuthController::login(const QString &phone, const QString &code) {
    return phone == "+380991001010" && code == "1234";
}
