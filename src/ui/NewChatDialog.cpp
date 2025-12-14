#include "NewChatDialog.h"
#include "./ui_newchatdialog.h"

NewChatDialog::NewChatDialog(QWidget *parent) :
    QDialog(parent), m_ui(new Ui::NewChatDialog), m_userSearchService(new UserSearchService(this)) {
    m_ui->setupUi(this);
    // search input
    connect(m_ui->searchLineEdit, &QLineEdit::textChanged, this, &NewChatDialog::onSearchChanged);
    // list click
    connect(m_ui->usersListWidget, &QListWidget::itemClicked, this, &NewChatDialog::onItemClicked);
    // connect service results
    connect(m_userSearchService, &UserSearchService::searchCompleted, this, [this](const QVector<UserSearchResult> &users) {
        showGlobalResults(users);
    });

    connect(m_userSearchService, &UserSearchService::searchFailed, this, [this](const QString &err) {
        m_ui->usersListWidget->clear();
        m_ui->usersListWidget->addItem("Search error: " + err);
    });

    // optional: load local contacts later here
}

NewChatDialog::~NewChatDialog() {
    delete m_ui;
}

void NewChatDialog::onSearchChanged(const QString &text) {
    const QString query = text.trimmed();
    m_ui->usersListWidget->clear();

    if (query.isEmpty()) {
        return;
    }

    m_userSearchService->search(query);
}

void NewChatDialog::onItemClicked(QListWidgetItem *item) {
    if (!item) return;

    UserSearchResult user;
    user.userId = item->data(Qt::UserRole).toString();
    user.displayName = item->data(Qt::UserRole + 1).toString();
    user.username = item->data(Qt::UserRole + 2).toString();

    if (user.userId.isEmpty()) {
        return;
    }

    emit userSelected(user);

    accept(); // close dialog
}

void NewChatDialog::loadContacts() {
}

void NewChatDialog::showContacts(const QVector<UserSearchResult> &) {
}

void NewChatDialog::showGlobalResults(const QVector<UserSearchResult> &users) {
    m_ui->usersListWidget->clear();

    if (users.isEmpty()) {
        m_ui->usersListWidget->addItem("No users found");
        return;
    }

    for (const auto &user : users) {
        auto *item = new QListWidgetItem();
        QString label = user.displayName.isEmpty() ? user.userId : user.displayName;
        if (!user.username.isEmpty()) {
            label += QString(" (@%1)").arg(user.username);
        }

        item->setText(label);
        item->setData(Qt::UserRole, user.userId);
        item->setData(Qt::UserRole + 1, user.displayName);
        item->setData(Qt::UserRole + 2, user.username);

        m_ui->usersListWidget->addItem(item);
    }
}
