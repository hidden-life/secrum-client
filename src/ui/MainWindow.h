#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(const QString &accessToken, QWidget *parent = nullptr);
    ~MainWindow() override;

    QString accessToken() const;

private:
    Ui::MainWindow *ui;
    QString m_accessToken;
};


#endif //UI_MAIN_WINDOW_H
