#include "ChatService.h"
#include "core/crypto/CryptoService.h"

#include <QJsonArray>

#include "core/auth/AuthSession.h"

ChatService::ChatService(QObject *parent) : QObject(parent), m_httpClient(new HttpClient(this)) {
    connect(m_httpClient, &HttpClient::success, this, [this](const QJsonDocument &doc) {
        if (!doc.isArray()) {
            emit requestFailed("Invalid /chats response format.");
            return;
        }

        emit chatsLoaded(parse(doc.array()));
    });

    connect(m_httpClient, &HttpClient::error, this, &ChatService::requestFailed);
}

void ChatService::fetchChats() {
    m_httpClient->get("/chats");
}

QVector<Chat> ChatService::parse(const QJsonArray &arr) {
    QVector<Chat> chats;
    chats.reserve(arr.size());

    const QString uid = AuthSession::instance().userId();
    for (const auto &val : arr) {
        if (!val.isObject()) {
            continue;
        }

        QJsonObject obj = val.toObject();
        Chat c;
        c.peerUserId = obj["peer_user_id"].toString();
        c.displayName = obj["peer_display_name"].toString();
        c.lastCipherText = obj["last_cipher_text"].toString();
        c.lastMessageAt = obj["last_message_at"].toString();
        c.unreadCount = obj["unread_count"].toInt();
        c.isPinned = obj["is_pinned"].toBool();
        c.isArchived = obj["is_archived"].toBool();
        c.isMuted = obj["is_muted"].toBool();

        chats.push_back(c);
    }

    return chats;
}
