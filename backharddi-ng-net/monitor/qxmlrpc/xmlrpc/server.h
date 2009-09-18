// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>

#ifndef XMLRPC_SERVER_H
#define XMLRPC_SERVER_H

#include <QTcpServer>
#include <QPointer>

#include "variant.h"

namespace  xmlrpc {
class IncomingConnection;
/*!
 \class xmlrpc::Server server.h
 \brief The xmlrpc::Server class provides an implementation of the XML-RPC server.
 */
class Server : public QObject {
friend class IncomingConnection;    
Q_OBJECT
public:
	Server( QObject * parent = 0 );
	virtual ~Server();

    bool listen ( quint16 port, const QHostAddress & address = QHostAddress::Any );
    bool isListening() const;

    void registerMethod( QString methodName, QVariant::Type returnType, 
                         QList<QVariant::Type> parameterTypes );

    void registerMethod( QString methodName, QVariant::Type returnType );
    void registerMethod( QString methodName, QVariant::Type returnType, 
                         QVariant::Type parameter1Type );
    void registerMethod( QString methodName, QVariant::Type returnType, 
                         QVariant::Type parameter1Type, QVariant::Type parameter2Type );
    void registerMethod( QString methodName, QVariant::Type returnType, 
                         QVariant::Type parameter1Type, QVariant::Type parameter2Type, QVariant::Type parameter3Type );
    void registerMethod( QString methodName, QVariant::Type returnType, 
                         QVariant::Type parameter1Type, QVariant::Type parameter2Type, QVariant::Type parameter3Type, QVariant::Type parameter4Type );
    
    QString getClientAddress(int requestId);

signals:
    void incomingRequest( int requestId, QString methodName, QList<xmlrpc::Variant> parameters );

public slots:
    void sendReturnValue( int requestId, const xmlrpc::Variant& value );
    void sendFault( int requestId, int faultCode, QString faultMessage );


protected slots:
    void newConnection();
    void processRequest( QByteArray data, QTcpSocket *socket );

private:
	class Private;
	Private *d;
}; 

} // namespace

#endif // XMLRPC_SERVER_H


