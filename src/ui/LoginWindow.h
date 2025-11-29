#ifndef UI_LOGIN_WINDOW_H
#define UI_LOGIN_WINDOW_H

#include <QWidget>
#include "app/AuthController.h"

class AuthController;

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class LoginWindow final : public QWidget {
Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() override;

signals:
    void loginCompleted();

private:
    Ui::LoginWindow *ui;
    AuthController *m_authController;
};


#endif //UI_LOGIN_WINDOW_H
