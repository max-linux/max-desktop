//this file just provides main page for doxygen

/** \mainpage QXmlRPC Documentation
* 
* \section intro_sec Introduction
*
* The QXML-RPC library is a full Qt4 based implementation of 
* XML-RPC ( http://www.xmlrpc.com/ ) protocol.
*
* It provides an easy way to construct, 
* send and receive XML-RPC messages on the client side, 
* and process XML-RPC messages on the server side. 
* It also supports XML-RPC 
* introspection (
* http://scripts.incutio.com/xmlrpc/introspection.html ) to
* provide information about methods of the xmlrpc server to
* clients and allows to perform method name and parameters type
* checks on the server before calling user implemented methods
* code.
* 
* To pass parameters of XML-RPC request, an QVariant based
* xmlrpc::Variant class is used. It provides deserialization and
* serialization from/to QDomElement, according to XML-RPC
* specification. It also restricts set of QVariant types to
* types accepted in XML-RPC protocol. This allows to perform
* compile time type checks and avoid of some bugs.
*
* This project is inspired but not based on the QuteXR library.
* 
* 
* \section client_subsec Building a XML-RPC client
* 
* Just create a xmlrpc::Client instance,
* set the XML-RPC server with xmlrpc::Client::setHost()
* and if necessary with xmlrpc::Client::setProxy() and
* xmlrpc::Client::setUser().
* 
* \code
* client = new xmlrpc::Client(this);
* connect( client, SIGNAL(done( int, QVariant )),
*          this, SLOT(processReturnValue( int, QVariant )) );
* connect( client, SIGNAL(failed( int, int, QString )),
*          this, SLOT(processFault( int, int, QString )) );
* 
* client->setHost( "localhost", 7777 );
* 
* int requestId = client->request( "sum", x, y )
* 
* \endcode
* 
* After the request is finished, done() or failed() signal will
* be emited with the request id and return value or fault
* information.
* 
* For more information check xmlrpc::Client class documentation
* and client example stored in examples/client directory.
* 
* \section server_sec Building a standalone XML-RPC server
* 
* To build a standalone XML-RPC server, just create a
* xmlrpc::Server instance and call xmlrpc::Server::listen() to
* listen for requests on specified port.
* 
* You also can register provided methods with
* xmlrpc::Server::registerMethod() call. In this case the server
* will perform method name, parameters and return value type
* checks before emiting of incomingRequest() signal. It will
* also provide information about provided methods to clients.
* 
* Server instance will emit incomingRequest() signal,
* with the method name and parameters list. After the
* corresponding method is processed, Server::sendReturnValue()
* or Server::sendFault() must be called.
* 
* \section server_cgi_sec Building non standalone XML-RPC server
* 
* If the standalone server is not acceptable, for example in CGI
* like environment, the request can be parsed with
* xmlrpc::Request class, processed, and result ( the return
* value or fault information ) encoded with xmlrpc::Response
* class. The xmlrpc::ServerIntrospection class can optionaly be
* used to perform method names and types checks.
*
* \section examples_sec Examples
*
* For examples of QXmlRPC based XML-RPC client and server
* check examples/client and examples/server directories.
*
* Similar python based implementations are stored 
* in test/pyclient and test/pyserver directories.
*
*
*/
