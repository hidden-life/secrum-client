#ifndef CORE_MODEL_CHAT_H
#define CORE_MODEL_CHAT_H

#include <QString>

struct Chat {
    QString peerUserId;
    QString displayName;
    QString lastCipherText;
    QString lastMessageAt;

    int unreadCount = 0;

    bool isPinned = false;
    bool isArchived = false;
    bool isMuted = false;
};

#endif //CORE_MODEL_CHAT_H
