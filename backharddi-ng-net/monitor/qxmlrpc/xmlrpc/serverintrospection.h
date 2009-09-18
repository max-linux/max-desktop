// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2007
// Author Dmitry Poplavsky <dmitry.poplavsky@gmail.com>

#ifndef SERVERINTROSPECTION_H
#define SERVERINTROSPECTION_H

#include "xmlrpc/variant.h"
namespace  xmlrpc {

/**
 * \brief ServerIntrospection class implements introspection
 *        functionality to the xmlrpc::Server.
 * 
 * \class xmlrpc::ServerIntrospection serverintrospection.h
 *
 * It provides information about methods of the xmlrpc server to
 * clients and allows to perform method name and parameters type
 * checks on the server before calling user code.
 *
 * ServerIntrospection is usually not used directly but from
 * the xmlrpc::Server.
 * 
 * Check http://scripts.incutio.com/xmlrpc/introspection.html
 * for more information about XML-RPC introspection.
 **/
class ServerIntrospection {
public:
    ServerIntrospection();
    virtual ~ServerIntrospection();

    void registerMethod( QString methodName, QVariant::Type returnType, QList<QVariant::Type> parameterTypes );
    void setMethodHelpText( QString methodName, QString helpText );
    void clear();

    bool isEmpty() const;

    bool isMethodSupported( QString methodName ) const;
    bool checkMethodParameters( QString methodName, const QList<xmlrpc::Variant>& parameters, QVariant::Type *returnType = 0 ) const;


    QStringList listMethods() const;
    QStringList methodSignatures( QString methodName ) const;
    QString methodHelp( QString methodName );

private:
    class Private;
	QSharedDataPointer<Private> d;
};

}; // namespace
 
#endif //SERVERINTROSPECTION_H
