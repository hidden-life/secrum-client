#include "LoginWindow.h"
#include "./ui_loginwindow.h"
#include "app/AuthController.h"

#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent), ui(new Ui::LoginWindow), m_authController(new AuthController(this)) {
    ui->setupUi(this);

    connect(ui->sendCodeButton, &QPushButton::clicked, this, [this] {
        ui->statusLabel->setText("Sending code...");
        m_authController->service()->loginBegin(ui->phoneLineEdit->text());
    });
    connect(ui->loginButton, &QPushButton::clicked, this, [this] {
        ui->statusLabel->setText("Verifying...");
        m_authController->service()->verifyCode(ui->codeLineEdit->text());
    });

    // server response
    connect(m_authController->service(), &AuthService::loginCodeSent, this, [this] {
        ui->statusLabel->setText("Code sent");
        ui->codeLineEdit->setEnabled(true);
        ui->loginButton->setEnabled(true);
    });

    connect(m_authController->service(), &AuthService::loginError, this, [this](const QString &err) {
        ui->statusLabel->setText("Error: " + err);
    });

    connect(m_authController->service(), &AuthService::loginSuccess, this, [this](const QString) {
        ui->statusLabel->setText("Success!");
        emit loginCompleted();
    });
}

LoginWindow::~LoginWindow() {
    delete ui;
}
