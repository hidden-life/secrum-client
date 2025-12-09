#include "MainWindow.h"
#include "./ui_mainwindow.h"

#include <QMenu>
#include <QMenuBar>
#include <QAction>

#include "DeviceManagerDialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);

    connect(m_ui->chatsToolButton, &QToolButton::clicked, this, [this]() {
        switchMode(Mode::Chats);
    });

    connect(m_ui->settingsToolButton, &QToolButton::clicked, this, [this]() {
        switchMode(Mode::Settings);
    });

    connect(m_ui->listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *) {
        if (m_mode == Mode::Chats) {
            m_ui->stackWidget->setCurrentWidget(m_ui->chatPage);
        } else {
            m_ui->stackWidget->setCurrentWidget(m_ui->defaultPage);
        }
    });

    switchMode(Mode::Chats);

    m_chatService = new ChatService(this);

    connect(m_chatService, &ChatService::chatsLoaded, this, &MainWindow::renderChats);
    connect(m_chatService, &ChatService::requestFailed, this, [](QString err) {
        qDebug() << "[CHAT ERROR] " << err;
    });
    m_chatService->fetchChats();
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
            m_ui->statusLabel->setStyleSheet("color: #f44336;");
        }
    });
}

void MainWindow::switchMode(const Mode mode) {
    if (m_mode == mode) return;

    m_mode = mode;
    updateHeader();
    updateLeftPanel();

    m_ui->stackWidget->setCurrentWidget(m_ui->defaultPage);
}

void MainWindow::updateHeader() {
    m_ui->chatsToolButton->setChecked(m_mode == Mode::Chats);
    m_ui->settingsToolButton->setChecked(m_mode == Mode::Settings);
}

void MainWindow::updateLeftPanel() {
    m_ui->listWidget->clear();

    if (m_mode == Mode::Chats) {
        m_ui->searchLineEdit->setPlaceholderText("Search chats...");
        m_ui->listWidget->addItem("Alice");
        m_ui->listWidget->addItem("Bob");
        m_ui->listWidget->addItem("Work Group");
    } else {
        m_ui->searchLineEdit->setPlaceholderText("Search settings...");
        m_ui->listWidget->addItem("Profile");
        m_ui->listWidget->addItem("Devices");
        m_ui->listWidget->addItem("Security");
    }
}

void MainWindow::renderChats(const QVector<Chat> &chats) {
    m_ui->listWidget->clear();

    for (const Chat &chat : chats) {
        QString title = chat.displayName.isEmpty() ? chat.peerUserId : chat.displayName;
        if (chat.unreadCount > 0) {
            title += QString(" (%1)").arg(chat.unreadCount);
        }

        auto *item = new QListWidgetItem(title);
        if (chat.isPinned) {
            item->setBackground(Qt::yellow);
        }

        if (chat.isMuted) {
            item->setForeground(Qt::gray);
        }

        m_ui->listWidget->addItem(item);
    }
}
