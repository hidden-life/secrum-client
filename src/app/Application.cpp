#include "Application.h"

#include <QMainWindow>

Application::Application() {}


void Application::start() {
    auto *mainWnd = new QMainWindow();
    mainWnd->setWindowTitle("SecRum (alpha))");
    mainWnd->resize(400, 300);
    mainWnd->show();
}