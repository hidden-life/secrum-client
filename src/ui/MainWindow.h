#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H

#include <QMainWindow>

#include "core/device/DeviceService.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void openDevicesDialog();

private:
    Ui::MainWindow *m_ui;
    DeviceService *m_deviceService = nullptr;

    void setupMenus();
};


#endif //UI_MAIN_WINDOW_H
