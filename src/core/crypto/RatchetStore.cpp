#include "RatchetStore.h"

#include "core/storage/SQLiteStorage.h"

void RatchetStore::save(const QString &peerUserId, const QString &peerDeviceId, const QJsonObject &state) {
    const QString k = keyFor(peerUserId, peerDeviceId);
    SQLiteStorage::instance().set(k, QString::fromUtf8(QJsonDocument(state).toJson(QJsonDocument::Compact)));
}

QJsonObject RatchetStore::load(const QString &peerUserId, const QString &peerDeviceId) {
    const QString k = keyFor(peerUserId, peerDeviceId);
    const QString v = SQLiteStorage::instance().get(k);

    if (v.isEmpty()) return {};

    return QJsonDocument::fromJson(v.toUtf8()).object();
}

void RatchetStore::remove(const QString &peerUserId, const QString &peerDeviceId) {
    SQLiteStorage::instance().remove(keyFor(peerUserId, peerDeviceId));
}
