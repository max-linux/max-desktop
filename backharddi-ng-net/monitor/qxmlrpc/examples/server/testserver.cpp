// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2007
// Author Dmitry Poplavsky <dmitry.poplavsky@gmail.com>

#include "testserver.h"

TestServer::TestServer(int port, QObject *parent)
    :QObject(parent)
{
    server = new xmlrpc::Server(this);

    //register sum and difference methods, with return type int and two int parameters
    server->registerMethod( "sum", QVariant::Int, QVariant::Int, QVariant::Int );
    server->registerMethod( "difference", QVariant::Int, QVariant::Int, QVariant::Int );

    connect( server, SIGNAL(incomingRequest( int, QString, QList<xmlrpc::Variant>)),
             this, SLOT(processRequest( int, QString, QList<xmlrpc::Variant>)));
    
    if( server->listen( port ) ) {
        qDebug() << "Listening for XML-RPC requests on port" << port;
    } else {
        qDebug() << "Error listening port" << port;
    }
}

TestServer::~TestServer()
{
}

void TestServer::processRequest( int requestId, QString methodName, QList<xmlrpc::Variant> parameters )
{
    // we doun't have to check parameters count and types here
    // since we registered methods "sum" and "difference"
    // with server->registerMethod() call

    int x = parameters[0].toInt();
    int y = parameters[1].toInt();

    qDebug() << methodName  << x << y; 

    if ( methodName == "sum" ) {
        server->sendReturnValue( requestId, x+y );
    }
    
    if ( methodName == "difference" ) {
        server->sendReturnValue( requestId, x-y );
    }
}


