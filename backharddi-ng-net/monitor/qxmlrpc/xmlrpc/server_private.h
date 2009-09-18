// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>

#ifndef XMLRPC_SERVER_PRIVATE_H
#define XMLRPC_SERVER_PRIVATE_H

#include <QTcpServer>
#include <QPointer>

#include "variant.h"

namespace  xmlrpc {

class Server;

//For internal use by xmlrpc::Serevr
//It collects data from one connection, and calls parent->processRequest()
class IncomingConnection : public QObject
{
Q_OBJECT
public:
    IncomingConnection(Server *parent, QTcpSocket *socket );
public slots:
    void readData();
private:
    Server *server;
    QPointer<QTcpSocket> socket;
    QByteArray data;
};



} // namespace

#endif // XMLRPC_SERVER_H


