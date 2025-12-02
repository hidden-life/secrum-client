#include "MainWindow.h"
#include "./ui_mainwindow.h"

#include <QMenu>
#include <QMenuBar>
#include <QAction>

#include "DeviceManagerDialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);
    setWindowTitle(tr("Secrum"));

    m_deviceService = new DeviceService(this);

    setupMenus();
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::setupMenus() {
    QMenu *accountMenu = menuBar()->addMenu(tr("Account"));
    QAction *devicesAction = new QAction(tr("Devices..."), this);
    accountMenu->addAction(devicesAction);

    connect(devicesAction, &QAction::triggered, this, &MainWindow::openDevicesDialog);
}

void MainWindow::updateConnectionStatus(int state) {
    switch (state) {
        case ConnectivityService::Connecting:
            m_ui->connectionBanner->setVisible(true);
            m_ui->connectionLabel->setText("Connecting...");
            m_ui->connectionBanner->setStyleSheet("background-color: #f0ad4e; color: black;");
            break;

        case ConnectivityService::Online:
            m_ui->connectionBanner->setVisible(false);
            break;
        case ConnectivityService::Offline:
            m_ui->connectionBanner->setVisible(true);
            m_ui->connectionLabel->setText("Server unavailable.");
            m_ui->connectionBanner->setStyleSheet("background-color: #d9534f; color: white;");
            break;
    }
}

void MainWindow::setConnectivity(ConnectivityService *svc) {
    connect(svc, &ConnectivityService::stateChanged, this, &MainWindow::updateConnectionStatus);
}

void MainWindow::openDevicesDialog() {
    if (!m_deviceService) return;

    DeviceManagerDialog devicesDialog(m_deviceService, this);
    devicesDialog.exec();
}
