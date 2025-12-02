#ifndef UI_LOGIN_WINDOW_H
#define UI_LOGIN_WINDOW_H

#include <QWidget>
#include "app/AuthController.h"
#include "core/network/ConnectivityService.h"

class AuthController;

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class LoginWindow final : public QWidget {
Q_OBJECT

public:
    explicit LoginWindow(AuthController *authController, QWidget *parent = nullptr);
    ~LoginWindow() override;

    void setConnectivity(ConnectivityService *svc);

signals:
    void loginCompleted(); // for Application

private slots:
    void onSendCodeClicked();
    void onLoginClicked();

    void onCodeSent(const QString &requestId, const QString &phone);
    void onLoginSuccess();
    void onLoginFailed(const QString &err);

private:
    enum class State {
        IDLE,
        SendingCode,
        CodeSent,
        Verifying
    };

    Ui::LoginWindow *m_ui;
    AuthController *m_authController;
    QString m_requestId;
    QString m_phone;
    State m_state = State::IDLE;
    ConnectivityService *m_connectivity = nullptr;

    void setState(State state);
};


#endif //UI_LOGIN_WINDOW_H
