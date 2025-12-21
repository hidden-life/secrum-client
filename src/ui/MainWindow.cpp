#include "MainWindow.h"
#include "./ui_mainwindow.h"
#include "NewChatDialog.h"

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
        qDebug() << "SEND BUTTON CLICKED!";
        const QString text = m_ui->messageEdit->toPlainText().trimmed();
        if (text.isEmpty()) {
            return;
        }

        if (m_currentPeerUserId.isEmpty()) {
            statusBar()->showMessage("Select a chat first.", 3000);
            return;
        }

        m_msgService->sendMessage(m_currentPeerUserId, text);
        m_ui->messageEdit->clear();
    });

    connect(m_msgService, &MessageService::messageAdded, this, [this](const Message &msg) {
        const QString status = "⏳";
        m_ui->messageView->append(QString("<b>You:</b %1 %2").arg(msg.plainText).arg((status)));
    });

    connect(m_msgService, &MessageService::messageFailed, this, [this](const QString&, const QString &reason) {
        statusBar()->showMessage("Send failed: " + reason, 5000);
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

    // new chat
    connect(m_ui->newChatToolButton, &QToolButton::clicked, this, [this]() {
        auto *newChatDialog = new NewChatDialog(this);
        connect(newChatDialog, &NewChatDialog::userSelected, this, [this](const UserSearchResult &u) {
            // add to chats if not exists
            addChatIfMissing(u);

            // open chat
            openChat(u.userId);
        });

        newChatDialog->exec();
    });

    connect(m_ui->profileToolButton, &QToolButton::clicked, this, [this]() {
        qDebug() << "Logout requested from main window!";
        emit logoutRequested();
    });

    connect(m_msgService, &MessageService::historyLoaded, this, [this](const QString &peerId, const QVector<Message> &messages) {
        if (peerId != m_currentPeerUserId) {
            return;
        }

        m_ui->messageView->clear();

        for (const auto &msg : messages) {
            const QString who = msg.isOutgoing ? "You" : "Peer";
            m_ui->messageView->append(QString("<b>%1:</b> %2").arg(who, msg.plainText));
        }
    });

    connect(m_msgService, &MessageService::historyFailed, this, [this](const QString &peerId, const QString &err) {
        if (peerId != m_currentPeerUserId) {
            return;
        }

        m_ui->messageView->append(QString("<i>History load failed: %1</i>").arg(err));
    });

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
    if (peerId.isEmpty()) {
        return;
    }

    if (m_currentPeerUserId == peerId && m_ui->stackWidget->currentWidget() == m_ui->chatPage) {
        return;
    }

    m_currentPeerUserId = peerId;
    m_ui->messageView->clear();
    m_ui->stackWidget->setCurrentWidget(m_ui->chatPage);

    // for (int i = 0; i < m_ui->listWidget->count(); ++i) {
    //     auto *it = m_ui->listWidget->item(i);
    //     if (it && it->data(Qt::UserRole).toString() == peerId) {
    //         m_ui->listWidget->setCurrentItem(it);
    //         break;
    //     }
    // }
    m_msgService->loadHistory(peerId, 50);
    m_msgService->markAllReadForPeer(peerId);
}

void MainWindow::addChatIfMissing(const UserSearchResult &u) {
    for (const auto &c : m_chats) {
        if (c.peerUserId == u.userId) {
            return;
        }
    }

    Chat chat;
    chat.peerUserId = u.userId;
    chat.displayName = u.displayName;

    m_chats.prepend(chat);
    onChatsLoaded(m_chats);
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
