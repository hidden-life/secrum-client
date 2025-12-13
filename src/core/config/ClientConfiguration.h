#ifndef CORE_CONFIG_CLIENT_CONFIGURATION_H
#define CORE_CONFIG_CLIENT_CONFIGURATION_H
#include <QString>
#include <QUrl>

class ClientConfiguration {
public:
    static ClientConfiguration &instance();

    QString baseURL() const;
    void setBaseURL(const QString &url);

    QString environment() const;
    void setEnvironment(const QString &env);

    void load();
    void save();

    QUrl wsUrl() const;

private:
    QString m_baseURL;
    QString m_environment;

    ClientConfiguration();
    void loadDefaults();
};

#endif //CORE_CONFIG_CLIENT_CONFIGURATION_H
