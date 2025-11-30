#ifndef JWT_H
#define JWT_H

#include <QJsonObject>

class JWT {
public:
    static QJsonObject decode(const QString &jwt);
    static QString getClaim(const QString &jwt, const QString &key);
};

#endif //JWT_H
