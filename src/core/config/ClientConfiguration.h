#ifndef CORE_CONFIG_CLIENT_CONFIGURATION_H
#define CORE_CONFIG_CLIENT_CONFIGURATION_H
#include <QString>

class ClientConfiguration {
public:
    static ClientConfiguration &instance();

    void setAccessToken(const QString &accessToken);
    QString getAccessToken() const;
    bool isAuthorized() const;

private:
    QString m_accessToken;
};

#endif //CORE_CONFIG_CLIENT_CONFIGURATION_H
