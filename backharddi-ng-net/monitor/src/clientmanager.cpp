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
#include "clientdatabase.h"
#include "clientmanager.h"
#include "MonitorServer.h"
#include "clientslist.h"
#include "cclient.h"
#include "Loader.h"

#include <QDate>
#include <QListWidgetItem>

class ClientManagerPriv
{
	public:
		ClientDatabase *clientDB;
		
		bool			modified;
		
		PClientMap	pClientMap;
		PClientOsdMap	pClientOsdMap;
		PClientItemMap pClientItemMap;
		PClientTreeItemMap pClientTreeItemMap;
		QList<QListWidgetItem *> pCurrent;
};

ClientManager *ClientManager::m_instance = 0;

ClientManager* ClientManager::instance()
{
	return m_instance;
}

ClientsList *ClientManager::getGraphicList()
{
	return clientsListWidget;
}

void ClientManager::setGraphicList(QWidget *parent)
{
	clientsListWidget = new ClientsList(parent);
}

ClientManager::ClientManager()
{
	m_instance     = this;
	d = new ClientManagerPriv;
	d->clientDB = new ClientDatabase();
	d->modified = false;
	clientsListWidget = NULL; 	
}

ClientManager::~ClientManager()
{
	delete d->clientDB;
	d->pClientMap.clear();
	d->pClientOsdMap.clear();
	delete d;

	m_instance = 0;
}

CClient* ClientManager::findClient(QByteArray clientId)
{	
	if(d->pClientMap.contains(clientId))
		return d->pClientMap.value(clientId);
	else 
		return NULL;	
}

CClient* ClientManager::findClient(QString clientName)
{	
	for(int i=0; i < d->pClientMap.values().size(); ++i) 
	{
		if(d->pClientMap.values().at(i)->getName() == clientName)
			return d->pClientMap.values().at(i);
	}

	return NULL;
}

CClient* ClientManager::findClientByIP(QString clientIP)
{
	for(int i=0; i < d->pClientMap.values().size(); ++i) 
	{
		if(d->pClientMap.values().at(i)->getInformation("ip_address") == clientIP)
			return d->pClientMap.values().at(i);
	}

	return NULL;
}

CClient* ClientManager::createClient(const QString &name, const QString &id, const QDate &date, const QString &mac, QString &errMsg, bool insert)
{
	if(name.isEmpty())
	{
		errMsg = tr("Client name cannot be empty.");
		return NULL;
	}
	
	if(id.isEmpty())
	{
		errMsg = tr("Client identification cannot be empty.");
	}

	if(name.contains("[',;--]"))
	{
		errMsg = tr("You used illegal characters on name.");
		return NULL;
	}

	PClientMap::iterator i = d->pClientMap.begin();	
	while(i != d->pClientMap.end()) 
	{
		if( (*i)->getName() == mac )
		{
			errMsg = tr("An existing client has the same MAC Address.");
			return NULL;
		}
		i++;
	}

	CClient *newClient = new CClient();
	newClient->setInformation("name", name);
	newClient->setName(mac);
	newClient->setStatus(CLIENT_STATUS_DISCONNECTED, true);
	newClient->setID(QByteArray(id.toStdString().c_str()));
	newClient->setInformation("mac", mac);
	newClient->setInformation("date", date.toString("yyyy-MM-dd"));
	d->clientDB->addClient(name, date, id, mac, errMsg);	
	
	if(insert)
		insertClient(newClient);
	
	// Create new OSD for this client
	OSDWidget *osd = new OSDWidget();
	osd->setText( newClient->getFakeName() );
	osd->setType( OSDWidget::Information );
	osd->setProgress( 0 );
	d->pClientOsdMap[newClient->getName()] = osd;

	return newClient;
}

bool ClientManager::renameClient(CClient *client, const QString &newName, QString &errMsg)
{
	if(!client) {
		errMsg = tr("The client pointer is not a valid pointer.");
		return false;
	}

	if(newName.isEmpty()) {
		errMsg = tr("The client new name can't be empty.");
		return false;
	}

	QString tmpName = newName;	

	QString oldName = client->getFakeName();
	if(oldName != tmpName) {
		client->setFakeName(newName);
		d->clientDB->setClientName(client->getID(), newName, errMsg);	
	
		setupClientName(oldName, client);
		drawClient(client);
	}

	return true;
}

bool ClientManager::deleteClient(CClient *client, QString &errMsg)
{
	if(!client) {
		errMsg = tr("The client pointer is not a valid pointer.");
		return false;
	}

	if(!d->pClientMap.contains(client->getID())) {     
		errMsg = tr("The client doesn't exist on client list.");
		return false;
	}
	
	if(findClient(client->getID()) != NULL) {
		QByteArray key = client->getID();
		d->pClientMap.remove(key);	
	}

	emit(signalClientDeleted(client));
	delete client;

	return true;
}

PClientMap::iterator ClientManager::getClientIterator(CClient *client)
{
	if(!client) 
		return d->pClientMap.end();

	PClientMap::iterator i = d->pClientMap.begin();	
	while(i != d->pClientMap.end()) {
		if( (*i)->getID() == client->getID() ) {
			return i;
		}
		i++;
	}

	return d->pClientMap.end();
}

OSDWidget *ClientManager::getClientOsd(const CClient *client)
{
	if( client == NULL )
		return NULL;
	
	if( d->pClientOsdMap.contains( client->getName() ) ) {
		return d->pClientOsdMap[client->getName()];
	}
	
	return NULL;
}

void ClientManager::addOsdToMap( const CClient *client, OSDWidget *osd )
{
	if( client && osd ) {
		if( !d->pClientOsdMap.contains( client->getName() ) ) 
			d->pClientOsdMap[client->getName()] = osd;
	}
}

void ClientManager::removeOsdFromMap( const CClient *client )
{
	if( client ) {
		if( !d->pClientOsdMap.contains( client->getName() ) )
			d->pClientOsdMap.remove( client->getName() );
	}
}

bool ClientManager::osdContains( const QString &name )
{
	if( !name.isEmpty() ) {
		return d->pClientOsdMap.contains( name );
	}
	
	return false;
}

bool ClientManager::osdContains( const CClient *client )
{
	if( client ) {
		return d->pClientOsdMap.contains( client->getName() );
	}
	
	return false;
}

void ClientManager::loadClient(QString &error, QString clientId)
{
	if(findClient(toBase64(clientId)) != NULL)
		  return;

	d->clientDB->loadClientFromDB(error, clientId);
}

void ClientManager::insertClient(CClient *client) 
{
	d->pClientMap.insert(client->getID(), client);	
	client->setStatus(CLIENT_STATUS_DISCONNECTED, true);	
	
	emit(signalClientAdded(client));
}

QByteArray ClientManager::toBase64(QString string)
{
	QByteArray base64 = QByteArray(string.toStdString().c_str()).toBase64();
	return base64;
}

void ClientManager::drawClient(CClient *client)
{	
	emit(signalDrawClient(client));	
}

void ClientManager::setupClientName(const QString &key, CClient *client)
{	
	if(!client)
		return;

	Q_UNUSED(key);
	
	QListWidgetItem *item = d->pClientItemMap.find(client->getName()).value();	
	
	item->setText(client->getFakeName());	
	
	setupClientTooltip(client);
	
	if(getClientTreeItem(client) != NULL) {
		if(!d->pClientTreeItemMap.contains(client->getName()))
			return;
		
		QTreeWidgetItem *titem = d->pClientTreeItemMap.find(client->getName()).value();
		titem->setText(1, client->getFakeName());
	}
}

void ClientManager::setupClientTooltip(CClient *client)
{
	QString tooltip;
	QString tmpDisks = "";
	
	for(int i = 0; i < client->getInformation("num_disks").toInt(); i++) {
		switch (i) {
			case 0:
				tmpDisks += "<b>" + tr("Disk") + ":</b> " + client->getInformation("disk0") + "<br />";
				break;
			case 1:
				tmpDisks += "<b>" + tr("Disk") + ":</b> " + client->getInformation("disk1") + "<br />";
				break;
			case 2:
				tmpDisks += "<b>" + tr("Disk") + ":</b> " + client->getInformation("disk2") + "<br />";
				break;
			case 3:
				tmpDisks += "<b>" + tr("Disk") + ":</b> " + client->getInformation("disk3") + "<br />";
				break;
			case 4:
				tmpDisks += "<b>" + tr("Disk") + ":</b> " + client->getInformation("disk4") + "<br />";
				break;
		}
	}	
			
	tooltip = "<b>" + tr("Motherboard") + ":</b> " + client->getInformation("core") + "<br />" + "<b>" + tr("Processor:") + "</b> " + client->getInformation("processor") + "<br />" + "<b>VGA:</b> " + client->getInformation("display") + "<br />" + tr("<b>RAM:</b>") + client->getInformation("memory") + "<br />" + "<b>" + tr("Disks number:") + "</b> " + client->getInformation("num_disks") + "<br />" + tmpDisks + "<br />" + "<b>" + tr("IP Address:") + "</b> " + client->getInformation("ip_address"); 	
	
	getClientItem(client)->setToolTip(tooltip);
}

void ClientManager::setCurrent(QList<QListWidgetItem *> clients)
{
	d->pCurrent = clients;
}

void ClientManager::rebootClient(CClient *client)
{
	if(!client) {
#ifdef __DEBUG_MONITOR
		qDebug() << "Can not reboot the client because the client pointer is null";
#endif
	}
	
	if(client->getStatus(CLIENT_STATUS_CONNECTED)) {
		MonitorClient *xmlRpcClient = new MonitorClient(this);
		xmlRpcClient->sendReboot(client->getInformation("ip_address"), 7776, client->getGroup());
		delete xmlRpcClient;
	}
}

void ClientManager::pingClient(CClient *client)
{
	if(!client) {
#ifdef __DEBUG_MONITOR
		qDebug() << "Can not ping the client because the client pointer is null";
#endif
		return;
	}

	
	if(client->getStatus(CLIENT_STATUS_CONNECTED)) {
		MonitorClient *xmlRpcClient = new MonitorClient(this);
		if(!xmlRpcClient->pingClient(client->getInformation("ip_address"), 7776))
			client->clientDisconnect();
		
		delete xmlRpcClient;
	}
}

QList<QListWidgetItem *> ClientManager::getCurrent() const
{
	return d->pCurrent;
}

PClientMap ClientManager::getClientsMap()
{
	return d->pClientMap;
}

QTreeWidgetItem *ClientManager::getClientTreeItem( const CClient *client )
{
	if( client ) {	
		if( d->pClientTreeItemMap.contains( client->getName() ) ) {
			return d->pClientTreeItemMap[client->getName()];
		}
	}
	
	return NULL;
}

QListWidgetItem *ClientManager::getClientItem( const CClient *client )
{	
	if( client != NULL ) {
		if( d->pClientItemMap.contains( client->getName() ) ) {
			return d->pClientItemMap[client->getName()];
		}
	}
	
	return NULL;
}

void ClientManager::addItemToMap( bool tree, const CClient *client, QTreeWidgetItem *treeitem, QListWidgetItem *listitem)
{
	if( client ) {
		if( !tree ) {
			if( !d->pClientItemMap.contains( client->getName() ) )
				d->pClientItemMap[client->getName()] = listitem;
		}
		else {
			if( !d->pClientTreeItemMap.contains( client->getName() ) )
				d->pClientTreeItemMap[client->getName()] = treeitem;
		}
	}
}

void ClientManager::removeItemFromMap( bool tree, const CClient *client )
{
	if( client ) {
		if( !tree ) {
			if( d->pClientItemMap.contains( client->getName() ) )
				d->pClientItemMap.remove( client->getName() );
		}
		else {
			if( d->pClientTreeItemMap.contains( client->getName() ) ) 
				d->pClientTreeItemMap.remove( client->getName() );
		}
	}
}

bool ClientManager::itemContains( bool tree, const CClient *client )
{
	if( client ) {
		if( !tree )
			return d->pClientItemMap.contains( client->getName() );
		else
			return d->pClientTreeItemMap.contains( client->getName() );
	}
	
	return false;
}

bool ClientManager::itemContains( bool tree, const QString &name )
{
	if( !tree )
		return d->pClientItemMap.contains( name );
	else
		return d->pClientTreeItemMap.contains( name );
}



