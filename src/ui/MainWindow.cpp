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

void MainWindow::openDevicesDialog() {
    if (!m_deviceService) return;

    DeviceManagerDialog devicesDialog(m_deviceService, this);
    devicesDialog.exec();
}
