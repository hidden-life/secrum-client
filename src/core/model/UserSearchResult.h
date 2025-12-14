#ifndef CORE_MODEL_USER_SEARCH_RESULT_H
#define CORE_MODEL_USER_SEARCH_RESULT_H

#include <QString>

struct UserSearchResult {
    QString userId;
    QString displayName;
    QString username;
};

Q_DECLARE_METATYPE(UserSearchResult)

#endif //CORE_MODEL_USER_SEARCH_RESULT_H
