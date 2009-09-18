// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2007
// Author Dmitry Poplavsky <dmitry.poplavsky@gmail.com>

#include "serverintrospection.h"

namespace xmlrpc {

    /** MethodSignature  */
    class MethodSignature {
    public:
        MethodSignature( QString methodName, QVariant::Type returnType, QList<QVariant::Type> parameterTypes )
        {
            this->methodName = methodName;
            this->returnType = returnType;
            this->parameterTypes = parameterTypes;
        }
    
        bool checkParameterTypes( const QList<xmlrpc::Variant>& parameters ) 
        {
            if ( parameters.count() != parameterTypes.count() ) 
                return false;
    
            for ( int i=0; i<parameters.count(); i++ ) {
                if ( parameters[i].type() != parameterTypes[i] )
                    return false;
            }
    
            return true;
        }

        //! return string presentation to be used in xml-rpc responses
        QString toString()
        {
            QStringList items;

            static QMap< QVariant::Type, QString > typeNames = QMap< QVariant::Type, QString >();
            if ( typeNames.isEmpty() ) {
                typeNames[ QVariant::Int ] = "int";
                typeNames[ QVariant::UInt ] = "int";
                typeNames[ QVariant::Bool ] = "boolean";
                typeNames[ QVariant::String ] = "string";
                typeNames[ QVariant::Double ] = "string";
                typeNames[ QVariant::DateTime ] = "dateTime.iso8601";
                typeNames[ QVariant::List ] = "array";
                typeNames[ QVariant::Map ] = "struct";
            }

            items << typeNames.value(returnType);

            foreach( QVariant::Type parameterType, parameterTypes ) {
                items << typeNames.value(parameterType);
            }

            return items.join(",");
        }

    
        QString methodName;
        QVariant::Type returnType;
        QList<QVariant::Type> parameterTypes;


    };
};


using namespace xmlrpc;

class ServerIntrospection::Private: public QSharedData
{
public:
    Private() {
    };

    // methodName -> return value and parameters types
    QMultiMap< QString, MethodSignature > methodSignatures;
    QMap<QString, QString> methodsHelp;

};

ServerIntrospection::ServerIntrospection()
{
    d = new Private();
}

ServerIntrospection::~ServerIntrospection()
{
}

/**
 * Register method \a methodName
 * \sa xmlrpc::Server::registerMethod()
 */
void ServerIntrospection::registerMethod( QString methodName, QVariant::Type returnType, QList<QVariant::Type> parameterTypes )
{
    d->methodSignatures.insert( methodName, MethodSignature(methodName,returnType,parameterTypes) );
}

/**
 * Register help message for method \a methonName.
 * 
 * \sa methodHelp()
 */
void ServerIntrospection::setMethodHelpText( QString methodName, QString helpText )
{
    d->methodsHelp[methodName] = helpText;
}

/**
 * Clear methods signatures and help data
 */
void ServerIntrospection::clear()
{
    d->methodSignatures.clear();
    d->methodsHelp.clear();
}

/**
 * Returns true if the object contains no items; otherwise
 * returns false.
 */
bool ServerIntrospection::isEmpty() const
{
    return d->methodSignatures.isEmpty();
}

/**
 * Returns true if the method methodName is registered;
 * otherwise returns false.
 */
bool ServerIntrospection::isMethodSupported( QString methodName ) const
{
    return d->methodSignatures.contains( methodName );
}


/**
 * Returns true if types of \a parameters correspond to at least
 * one method methodName signature. If returnType != 0 it will
 * be filled with expected return value type for this method and
 * parameters.
 */
bool ServerIntrospection::checkMethodParameters( QString methodName, 
                                                 const QList<xmlrpc::Variant>& parameters,
                                                 QVariant::Type *returnType ) const
{
    QList<MethodSignature> signatures = d->methodSignatures.values(methodName);

    foreach( MethodSignature signature, signatures ) {
        if ( signature.checkParameterTypes(parameters) ) {
            if ( returnType )
                *returnType = signature.returnType;

            return true;
        }
    }

    return false;
}

/**
 * Return list of registered methods.
 * This method is intended to be used for "system.listMethods"
 * XML-RPC call.
 */
QStringList ServerIntrospection::listMethods() const
{
    return d->methodSignatures.keys();
}

/**
 * Return list of registered method signatures for method
 * methodName. This method is intended to be used for
 * "system.methodSignature" XML-RPC call.
 * 
 * Signatures themselves are restricted to the top level
 * parameters expected by a method. For instance if a method
 * expects one array of structs as a parameter, and it returns a
 * string, its signature is simply "string, array". If it
 * expects three integers, its signature is "string, int, int,
 * int".
 */
QStringList ServerIntrospection::methodSignatures( QString methodName ) const
{
    QList<MethodSignature> signatures = d->methodSignatures.values(methodName);

    QStringList res;
    foreach( MethodSignature signature, signatures ) {
        res << signature.toString();
    }

    return res;
}

/**
 * Returns a documentation string describing the use of method
 * methodName. If no such string is available, an empty string
 * is returned. The documentation string may contain HTML
 * markup.
 * 
 * This method is intended to be used for "system.methodHelp"
 * XML-RPC call.
 */
QString ServerIntrospection::methodHelp( QString methodName )
{
    return d->methodsHelp.value( methodName, "" );
}


