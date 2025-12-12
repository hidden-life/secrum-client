#include "MessageService.h"

#include "core/auth/AuthSession.h"
#include "core/crypto/CryptoService.h"

MessageService::MessageService(QObject *parent) : QObject(parent), m_httpClient(new HttpClient(this)) {
    connect(m_httpClient, &HttpClient::success, this, [this](const QJsonDocument &doc) {
        if (!doc.isObject()) {
            return;
        }

        const QJsonObject obj = doc.object();
        Message msg;
        msg.id = obj["message_id"].toString();
        msg.status = MessageStatus::Sent;

        emit messageUpdated(msg);
    });

    connect(m_httpClient, &HttpClient::error, this, [this](const QString &err) {
        emit messageFailed("", err);
    });
}

void MessageService::sendMessage(const QString &peerUserId, const QString &text) {
    auto &crypto = CryptoService::instance();
    const QString uid = AuthSession::instance().userId();

    // encrypt
    const QString cipher = crypto.encryptForChat(uid, peerUserId, text);

    // optimistic message
    Message msg;
    msg.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    msg.peerUserId = peerUserId;
    msg.plainText = text;
    msg.cipherText = cipher;
    msg.isOutgoing = true;
    msg.status = MessageStatus::Sending;
    msg.createdAt = QDateTime::currentDateTime();

    emit messageAdded(msg);

    // send to server
    const QJsonObject body {
        {"recipient_user_id", peerUserId},
        {"cipher_text", cipher}
    };

    m_httpClient->post("/message/send", body);
}
