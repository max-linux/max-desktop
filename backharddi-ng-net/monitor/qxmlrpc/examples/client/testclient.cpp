// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2007
// Author Dmitry Poplavsky <dmitry.poplavsky@gmail.com>

#include <QApplication>
#include <QMessageBox>

#include "testclient.h"

TestClient::TestClient(QWidget *parent)
    :QWidget(parent)
{
    ui.setupUi(this);

    ui.x->setValidator( new QIntValidator( this ) );
    ui.y->setValidator( new QIntValidator( this ) );
    

    requestIdSum = -1;
    requestIdDiff = -1;

    connect( ui.callButton, SIGNAL(clicked()), SLOT(sendRequest()) );
    connect( ui.exitButton, SIGNAL(clicked()), qApp, SLOT(quit()) );
    
    client = new xmlrpc::Client(this);
    
    connect( client, SIGNAL(done( int, QVariant )),
             this, SLOT(processReturnValue( int, QVariant )) );
    connect( client, SIGNAL(failed( int, int, QString )),
             this, SLOT(processFault( int, int, QString )) );
}

TestClient::~TestClient()
{
}

void TestClient::sendRequest()
{
    client->setHost( ui.host->text(), ui.port->value() );
    
    int x = ui.x->text().toInt();
    int y = ui.y->text().toInt();

    requestIdSum = client->request( "sum", x, y );
    requestIdDiff = client->request( "difference", x, y );
    qDebug() << client->request("SetQuestion", "modo", "rest");
}

void TestClient::processReturnValue( int requestId, QVariant value )
{
    Q_ASSERT( value.canConvert( QVariant::Int ) );
    
    if ( requestId == requestIdSum )
        ui.sum->setText( QString::number( value.toInt() ) );
    
    if ( requestId == requestIdDiff )
        ui.difference->setText( QString::number( value.toInt() ) );
}

void TestClient::processFault( int requestId, int errorCode, QString errorString )
{
    Q_UNUSED( requestId );

    QMessageBox::warning(this, tr("Request failed"),
            QString("XML-RPC request  failed.\n\nFault code: %1\n%2\n") \
            .arg(errorCode).arg(errorString),
            QMessageBox::Ok );

}


