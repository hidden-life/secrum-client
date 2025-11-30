#include "SQLiteStorage.h"

#include <QDir>
#include <QSqlQuery>

SQLiteStorage &SQLiteStorage::instance() {
    static SQLiteStorage instance;
    return instance;
}

SQLiteStorage::SQLiteStorage() {
    QDir().mkpath("data");

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("data/client.db");
    m_db.open();

    QSqlQuery q;
    q.exec(R"(CREATE TABLE IF NOT EXISTS client_state (key TEXT PRIMARY KEY, value TEXT))");
}

QString SQLiteStorage::get(const QString &key) {
    QSqlQuery q;
    q.prepare("SELECT value FROM client_state WHERE key = ?");
    q.addBindValue(key);
    q.exec();
    if (q.next()) {
        return q.value(0).toString();
    }

    return "";
}

void SQLiteStorage::set(const QString &key, const QString &value) {
    QSqlQuery q;
    q.prepare("INSERT OR REPLACE INTO client_state (key, value) VALUES (?, ?)");
    q.addBindValue(key);
    q.addBindValue(value);
    q.exec();
}

void SQLiteStorage::remove(const QString &key) {
    QSqlQuery q;
    q.prepare("DELETE FROM client_state WHERE key = ?");
    q.addBindValue(key);
    q.exec();
}
