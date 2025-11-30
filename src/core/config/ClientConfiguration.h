#ifndef CORE_CONFIG_CLIENT_CONFIGURATION_H
#define CORE_CONFIG_CLIENT_CONFIGURATION_H
#include <QString>

class ClientConfiguration {
public:
    static ClientConfiguration &instance();

    QString baseURL() const;
    void setBaseURL(const QString &url);

    QString environment() const;
    void setEnvironment(const QString &env);

private:
    QString m_baseURL;
    QString m_environment;

    ClientConfiguration();
    void load();
    void save();
};

#endif //CORE_CONFIG_CLIENT_CONFIGURATION_H
