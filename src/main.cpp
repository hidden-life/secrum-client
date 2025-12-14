#include  "app/Application.h"

static void msgHandler(QtMsgType type, const QMessageLogContext &, const QString &msg) {
    fprintf(stderr, "QT: %s\n", msg.toLocal8Bit().constData());
    fflush(stderr);
    if (type == QtFatalMsg) {
        abort();
    }
}

int main(int argc, char *argv[]) {
    qInstallMessageHandler(msgHandler);

    Application app;

    return app.start(argc, argv);
}