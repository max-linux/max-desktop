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
#ifndef MonitorServer_h
#define MonitorServer_h

#include "xmlrpc/server.h"
#include "xmlrpc/client.h"
#include "cgroup.h"

#include <QObject>
#include <iostream>

class CClient;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class MonitorServer : public QObject
{
Q_OBJECT
public:
     MonitorServer(int port, QObject *parent = 0);
     virtual ~MonitorServer();
    
private slots:
     void processRequest( int requestId, QString methodName, QList<xmlrpc::Variant> parameters );
		
private:
	bool parseHardwareMap(QMap<QString, QVariant> structMap);
     
private:
     xmlrpc::Server *server;
	
signals:
	void signalClientIdentified(CClient *client);
	void signalClientHardList(CClient *client);
	void clientAdded(CClient *client);

};

class MonitorClient : public QObject
{
Q_OBJECT
public:
	MonitorClient(QObject *parent = 0);
	virtual ~MonitorClient();
	
public slots:
	void sendRequest(QString host, unsigned short int port, CGroup *group);
	void sendReboot(QString host, unsigned short int port, CGroup *group);
	bool pingClient(QString host, unsigned short int port);

private slots:
	void readFromPipe(FILE *fd, char *buf);
	void openPipe(QString cmd, QString params);	
	
private:
	QString host;
	unsigned short int port;
	FILE *pDescriptor;
};

#endif
