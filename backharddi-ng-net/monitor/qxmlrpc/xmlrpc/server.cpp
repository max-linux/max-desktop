// vim:tabstop=4:shiftwidth=4:foldmethod=marker:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>

#include "xmlrpc/server.h"
#include "xmlrpc/server_private.h"
#include "xmlrpc/variant.h"
#include "xmlrpc/request.h"
#include "xmlrpc/response.h"
#include "xmlrpc/serverintrospection.h"

#include <QtNetwork>
#include <QtXml>

namespace  xmlrpc {

class Server::Private
{
public:
    Private() 
    {
        lastRequestId = 0;
    }

    
    QTcpServer *server;

    int lastRequestId;
    QMap< int, QPointer<QTcpSocket> > processingRequests;

    // requestId -> type of return value map
    QMap< int, QVariant::Type > expectedReturnTypes;

    QSet<int> keepAliveRequests;

    ServerIntrospection introspection;

    //send 200/OK http response back to socket
    void sendResponse( QTcpSocket *socket, QByteArray data, bool keepAlive );

};

//send http response back to socket
void Server::Private::sendResponse( QTcpSocket *socket, QByteArray data, bool keepAlive )
{
    QHttpResponseHeader header(200,"OK");
    header.setContentType("text/xml");

    if ( keepAlive ) {
        header.setValue( "Connection", "Keep-Alive" );
    } else {
        header.setValue( "Connection", "Close" );
    }

    header.setValue( "Content-Encoding", "UTF-8" );
    header.setContentLength( data.size() );

    socket->write( header.toString().toUtf8() );
    socket->write( data );


    if ( !keepAlive ) {
        //qDebug() << "close connection";
        socket->close();
        socket->disconnectFromHost();
    }
}

/**
 * Create new xmlrpc::Server instance.
 */
Server::Server( QObject * parent)
: QObject( parent )
{
    d = new Private;
    d->server = new QTcpServer(this);

    connect( d->server, SIGNAL(newConnection()), SLOT(newConnection()) );
}

/** Delete xmlrpc::Server instance. */
Server::~Server()
{
    delete d->server;
    delete d;
}

/**
 * Tells the server to listen for incoming connections on
 * address address and port port. If address is QHostAddress::Any, the server
 * will listen on all network interfaces.
 * 
 * Returns true on success; otherwise returns false.
 * 
 * \sa isListening()
 */
bool Server::listen ( quint16 port, const QHostAddress & address )
{
    return d->server->listen( address, port );
}

/**
 * Returns true if the server is currently listening for
 * incoming connections; otherwise returns false.
 * 
 * \sa listen()
 */
bool Server::isListening() const
{
    return d->server->isListening();
}

/**
 * Register method \a methodName as supported by this server
 * instance.
 * 
 * This allows the server to provide information about methods,
 * parameters and return values to clients.
 * 
 * This data is also used to perform checks for supported
 * methodNames and parameter types before incomingRequest()
 * signal is emited. This allows to remove types checks from
 * the methods implementation and avoid of possible bugs.
 * 
 * Methods registration is optional, but if at least one method
 * is registered, all other have to be registered too,
 * since this data will be used for method names checks.
 * 
 * Warning: only top level types are checked. For example if
 * some method has two parametrs: int and QList<double>.
 * Type of the first parameter will be checked, but the second
 * parameter will be only checked to be a list, but type of
 * items in the list must be checked in the implementation of
 * this method.
 */
void Server::registerMethod( QString methodName, QVariant::Type returnType, 
                             QList<QVariant::Type> parameterTypes )
{
    d->introspection.registerMethod(methodName,returnType,parameterTypes);
}

/** This is an overloaded member function, provided
 *  for convenience. */
void Server::registerMethod( QString methodName, QVariant::Type returnType, 
                             QVariant::Type parameter1Type, QVariant::Type parameter2Type, QVariant::Type parameter3Type, QVariant::Type parameter4Type )
{
    QList<QVariant::Type> parameterTypes;
    parameterTypes << parameter1Type << parameter2Type << parameter3Type << parameter4Type;

    registerMethod( methodName, returnType, parameterTypes );
}

/** This is an overloaded member function, provided
 *  for convenience. */
void Server::registerMethod( QString methodName, QVariant::Type returnType, 
                             QVariant::Type parameter1Type, QVariant::Type parameter2Type, QVariant::Type parameter3Type )
{
    QList<QVariant::Type> parameterTypes;
    parameterTypes << parameter1Type << parameter2Type << parameter3Type;

    registerMethod( methodName, returnType, parameterTypes );
}

/** This is an overloaded member function, provided
 *  for convenience. */
void Server::registerMethod( QString methodName, QVariant::Type returnType, 
                             QVariant::Type parameter1Type, QVariant::Type parameter2Type )
{
    QList<QVariant::Type> parameterTypes;
    parameterTypes << parameter1Type << parameter2Type;

    registerMethod( methodName, returnType, parameterTypes );
}

/** This is an overloaded member function, provided
 *  for convenience. */
void Server::registerMethod( QString methodName, QVariant::Type returnType, 
                             QVariant::Type parameter1Type )
{
    QList<QVariant::Type> parameterTypes;
    parameterTypes << parameter1Type;

    registerMethod( methodName, returnType, parameterTypes );
}

/** This is an overloaded member function, provided
 *  for convenience. */
void Server::registerMethod( QString methodName, QVariant::Type returnType )
{
    QList<QVariant::Type> parameterTypes;

    registerMethod( methodName, returnType, parameterTypes );
}

void Server::newConnection()
{
    //qDebug() << "new connection";
    while ( d->server->hasPendingConnections() ) {
        QTcpSocket *socket = d->server->nextPendingConnection();
    
        if ( !socket )
            return;
    
        
        connect(socket, SIGNAL(disconnected()),
                socket, SLOT(deleteLater()));
    
        new IncomingConnection(this, socket);
    }
}

/**
 *  Parse incoming \a request, and emit incomingRequest() signal
 *  or return xmlrpc response back to \a socket */
void Server::processRequest( QByteArray request, QTcpSocket *socket )
{
    d->lastRequestId++;


    int requestId = d->lastRequestId;
    d->processingRequests[ requestId ] = socket;


    int dataPos = request.indexOf("\r\n\r\n")+4;

    if ( dataPos < 0 ) {
        qDebug() << "did not find data part";
        socket->disconnectFromHost();
        return;
    }

    QByteArray requestHeader = request.left( dataPos );
    QByteArray requestData = request.mid( dataPos );

    QHttpRequestHeader httpRequestHeader( QString::fromUtf8(requestHeader) );

    bool keepAlive = httpRequestHeader.value("Connection").toLower() == "keep-alive";
    if ( keepAlive ) {
        d->keepAliveRequests.insert( requestId );
    }

    
    /*qDebug() << "request:";
    if ( request.count() < 1024 )
        qDebug() << QString( request );
    else 
        qDebug() << "too long";*/
    
    
    Request parsedRequest;
    QString errorMessage;

    if ( !parsedRequest.setContent(requestData,&errorMessage) ) {
        sendFault( requestId, -32600, errorMessage );
        return;
    }

    QString methodName = parsedRequest.methodName();
    QList<Variant> parameters = parsedRequest.parameters();

    if ( methodName.startsWith("system.") ) {
        //reserved server methods

        if ( methodName == "system.listMethods" ) {
            sendReturnValue(requestId, Variant( d->introspection.listMethods() ) );
            return;
        }

        if ( methodName == "system.methodSignature" && parameters.count() == 1 ) {
            QString methodName = parameters[0].toString();

            sendReturnValue(requestId, Variant( d->introspection.methodSignatures(methodName) ) );
            return;
        }

        if ( methodName == "system.methodHelp" && parameters.count() == 1 ) {
            QString methodName = parameters[0].toString();

            sendReturnValue(requestId, Variant( d->introspection.methodHelp(methodName) ) );
            return;
        }

        sendFault(requestId, -32601, "Method not found" );
        return;
    }

    if ( !d->introspection.isEmpty() ) {
        // check method name first:

        if ( !d->introspection.isMethodSupported(methodName) ) {
            sendFault(requestId, -32601, "Method not found" );
            return;
        }

        QVariant::Type returnType;

        

        if ( !d->introspection.checkMethodParameters( methodName,
                                                      parameters,
                                                      &returnType ) ) {
            sendFault(requestId, -32602, "Invalid method parameters" );
            return;
        }

        d->expectedReturnTypes[ requestId ] = returnType;
    }

    emit incomingRequest( requestId, methodName, parameters );
}

/**
 * Send method return value back to client.
 * @param requestId id of the request, provided by
 *                  incomingRequest() signal
 * @param value to be returned to client
 */
void Server::sendReturnValue( int requestId, const xmlrpc::Variant& value )
{
    Q_ASSERT( d->processingRequests.contains( requestId ) );

    //check for return type
    if ( d->expectedReturnTypes.contains(requestId) ) {
        Q_ASSERT( value.type() == d->expectedReturnTypes[requestId] );
        d->expectedReturnTypes.remove( requestId );
    }

    Response response( value );

    QTcpSocket *socket =  d->processingRequests.take(requestId);
    d->sendResponse( socket, response.composeResponse(), d->keepAliveRequests.contains(requestId) );
    d->keepAliveRequests.remove( requestId );
    
}

/**
 * Send exception ( faultCode and faultMessage ) back to client.
 * @param requestId id of the request, provided by
 *                  incomingRequest() signal
 */
void Server::sendFault( int requestId, int faultCode, QString faultMessage )
{
    Q_ASSERT( d->processingRequests.contains( requestId ) );

    Response response( faultCode, faultMessage );

    QTcpSocket *socket =  d->processingRequests.take(requestId);

    d->sendResponse( socket, response.composeResponse(), d->keepAliveRequests.contains(requestId) );
    d->keepAliveRequests.remove( requestId );
}



IncomingConnection::IncomingConnection(Server *server, QTcpSocket *socket )
    :QObject(server)
{	
    this->server = server;
    this->socket = socket;

    connect( socket, SIGNAL(readyRead()), this, SLOT(readData()) );
    readData();
}

void IncomingConnection::readData()
{
    if ( !socket ) {
        deleteLater();
        return;
    }

    if ( socket->bytesAvailable() == 0 ) 
        return;

    data.append( socket->readAll() );

    if ( data.count() > 15 && data.indexOf( "</methodCall>", data.count()-15 ) != -1 ) {
        server->processRequest( data, socket );
        deleteLater();
    }
}

QString Server::getClientAddress(int requestId)
{
	QTcpSocket *socket =  d->processingRequests[requestId];
	if(socket)
		return socket->peerAddress().toString();
	else
		return QString();
}


} // namespace xmlrpc


