#ifndef CORE_MODEL_MESSAGE_H
#define CORE_MODEL_MESSAGE_H

#include <QString>
#include <QDateTime>

enum class MessageStatus {
    Sending,
    Sent,
    Delivered,
    Read,
    Failed,
};

struct Message {
    QString id;
    QString peerUserId;
    QString plainText;
    QString cipherText;

    bool isOutgoing = true;
    MessageStatus status = MessageStatus::Sending;

    QDateTime createdAt;
};

inline QString toString(MessageStatus status) {
    switch (status) {
        case MessageStatus::Sending: return "sending";
        case MessageStatus::Sent: return "sent";
        case MessageStatus::Delivered: return "delivered";
        case MessageStatus::Read: return "read";
        case MessageStatus::Failed: return "failed";
    }

    return "unknown";
}

inline MessageStatus fromString(const QString &str) {
    if (str == "sending") return MessageStatus::Sending;
    if (str == "sent") return MessageStatus::Sent;
    if (str == "delivered") return MessageStatus::Delivered;
    if (str == "read") return MessageStatus::Read;
    if (str == "failed") return MessageStatus::Failed;

    return MessageStatus::Failed;
}

#endif //CORE_MODEL_MESSAGE_H
