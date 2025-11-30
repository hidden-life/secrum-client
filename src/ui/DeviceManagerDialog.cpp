#include "DeviceManagerDialog.h"
#include "./ui_devicemanagerdialog.h"
#include "core/device/DeviceService.h"

#include <QPushButton>
#include <QMessageBox>

DeviceManagerDialog::DeviceManagerDialog(DeviceService *service, QWidget *parent) :
    QDialog(parent), ui(new Ui::DeviceManagerDialog), m_deviceService(service) {
    ui->setupUi(this);

    setWindowTitle("Secrum - Devices");

    // service subscription
    connect(m_deviceService, &DeviceService::devicesLoaded, this, &DeviceManagerDialog::onDevicesLoaded);
    connect(m_deviceService, &DeviceService::requestFailed, this, &DeviceManagerDialog::onRequestFailed);
    connect(m_deviceService, &DeviceService::deviceActionCompleted, this, &DeviceManagerDialog::onDeviceActionCompleted);

    // buttons subscription
    connect(ui->refreshButton, &QPushButton::clicked, this, &DeviceManagerDialog::onRefreshClicked);
    connect(ui->deactivateButton, &QPushButton::clicked, this, &DeviceManagerDialog::onDeactivateClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &DeviceManagerDialog::onDeleteClicked);

    // table
    ui->devicesTable->setColumnCount(5);
    QStringList headers;
    headers << tr("Name") << tr("Platform") << tr("Active") << tr("Current") << tr("Last active");
    ui->devicesTable->setHorizontalHeaderLabels(headers);
    ui->devicesTable->horizontalHeader()->setStretchLastSection(true);
    ui->devicesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->devicesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->devicesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    reloadDevices();
}

DeviceManagerDialog::~DeviceManagerDialog() {
    delete ui;
}

void DeviceManagerDialog::onDevicesLoaded(const QVector<Device> &devices) {
    m_devices = devices;

    ui->devicesTable->setRowCount(devices.size());

    for (int i = 0; i < m_devices.size(); ++i) {
        const auto &d = devices[i];

        auto *nameItem = new QTableWidgetItem(d.name);
        auto *platformItem = new QTableWidgetItem(d.platform);
        auto *activeItem = new QTableWidgetItem(d.isActive ? tr("Yes") : tr("No"));
        auto *currentItem = new QTableWidgetItem(d.isCurrent ? tr("Yes") : tr("No"));
        auto *lastSeen = new QTableWidgetItem(d.lastSeen);

        ui->devicesTable->setItem(i, 0, nameItem);
        ui->devicesTable->setItem(i, 1, platformItem);
        ui->devicesTable->setItem(i, 2, activeItem);
        ui->devicesTable->setItem(i, 3, currentItem);
        ui->devicesTable->setItem(i, 4, lastSeen);
    }
}

void DeviceManagerDialog::onRequestFailed(const QString &msg) {
    QMessageBox::warning(this, tr("Error"), msg);
}

void DeviceManagerDialog::onDeviceActionCompleted() {
    // reload list after delete/deactivate action
    reloadDevices();
}

void DeviceManagerDialog::reloadDevices() {
    if (m_deviceService) {
        m_deviceService->fetchDevices();
    }
}

int DeviceManagerDialog::currentRow() const {
    auto ranges = ui->devicesTable->selectedRanges();
    if (ranges.isEmpty()) {
        return -1;
    }

    return ranges.first().topRow();
}

void DeviceManagerDialog::onRefreshClicked() {
    reloadDevices();
}

void DeviceManagerDialog::onDeactivateClicked() {
    const int row = currentRow();
    if (row < 0 || row >= m_devices.size()) {
        return;
    }

    const auto &d = m_devices[row];
    if (!d.isActive) {
        QMessageBox::information(this, tr("Info"), tr("Device is alread inactive."));
        return;
    }

    if (QMessageBox::question(this, tr("Deactivate device"), tr("Deactivate device \"%1\"?").arg(d.name)) == QMessageBox::Yes) {
        if (m_deviceService) {
            m_deviceService->deactivateDevice(d.id);
        }
    }
}

void DeviceManagerDialog::onDeleteClicked() {
    const int row = currentRow();
    if (row < 0 || row >= m_devices.size()) {
        return;
    }

    const auto &d = m_devices[row];

    if (d.isCurrent) {
        QMessageBox::warning(this, tr("Not allowed"), tr("You cannot delete current device. Deactivate it first from another session."));
        return;
    }

    if (QMessageBox::question(this, tr("Delete device"), tr("Delete device \"%1\"").arg(d.name)) == QMessageBox::Yes) {
        if (m_deviceService) {
            m_deviceService->deleteDevice(d.id);
        }
    }
}
