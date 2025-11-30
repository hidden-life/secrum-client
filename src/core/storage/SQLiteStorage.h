#ifndef CORE_STORAGE_SQLITE_STORAGE_H
#define CORE_STORAGE_SQLITE_STORAGE_H

#include <QString>
#include <QSqlDatabase>

class SQLiteStorage {
public:
    static SQLiteStorage &instance();

    QString get(const QString &key);
    void set(const QString &key, const QString &value);
    void remove(const QString &key);

private:
    SQLiteStorage();
    QSqlDatabase m_db;
};

#endif //CORE_STORAGE_SQLITE_STORAGE_H
