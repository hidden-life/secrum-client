#include "ClientConfiguration.h"

ClientConfiguration &ClientConfiguration::instance() {
    static ClientConfiguration instance;
    return instance;
}

QString ClientConfiguration::baseURL() const {
    return m_baseURL;
}

void ClientConfiguration::setBaseURL(const QString &url) {
    m_baseURL = url;
}

QString ClientConfiguration::environment() const {
    return m_environment;
}

void ClientConfiguration::setEnvironment(const QString &env) {
    m_environment = env;
}

ClientConfiguration::ClientConfiguration() {
}

void ClientConfiguration::load() {
}

void ClientConfiguration::save() {
}
