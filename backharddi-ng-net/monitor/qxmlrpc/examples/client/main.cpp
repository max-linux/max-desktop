#include <QApplication>

#include "testclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestClient client;
    client.show();
    return app.exec();
}
