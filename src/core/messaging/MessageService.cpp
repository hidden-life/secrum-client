#include "MessageService.h"

#include "core/auth/AuthSession.h"
#include "core/crypto/CryptoService.h"

MessageService::MessageService(WSClient *wsClient, QObject *parent) : QObject(parent), m_wsClient(wsClient) {
    connect(wsClient, &WSClient::eventReceived, this, &MessageService::handleIncoming);
}

void MessageService::sendMessage(const QString &peerUserId, const QString &text) {
    Message msg;
    msg.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    msg.peerUserId = peerUserId;
    msg.plainText = text;
    msg.status = MessageStatus::Sending;
    msg.isOutgoing = true;
    msg.createdAt = QDateTime::currentDateTimeUtc();

    const QString uid = AuthSession::instance().userId();
    msg.cipherText = CryptoService::instance().encryptForChat(uid, peerUserId, text);

    m_messages[msg.id] = msg;

    emit messageAdded(msg);

    QJsonObject body;
    body["id"] = msg.id;
    body["recipient_user_id"] = peerUserId;
    body["cipher_text"] = msg.cipherText;

    m_wsClient->send("message", body);
}

void MessageService::markDelivered(const QString &msgId) {
}

void MessageService::markRead(const QString &msgId) {
}

void MessageService::handleIncoming(const QString &type, const QJsonObject &json) {
    if (type == "message") {
        Message msg;
        msg.id = json["id"].toString();
        msg.peerUserId = json["sender_user_id"].toString();
        msg.cipherText = json["cipher_text"].toString();
        msg.isOutgoing = false;
        msg.status = MessageStatus::Delivered;
        msg.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);

        const QString uid = AuthSession::instance().userId();
        msg.plainText = CryptoService::instance().decryptForChat(uid, msg.peerUserId, msg.cipherText);

        m_messages[msg.id] = msg;

        emit messageAdded(msg);

        QJsonObject ack;
        ack["message_id"] = msg.id;
        m_wsClient->send("ack_delivered", ack);

        return;
    }

    if (type == "ack_delivered") {
        if (const QString id = json["message_id"].toString(); m_messages.contains(id)) {
            Message &msg = m_messages[id];
            msg.status = MessageStatus::Delivered;

            emit messageUpdated(msg);
            return;
        }
    }

    if (type == "ack_read") {
        const QString id = json["message_id"].toString();
        if (!m_messages.contains(id)) {
            return;
        }

        Message &msg = m_messages[id];
        msg.status = MessageStatus::Read;

        emit messageUpdated(msg);
        return;
    }
}
