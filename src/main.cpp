#include <QApplication>
#include  "app/Application.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Application application;
    application.start();

    return a.exec();
}