/***************************************************************************
 *   Copyright (C) 2007 by Oscar Campos Ruiz-Adame                         *
 *   oscar.campos@edmanufacturer.es                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "MonitorServer.h"
#include "Common.h"
#include "cclient.h"
#include "Database.h"
#include "clientmanager.h"
#include "groupmanager.h"
#include "TaskManager.h"
#include "osdwidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QDate>
#include <iostream>

extern CDatabase BDatabase;

MonitorServer::MonitorServer(int port, QObject *parent)
 : QObject(parent)
{
     server = new xmlrpc::Server(this);	

     // Register Methods
     server->registerMethod( "protocolRevision", QVariant::Int, QVariant::Int/*, QVariant::Int*/ );
     server->registerMethod( "identification", QVariant::Int, QVariant::String );
	server->registerMethod( "clientInformation", QVariant::Int, QVariant::String, QVariant::String );	
	server->registerMethod( "setHardwareList", QVariant::Int, QVariant::Map );
	server->registerMethod( "status", QVariant::Int, QVariant::String );
	server->registerMethod( "pong", QVariant::Int, QVariant::String );

     connect( server, SIGNAL(incomingRequest( int, QString, QList<xmlrpc::Variant>)),
              this, SLOT(processRequest( int, QString, QList<xmlrpc::Variant>)));

     if( server->listen( port ) ) {
#ifdef __MONITOR_DEBUG       
          qDebug() << "MonitorServer Listening for XML-RPC requests on port" << port;
#endif
     } 
     else {
#ifdef __MONITOR_DEBUG
          qDebug() << "MonitorServer Error listening port" << port;
#endif
          QMessageBox::warning(NULL, tr("Backharddi Net Monitor"), tr("MonitorServer can't start to listen for XML-RPC request."), QMessageBox::Ok | QMessageBox::Escape); 
		QApplication::instance()->quit();
     }
}


MonitorServer::~MonitorServer()
{
}

void MonitorServer::processRequest( int requestId, QString methodName, QList<xmlrpc::Variant> parameters )
{

#ifdef __MONITOR_DEBUG
     qDebug() << QString("Received XML-RPC packet methodName ").append(methodName);
#endif

     if ( methodName == "protocolRevision" ) {
          int clientProtocol = parameters[0].toInt();
          if(clientProtocol != MONITOR_VERSION) {
               qDebug( "MonitorServer Debug: Client protocol %d  Server [%d].", 
                     clientProtocol, MONITOR_VERSION );

               server->sendReturnValue( requestId, ERR_PROTOCOL_MISMATCH );
          }
          else { 
               server->sendReturnValue( requestId, ERR_NO_ERROR );
          }
     }
     else if( methodName == "identification" ) {
          QString clientIdent = parameters[0].toString();
          if(clientIdent.isEmpty()) {
#ifdef __MONITOR_DEBUG
               qDebug() << "MonitorServer Debug: Client sent empty identification request.";
#endif
               server->sendReturnValue( requestId, ERR_NAME_EMPTY );
			return;
          }

          if(clientIdent.size() < MIN_CLIENT_NAME_LENGTH) {
#ifdef __MONITOR_DEBUG
               qDebug() << "MonitorServer Debug: Client sent a too short identification.";
#endif
               server->sendReturnValue( requestId, ERR_NAME_TOO_SHORT );
			return;
          }

          if(clientIdent.size() > MAX_CLIENT_NAME_LENGTH) {
#ifdef __MONITOR_DEBUG
               qDebug() << "MonitorServer Debug: Client sent a too large identification.";
#endif
               server->sendReturnValue( requestId, ERR_NAME_TOO_LONG );
			return;
          }

		CClient *tmpClient;
	  	if((tmpClient = ClientManager::instance()->findClient(QByteArray(clientIdent.toStdString().c_str()).toBase64())) != NULL) {
	     	if(tmpClient->getStatus(CLIENT_STATUS_DISCONNECTED)) {
				tmpClient->setStatus(CLIENT_STATUS_DISCONNECTED, false);
		     	tmpClient->setStatus(CLIENT_STATUS_CONNECTED, true);
		     	tmpClient->setStatus(CLIENT_STATUS_WAITING, true);

				QString clientIp = server->getClientAddress(requestId);
		     	tmpClient->setInformation("ip_address", clientIp);

				QTreeWidgetItem *item = ClientManager::instance()->getClientTreeItem(tmpClient);
				
				if(item != NULL) {
					TaskManager::instance()->redrawTask();
				}
				else {
					QListWidgetItem *item = ClientManager::instance()->getClientItem(tmpClient);
					if(item) {
						item->setTextColor(QColor::fromRgb(50, 205, 50));
					}
				}
	       	}
               server->sendReturnValue( requestId, ERR_NO_ERROR );
			
			// Update OSD information
			OSDWidget *osd = ClientManager::instance()->getClientOsd( tmpClient );
			if( osd ) 
				osd->setText( tmpClient->getFakeName() + " " + tr( "is Connected and Waiting for instructions." ) );	
			
			// Connect the timer events
			connect(tmpClient->getTimer(), SIGNAL(timeout()), tmpClient, SLOT(ping()));
			
			CGroup *tmpGroup = NULL;
			// Check if the client is in a configured group and if that group is already throwed
			if(tmpClient->getGroup() != NULL) {
				tmpGroup = GroupManager::instance()->findGroup(tmpClient->getGroup()->getId());
			}
			
			if(tmpGroup) {
				if(tmpGroup->getStatus(CGroup::STATUS_THROWED)) {
					MonitorClient *xmlRpcClient = new MonitorClient(this);
					xmlRpcClient->sendRequest(tmpClient->getInformation("ip_address"), 7776, tmpGroup);
					delete xmlRpcClient;
			
					tmpGroup->setNumThrowedClients(tmpGroup->getThrowedClients() +1);
				}
			}

               return;
          }

		QString errMsg;
		CClient *newClient = ClientManager::instance()->createClient(clientIdent, ClientManager::toBase64(clientIdent), QDate::currentDate(), clientIdent, errMsg, false);
		
	  	if(!newClient) {
#ifdef __MONITOR_DEBUG
			qDebug() << "MonitorServer Debug: " << errMsg;
#endif
			return;
	  	}
		
          if( newClient->addCLientToMap() == ERR_CLIENT_ALREADY_ON_MAP ) {
#ifdef __MONITOR_DEBUG
               qDebug() << "MonitorServer Debug: Server tried to add Client " << newClient->getFakeName() << " but key is already at map.";
#endif

               // We tract this error on server side, clients doesn't need to know about it.
               // The client can exist on memory before real clients disconnects or reboot.
               server->sendReturnValue( requestId, ERR_NO_ERROR );
			
			ClientManager::instance()->deleteClient(newClient, errMsg);
               return;
          }
		
		newClient->setStatus(CLIENT_STATUS_DISCONNECTED, false);
		newClient->setStatus(CLIENT_STATUS_CONNECTED, true);
		newClient->setStatus(CLIENT_STATUS_WAITING, true);
		QString clientIp = server->getClientAddress(requestId);
		newClient->setInformation("ip_address", clientIp);
		
		if(ClientManager::instance()->getClientTreeItem(newClient) != NULL)
			ClientManager::instance()->getClientTreeItem(newClient)->setText(2, newClient->getStatusFlagCode().join("-"));
		
		server->sendReturnValue( requestId, ERR_NO_ERROR );
		
		// Connect the timer events
		connect(newClient->getTimer(), SIGNAL(timeout()), newClient, SLOT(ping()));
     }
     else if( methodName == "clientInformation" ) {
          QString clientId = parameters[0].toString();	

          CClient *tmpClient = ClientManager::instance()->findClient(ClientManager::toBase64(clientId));
		if(tmpClient)
		{
               QString key = parameters[1].toString();
               QString val = parameters[2].toString();
			
			tmpClient->setInformation(key, val);
          }
          else {
#ifdef __MONITOR_DEBUG
               qDebug() << "MonitorServer Debug: Received clientInformation request from " << clientId.data() << " but is not into the clients map ?????";
#endif
          }
          
          return;
     }
	else if( methodName == "setHardwareList" ) {	
		QMap<QString, QVariant> structMap = parameters[0].toMap();
		if (!parseHardwareMap(structMap))
			server->sendReturnValue( requestId, ERR_CLIENT_UNKNOWN );
		else
			server->sendReturnValue( requestId, ERR_NO_ERROR );
	}
	else if( methodName == "status" ){
		if(!TaskManager::instance()->currentTask()) {
			server->sendReturnValue( requestId, ERR_NO_ERROR );
			return;
		}
		QString statusParams = parameters[0].toString();
		QStringList statusList = statusParams.split("<<");
		
		QString errMsg;
		int numParams = statusList.count();
		
		if(numParams >= 2) 
			errMsg = statusList.at(1);
		
		
		if(numParams == 0) {
#ifdef __MONITOR_DEBUG
			qDebug() << "MonitorServer Debug; Received an empty status packet!!!";
#endif
		}
		else {
			CClient *tmpClient;
			
			switch(statusList.at(0).toInt()) {
				case CLIENT_STATUS_JUST_ERROR:
					if((tmpClient = ClientManager::instance()->findClientByIP(server->getClientAddress(requestId))) != NULL) {
						tmpClient->setStatus(CLIENT_STATUS_JUST_ERROR, true);
						tmpClient->setStatus(CLIENT_STATUS_MASTERING, false);
						tmpClient->setStatus(CLIENT_STATUS_UPLOADING, false);
						tmpClient->setStatus(CLIENT_STATUS_WAITING, false);
						
						if(tmpClient->getGroup() != NULL) {
							tmpClient->getGroup()->completeClient();
							tmpClient->getGroup()->addFailedClientToMap(tmpClient);
						}
							
							// Update OSD information
						OSDWidget *osd = ClientManager::instance()->getClientOsd( tmpClient );
						if( osd ) {
							osd->setText( tmpClient->getFakeName() + " " + tr( "FAILED" ) );
							osd->setProgress( 0 );
						}
					}
					
					break;
				
				case CLIENT_STATUS_JUST_DONE:
					if((tmpClient = ClientManager::instance()->findClientByIP(server->getClientAddress(requestId))) != NULL) {
						tmpClient->setStatus(CLIENT_STATUS_JUST_DONE, true);
						tmpClient->setStatus(CLIENT_STATUS_MASTERING, false);
						tmpClient->setStatus(CLIENT_STATUS_UPLOADING, false);
						tmpClient->setStatus(CLIENT_STATUS_WAITING, false);

						// Update OSD information
						OSDWidget *osd = ClientManager::instance()->getClientOsd( tmpClient );
						if( osd ) {
							osd->setText( tmpClient->getFakeName() + " " + tr( "Success" ) );
							osd->setProgress( 0 );
						}

						GroupManager::instance()->completeGroupClient(tmpClient);
					}
					
					break;

				
				case CLIENT_STATUS_MASTERING:
					if((tmpClient = ClientManager::instance()->findClientByIP(server->getClientAddress(requestId))) != NULL) {
						tmpClient->setStatus(CLIENT_STATUS_UPLOADING, false);
						tmpClient->setStatus(CLIENT_STATUS_WAITING, false);
						tmpClient->setStatus(CLIENT_STATUS_MASTERING, true);
						
						if(!errMsg.isEmpty()) {
							QString percentage = tr("Percent done :");
							tmpClient->setToolTip(" " + errMsg + " \n\n " + percentage + statusList.at(2) + "%\n");

							// Update OSD information
							OSDWidget *osd = ClientManager::instance()->getClientOsd( tmpClient );
							if( osd ) {
								osd->setText( tmpClient->getFakeName() + " " + errMsg );
								osd->setProgress( statusList.at( 2 ).toInt() );
							}

							GroupManager::instance()->osdUpdateProgress(tmpClient->getGroup());
						}
					}

					break;
					
				case CLIENT_STATUS_UPLOADING:
					if((tmpClient = ClientManager::instance()->findClientByIP(server->getClientAddress(requestId))) != NULL) {
						if(!tmpClient->getStatus(CLIENT_STATUS_UPLOADING)) {
							tmpClient->setStatus(CLIENT_STATUS_UPLOADING, true);
							tmpClient->setStatus(CLIENT_STATUS_WAITING, false);
							tmpClient->setStatus(CLIENT_STATUS_MASTERING, false);
						}
								
						if(!errMsg.isEmpty()) {
							QString percentage = tr("Percent done :");
							tmpClient->setToolTip(" " + errMsg + " \n\n " + percentage + statusList.at(2) + "%\n");

							// Update OSD information
							OSDWidget *osd = ClientManager::instance()->getClientOsd( tmpClient );
							if( osd ) {
								osd->setText( tmpClient->getFakeName() + " " + errMsg );
								osd->setProgress( statusList.at( 2 ).toInt() );
							}

							GroupManager::instance()->osdUpdateProgress(tmpClient->getGroup());
						}
					}
					
					break;
				
				default:
#ifdef __MONITOR_DEBUG
					qDebug() << "MonitorServer Debug: Server received an unhandled error code at opcode status.";
#endif
					server->sendReturnValue( requestId, ERR_NO_ERROR );
					return;
			}
			
			// Redraw the taskstree widget
			TaskManager::instance()->redrawTask();
		}
		
		server->sendReturnValue( requestId, ERR_NO_ERROR );
	}
	else if(methodName == "pong") {
		QString msg = parameters[0].toString();
		
		if(msg.isEmpty())
			return;
		
#ifdef __MONITOR_DEBUG
		qDebug() << QDateTime::currentDateTime() << ": Received PONG response from " << server->getClientAddress(requestId);
#endif
		
		server->sendReturnValue( requestId, ERR_NO_ERROR );
	}
}

bool MonitorServer::parseHardwareMap(QMap<QString, QVariant> structMap)
{
	if(structMap.isEmpty()) {
#ifdef __MONITOR_DEBUG
		qDebug() << "MonitorServer Debug: parseHardwareMap received an empty map, aborting.";
#endif
		return false;
	}
	
	QString clientID = structMap["identification"].toString();
	if( ClientManager::instance()->findClient(ClientManager::toBase64(clientID)) == NULL) {
#ifdef __MONITOR_DEBUG
		qDebug() << "MonitorServer Debug: Received parseHardwareMap request from client " << clientID << " but is not into the clients map ?????";
#endif
		return false;
	}
	
#ifdef __MONITOR_DEBUG
	qDebug() << "Fijando hardware list para cliente " << clientID << "  con nombre " << ClientManager::instance()->findClient(ClientManager::toBase64(clientID))->getFakeName();
#endif
	
	QMap<QString, QVariant>::iterator it = structMap.begin();
	while( it != structMap.end() ) {
		ClientManager::instance()->findClient(ClientManager::toBase64(clientID))->setInformation(it.key(), it.value().toString());
		it++;
	}
	
	QMap<QString, QVariant> cpu = structMap["cpu"].toMap();
		
	ClientManager::instance()->findClient(ClientManager::toBase64(clientID))->setInformation("processor", cpu["processor"].toString());	
	ClientManager::instance()->findClient(ClientManager::toBase64(clientID))->setInformation("cores", QString(cpu["cores"].toList().count()));
	ClientManager::instance()->findClient(ClientManager::toBase64(clientID))->setInformation("display", structMap["display:0"].toString());	
	
	QMap<QString, QVariant> memory = structMap["memory"].toMap();	
	ClientManager::instance()->findClient(ClientManager::toBase64(clientID))->setInformation("memory", memory["description"].toString());
	
	QMap<QString, QVariant> disk;
	QString myDisk;
	for(int j = 0; j < structMap["num_disks"].toInt(); j++) {
		switch(j) {
			case 0:
				disk = structMap["disk0"].toMap();	
				myDisk = "disk0";
				break;
			case 1:
				disk = structMap["disk1"].toMap();	
				myDisk = "disk1";
				break;
			case 2:
				disk = structMap["disk2"].toMap();	
				myDisk = "disk2";
				break;
			case 3:
				disk = structMap["disk3"].toMap();	
				myDisk = "disk3";
				break;
			case 4:
				disk = structMap["disk4"].toMap();	
				myDisk = "disk4";
				break;
		}
		ClientManager::instance()->findClient(ClientManager::toBase64(clientID))->setInformation(myDisk, disk["model"].toString());
	}
	
	ClientManager::instance()->findClient(ClientManager::toBase64(clientID))->setInformation("num_disks", structMap["num_disks"].toString());
	
	QString name, error;
	name = structMap["core"].toString() + "-" + cpu["processor"].toString() + " (" + structMap["identification"].toString().right(2) + ")";

	if(!ClientManager::instance()->renameClient(ClientManager::instance()->findClient(ClientManager::toBase64(clientID)), name, error)) {
		QMessageBox::critical(0, "Backharddi-Net Monitor", tr(error), QMessageBox::Ok);
		return false;
	}
	
	return true;
}


MonitorClient::MonitorClient(QObject *parent) : QObject(parent)
{
}

MonitorClient::~MonitorClient()
{
}

void MonitorClient::sendRequest(QString host, unsigned short int port, CGroup *group)
{	
	if(group->getSettings().isEmpty()) {
		QMessageBox::critical(0, "Backharddi-Net Monitor", tr("Ooops!! Seems like the group's configuration is corrupted, please, edit the group configuration and try again."), QMessageBox::Ok);
		group->setStatus(CGroup::STATUS_THROWED, false);
		return;
	}
	
	char buf[1024];
	QString modo = (group->getSettings().split(",").at(0) == "0") ? "gen" : "rest";
	QString imagen = group->getSettings().split(",").at(1);
	QString bmodo = "backharddi/modo";
	QString bimagen = "backharddi/imagenes";
	QString minclient = "backharddi/net/minclients";
	
	this->host = host;
	this->port = port;
	
	openPipe(QString("SetQuestion"), bmodo + "," + modo);
	readFromPipe(pDescriptor, buf);
	pclose(pDescriptor);
	qDebug() << buf;
	
	openPipe(QString("SetQuestion"), bimagen + "," + imagen.replace("/tmp/backharddi-ng/", "/target/"));
	readFromPipe(pDescriptor, buf);
	pclose(pDescriptor);
	qDebug() << buf;
	
	QString numClients;
	int count = 0;
	for(int i = 0; i < group->getAllClientsFromMap().count(); ++i) {
		CClient *tmpClient = group->getClientFromMap(group->getAllClientsFromMap().keys().at(i));
		if(tmpClient) {
			if(tmpClient->getStatus(CLIENT_STATUS_CONNECTED))
				count++;
		}
	}
	
	if(count == 0) {
		QMessageBox::critical(0, "Backharddi-Net Monitor", tr("Ooops!! Seems like there is no client connected to this group."), QMessageBox::Ok);
		return;
	}
	
	numClients = QString::number(count);	
	openPipe(QString("SetQuestion"), minclient + "," + numClients);
	readFromPipe(pDescriptor, buf);
	pclose(pDescriptor);
	
	group->setStatus(CGroup::STATUS_THROWED, true);
}

void MonitorClient::sendReboot(QString host, unsigned short int port, CGroup *group)
{
	Q_UNUSED(group);
	char buf[1024];
	
	this->host = host;
	this->port = port;
	
	sprintf(buf, "bxmlclient-ng %s %d Reboot \"i 0\"", this->host.toStdString().c_str(), this->port);		
	if((pDescriptor = popen(buf, "r")) == NULL) {
#ifdef __MONITOR_DEBUG
		qDebug() << "Error: Error opening the pipe to call bxmlclient-ng.";
#endif
		return;
	}
	
	char c;
	char ret[256];
	int lSize = 0;
	do {
		c = fgetc(pDescriptor);
		ret[lSize] = c;
		lSize++;
	} while (c != EOF);
	
	ret[lSize -1] = '\0';
	
	qDebug() << ret;
}

bool MonitorClient::pingClient(QString host, unsigned short int port)
{
	char buf[1024];
	
	this->host = host;
	this->port = port;	
	
	sprintf(buf, "bxmlclient-ng %s %d Pong \"s PONG\"", this->host.toStdString().c_str(), this->port);
	if((pDescriptor = popen(buf, "r")) == NULL) {	
#ifdef __MONITOR_DEBUG
		qDebug() << "Error: Error opening the pipe to call bxmlclient-ng.";
#endif
		return false;
	}
	
	char c;
	char ret[256];
	int lSize = 0;
	do {
		c = fgetc(pDescriptor);
		ret[lSize] = c;
		lSize++;
	} while (c != EOF);
	
	ret[lSize -1] = '\0';	
	
	if(QString(ret).left(2) == QString("ok"))
		return true;
		
	return false;
}

void MonitorClient::readFromPipe(FILE *fd, char *buf)
{
	char c;	
	int lSize = 0;
	if(fd == NULL) {
#ifdef __MONITOR_DEBUG
		qDebug() << "Error from pipe.";
#endif
		return;
	}
	
	do {
		c = fgetc(fd);
		buf[lSize] = c;
		lSize++;
	} while (c != EOF);
	
	buf[lSize -1] = '\0';
}

void MonitorClient::openPipe(QString cmd, QString params)
{
	QString xmlCmd = QString("bxmlclient-ng %1 %2 %3 's %4,s %5'").arg(host, QString::number(port), cmd, params.split(",").at(0), params.split(",").at(1));
	if((pDescriptor = popen(xmlCmd.toStdString().c_str(), "r")) == NULL) {
#ifdef __MONITOR_DEBUG
		qDebug() << "Error: Error opening the pipe to call bxmlclient-ng.";
#endif
		return;
	}
}

