#include "JWT.h"

QJsonObject JWT::decode(const QString &jwt) {
    auto parts = jwt.split('.');
    if (parts.size() != 3) {
        return {};
    }

    const QByteArray payload = QByteArray::fromBase64(parts[1].toUtf8(), QByteArray::Base64UrlEncoding);
    const QJsonDocument doc = QJsonDocument::fromJson(payload);

    return doc.object();
}

QString JWT::getClaim(const QString &jwt, const QString &key) {
    auto obj = decode(jwt);

    return obj.contains(key) ? obj[key].toString() : QString();
}