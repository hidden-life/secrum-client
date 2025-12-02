#include "MainWindow.h"
#include "./ui_mainwindow.h"

#include <QMenu>
#include <QMenuBar>
#include <QAction>

#include "DeviceManagerDialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);

    m_ui->pagesStack->setCurrentWidget(m_ui->pageChats);

    connect(m_ui->chatsButton, &QPushButton::clicked, this, [this]() {
        m_ui->pagesStack->setCurrentWidget(m_ui->pageChats);
        m_ui->currentSectionLabel->setText("Chats");
    });

    connect(m_ui->usersButton, &QPushButton::clicked, this, [this]() {
        m_ui->pagesStack->setCurrentWidget(m_ui->pageUsers);
        m_ui->currentSectionLabel->setText("Users");
    });

    connect(m_ui->devicesButton, &QPushButton::clicked, this, [this]() {
        m_ui->pagesStack->setCurrentWidget(m_ui->pageDevices);
        m_ui->currentSectionLabel->setText("Devices");
    });

    connect(m_ui->settingsButton, &QPushButton::clicked, this, [this]() {
        m_ui->pagesStack->setCurrentWidget(m_ui->pageSettings);
        m_ui->currentSectionLabel->setText("Settings");
    });
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::setConnectivity(ConnectivityService *svc) {
    if (!svc) return;

    connect(svc, &ConnectivityService::stateChanged, this, [this](const int state) {
        if (state == ConnectivityService::State::Online) {
            m_ui->statusLabel->setText("Online");
            m_ui->statusLabel->setStyleSheet("color: #4caf50;");
        } else {
            m_ui->statusLabel->setText("Offline");
            m_ui->statusLabel->setStyleSheet("color: f44336;");
        }
    });
}
