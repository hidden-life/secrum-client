#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H

#include <QMainWindow>

#include "core/device/DeviceService.h"
#include "core/network/ConnectivityService.h"
#include "core/chat/ChatService.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setConnectivity(ConnectivityService *svc);

private:
    Ui::MainWindow *m_ui;
    ChatService *m_chatService = nullptr;

    enum class Mode {
        None,
        Chats,
        Settings,
    };

    Mode m_mode = Mode::None;

    void switchMode(Mode mode);
    void updateHeader();
    void updateLeftPanel();

private slots:
    void renderChats(const QVector<Chat> &chats);
};


#endif //UI_MAIN_WINDOW_H
