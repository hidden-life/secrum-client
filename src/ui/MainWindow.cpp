#include "MainWindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(const QString &accessToken, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), m_accessToken(accessToken) {
    ui->setupUi(this);
    ui->statusLabel->setText("Connected");
}

MainWindow::~MainWindow() {
    delete ui;
}

QString MainWindow::accessToken() const {
    return m_accessToken;
}
