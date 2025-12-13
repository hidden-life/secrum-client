#include "MainWindow.h"
#include "./ui_mainwindow.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QFont>

#include "core/auth/AuthSession.h"
#include "core/crypto/CryptoService.h"
#include "core/network/WSClient.h"

MainWindow::MainWindow(WSClient *wsClient, QWidget *parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);

    connect(m_ui->chatsToolButton, &QToolButton::clicked, this, [this]() {
        switchMode(Mode::Chats);
    });

    connect(m_ui->settingsToolButton, &QToolButton::clicked, this, [this]() {
        switchMode(Mode::Settings);
    });

    connect(m_ui->listWidget, &QListWidget::itemClicked, this, [this](const QListWidgetItem *item) {
        if (m_mode == Mode::Chats) {
            openChat(item->data(Qt::UserRole).toString());
        } else {
            m_ui->stackWidget->setCurrentWidget(m_ui->defaultPage);
        }
    });

    m_chatService = new ChatService(this);
    m_msgService = new MessageService(wsClient, this);

    connect(m_chatService, &ChatService::chatsLoaded, this, &MainWindow::onChatsLoaded);
    connect(m_chatService, &ChatService::requestFailed, this, &MainWindow::onChatRequestFailed);

    connect(m_ui->sendButton, &QPushButton::clicked, this, [this]() {
        const QString text = m_ui->messageEdit->toPlainText().trimmed();
        if (text.isEmpty()) {
            return;
        }

        const auto *item = m_ui->listWidget->currentItem();
        if (!item) return;

        const QString peerId = item->data(Qt::UserRole).toString();
        m_msgService->sendMessage(peerId, text);
        m_ui->messageEdit->clear();
    });

    connect(m_msgService, &MessageService::messageAdded, this, [this](const Message &msg) {
        const QString status = "⏳";
        m_ui->messageView->append(QString("<b>You:</b %1 %2").arg(msg.plainText).arg((status)));
    });

    m_userSearchService = new UserSearchService(this);
    // search by typing
    connect(m_ui->searchLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (m_mode == Mode::Chats) {
            m_userSearchService->search(text);
        }
    });

    // result
    connect(m_userSearchService, &UserSearchService::searchCompleted, this, &MainWindow::onSearchResults);

    switchMode(Mode::Chats);
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
    if (m_mode == Mode::Chats) {
        updateLeftPanelChats();
        m_chatService->fetchChats();
    } else {
        updateLeftPanelSettings();
    }

    m_ui->stackWidget->setCurrentWidget(m_ui->defaultPage);
}

void MainWindow::updateHeader() {
    m_ui->chatsToolButton->setChecked(m_mode == Mode::Chats);
    m_ui->settingsToolButton->setChecked(m_mode == Mode::Settings);
}

void MainWindow::updateLeftPanelChats() {
    m_ui->searchLineEdit->setPlaceholderText("Search chats...");
    m_ui->listWidget->clear();
}

void MainWindow::updateLeftPanelSettings() {
    m_ui->searchLineEdit->setPlaceholderText("Search settings...");
    m_ui->listWidget->clear();

    m_ui->listWidget->addItem("Profile");
    m_ui->listWidget->addItem("Devices");
    m_ui->listWidget->addItem("Security");
}

void MainWindow::openChat(const QString &peerId) {
    m_currentPeerUserId = peerId;
    m_ui->messageView->clear();
    m_ui->stackWidget->setCurrentWidget(m_ui->chatPage);
}

void MainWindow::onChatsLoaded(const QVector<Chat> &chats) {
    m_chats = chats;
    m_ui->listWidget->clear();

    const QString uid = AuthSession::instance().userId();
    auto &crypto = CryptoService::instance();

    for (const Chat &chat : chats) {
        auto *item = new QListWidgetItem();

        const QString title = chat.displayName.isEmpty() ? chat.peerUserId : chat.displayName;
        QString preview = "No messages yet.";
        if (!chat.lastCipherText.isEmpty()) {
            preview = crypto.decryptForChat(uid, chat.peerUserId, chat.lastCipherText);
        }

        if (chat.unreadCount > 0) {
            preview = QStringLiteral("(%1) %2").arg(chat.unreadCount).arg(preview);
        }

        item->setText(title + "\n" + preview);
        item->setData(Qt::UserRole, chat.peerUserId);

        if (chat.isPinned) {
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
        }

        m_ui->listWidget->addItem(item);
    }
}

void MainWindow::onChatRequestFailed(const QString &msg) {
    statusBar()->showMessage("Failed to load chats: " + msg, 5000);
}

void MainWindow::onSearchResults(const QVector<UserSearchResult> &results) {
    m_ui->listWidget->clear();

    for (const auto &u : results) {
        auto *item = new QListWidgetItem();
        QString title = u.displayName;
        if (!u.username.isEmpty()) {
            title += QString(" (@%1)").arg(u.username);
        }

        item->setText(title);
        item->setData(Qt::UserRole, u.userId);

        m_ui->listWidget->addItem(item);
    }
}
