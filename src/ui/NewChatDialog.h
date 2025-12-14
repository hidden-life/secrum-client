#ifndef UI_NEW_CHAT_DIALOG_H
#define UI_NEW_CHAT_DIALOG_H

#include <QDialog>
#include <QListWidgetItem>

#include "core/model/UserSearchResult.h"
#include "core/users/UserSearchService.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NewChatDialog; }
QT_END_NAMESPACE

class NewChatDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewChatDialog(QWidget *parent = nullptr);
    ~NewChatDialog() override;

signals:
    void userSelected(const UserSearchResult &user);

private slots:
    void onSearchChanged(const QString &text);
    void onItemClicked(QListWidgetItem *item);

private:
    Ui::NewChatDialog *m_ui;
    UserSearchService *m_userSearchService;

    void loadContacts();
    void showContacts(const QVector<UserSearchResult> &);
    void showGlobalResults(const QVector<UserSearchResult> &);
};

#endif //UI_NEW_CHAT_DIALOG_H
