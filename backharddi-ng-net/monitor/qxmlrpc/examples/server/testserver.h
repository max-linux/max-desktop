// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2007
// Author Dmitry Poplavsky <dmitry.poplavsky@gmail.com>

#ifndef TESTSERVER_H
#define TESTSERVER_H

#include "xmlrpc/server.h"

class TestServer: public QObject{
Q_OBJECT
public:
    TestServer(int port, QObject *parent = 0);
    virtual ~TestServer();
private slots:
    void processRequest( int requestId, QString methodName, QList<xmlrpc::Variant> parameters );

private:
    xmlrpc::Server *server;
};

#endif
