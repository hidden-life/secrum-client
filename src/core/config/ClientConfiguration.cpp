#include "ClientConfiguration.h"

ClientConfiguration &ClientConfiguration::instance() {
    static ClientConfiguration instance;
    return instance;
}

void ClientConfiguration::setAccessToken(const QString &accessToken) {
    m_accessToken = accessToken;
}

QString ClientConfiguration::getAccessToken() const {
    return m_accessToken;
}

bool ClientConfiguration::isAuthorized() const {
    return !m_accessToken.isEmpty();
}