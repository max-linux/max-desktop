// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2007
// Author Dmitry Poplavsky <dmitry.poplavsky@gmail.com>

#ifndef TESTCLIENT_H
#define TESTCLIENT_H

#include <QWidget>
#include "xmlrpc/client.h"

#include "ui_testclient.h"

class TestClient: public QWidget {
Q_OBJECT
public:
    TestClient(QWidget *parent=0);
    virtual ~TestClient();
public slots:
    void sendRequest();
private slots:
    void processReturnValue( int requestId, QVariant value );
    void processFault( int requestId, int errorCode, QString errorString );

private:
    Ui::TestClient ui;
    xmlrpc::Client *client;

    int requestIdSum;
    int requestIdDiff;
};

#endif
