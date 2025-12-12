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

#endif //CORE_MODEL_MESSAGE_H
