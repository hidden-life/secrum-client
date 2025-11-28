#ifndef UI_LOGIN_WINDOW_H
#define UI_LOGIN_WINDOW_H

#include <QWidget>

class AuthController;

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class LoginWindow : public QWidget {
Q_OBJECT

public:
    explicit LoginWindow(AuthController *controller, QWidget *parent = nullptr);
    ~LoginWindow() override;

private slots:
    void onSendCodeButtonClicked();
    void onLoginButtonClicked();

private:
    Ui::LoginWindow *ui;
    AuthController *m_authController;
};


#endif //UI_LOGIN_WINDOW_H
