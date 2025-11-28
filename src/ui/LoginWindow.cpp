#include "LoginWindow.h"
#include "./ui_loginwindow.h"
#include "app/AuthController.h"

#include <QMessageBox>

LoginWindow::LoginWindow(AuthController *controller, QWidget *parent) :
    QWidget(parent), ui(new Ui::LoginWindow), m_authController(controller) {
    ui->setupUi(this);

    connect(ui->sendCodeButton, &QPushButton::clicked, this, &LoginWindow::onSendCodeButtonClicked);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);
}

LoginWindow::~LoginWindow() {
    delete ui;
}

void LoginWindow::onSendCodeButtonClicked() {
    const auto phone = ui->phoneLineEdit->text();
    if (!m_authController->sendCode(phone)) {
        QMessageBox::warning(this, "Error", "Invalid phone number");
        return;
    }

    QMessageBox::information(this, "Code sent", "Fake SMS code sent :)");
}

void LoginWindow::onLoginButtonClicked() {
    const auto phone = ui->phoneLineEdit->text();
    const auto code = ui->codeLineEdit->text();

    if (!m_authController->login(phone, code)) {
        QMessageBox::critical(this, "Error", "Login failed");
        return;
    }

    QMessageBox::information(this, "Success", "Logged in (stub)");
    close();
}
