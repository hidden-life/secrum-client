#ifndef UI_DEVICE_MANAGER_DIALOG_H
#define UI_DEVICE_MANAGER_DIALOG_H

#include <QDialog>

#include "core/model/Device.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DeviceManagerDialog; }
QT_END_NAMESPACE

class DeviceService;

class DeviceManagerDialog final : public QDialog {
Q_OBJECT

public:
    explicit DeviceManagerDialog(DeviceService *service, QWidget *parent = nullptr);
    ~DeviceManagerDialog() override;

private slots:
    void onDevicesLoaded(const QVector<Device> &devices);
    void onRequestFailed(const QString &msg);
    void onDeviceActionCompleted();

    void onRefreshClicked();
    void onDeactivateClicked();
    void onDeleteClicked();

private:
    Ui::DeviceManagerDialog *ui;
    DeviceService *m_deviceService = nullptr;
    QVector<Device> m_devices;

    void reloadDevices();
    int currentRow() const;
};

#endif //UI_DEVICE_MANAGER_DIALOG_H
