#include <QApplication>

#include "testserver.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestServer server(7777);
    return app.exec();
}
