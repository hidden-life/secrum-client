#include "MainWindow.h"
#include "./ui_mainwindow.h"

#include <QMenu>
#include <QMenuBar>
#include <QAction>

#include "DeviceManagerDialog.h"

MainWindow::MainWindow(const QString &accessToken, const QString &refreshToken ,QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), m_accessToken(accessToken), m_refreshToken(refreshToken) {
    ui->setupUi(this);
    setWindowTitle(tr("Secrum"));

    m_deviceService = new DeviceService(this);
    m_deviceService->setAccessToken(accessToken);

    setupMenus();
}

MainWindow::~MainWindow() {
    delete ui;
}

QString MainWindow::accessToken() const {
    return m_accessToken;
}

void MainWindow::setupMenus() {
    QMenu *accountMenu = menuBar()->addMenu(tr("Account"));
    QAction *devicesAction = new QAction(tr("Devices..."), this);
    accountMenu->addAction(devicesAction);

    connect(devicesAction, &QAction::triggered, this, &MainWindow::openDevicesDialog);
}

void MainWindow::openDevicesDialog() {
    if (!m_deviceService) return;

    DeviceManagerDialog devicesDialog(m_deviceService, this);
    devicesDialog.exec();
}
