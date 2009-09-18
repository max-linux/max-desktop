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
#include "groupmanager.h"
#include "groupdatabase.h"
#include "TaskManager.h"
#include "MonitorServer.h"
#include "clientmanager.h"
#include "cgroup.h"
#include "cclient.h"
#include "osdwidget.h"

#include <QDate>

class GroupManagerPriv 
{
	public:
		GroupDatabase  *groupDB;
		
		bool			modified;
		
		PGroupMap		pGroupMap;
		PGroupOsdMap   pGroupOsdMap;
		PGroupItemMap	pGroupItemMap;
};

GroupManager *GroupManager::m_instance = 0;

GroupManager *GroupManager::instance()
{
	return m_instance;
}

GroupManager::GroupManager(QObject *parent) : QObject(parent)
{
	m_instance = this;
	d = new GroupManagerPriv;
	d->groupDB = new GroupDatabase();
	d->modified = false;
}

GroupManager::~GroupManager()
{
	delete d->groupDB;
	d->pGroupMap.clear();
	d->pGroupOsdMap.clear();
	d->pGroupItemMap.clear();
	delete d;
	
	m_instance = 0;
}

CGroup* GroupManager::findGroup(int groupId)
{	
	if(d->pGroupMap.contains(groupId))
		return d->pGroupMap.value(groupId);
	else 
		return NULL;	
}

CGroup* GroupManager::findGroup(QString groupName)
{	
	for(int i=0; i < d->pGroupMap.values().size(); ++i) 
	{
		if(d->pGroupMap.values().at(i)->getName() == groupName)
			return d->pGroupMap.values().at(i);
	}
     
	return NULL;
}

CGroup* GroupManager::createGroup(const QString &name, const QDate &date, unsigned int idtask, QString &errMsg, int groupid, bool configured)
{
	if(name.isEmpty())
	{
		errMsg = tr("Group name cannot be empty.");
		return NULL;
	}

	if(name.contains("[',;--]"))
	{
		errMsg = tr("You used illegal characters on name.");
		return NULL;
	}

	PGroupMap::iterator i = d->pGroupMap.begin();	
	while(i != d->pGroupMap.end()) 
	{
		if( (*i)->getName() == name )
		{
			errMsg = tr("An existing group has the same name.");
			return NULL;
		}
		i++;
	}	

	CGroup *newGroup = new CGroup();
	newGroup->setName(name);
	newGroup->setTask(idtask);	
	newGroup->setDate(date.toString("yyyy-MM-dd"));
	if(configured)
		newGroup->setConfigured(true);
	
	if( groupid > -1 ) 
		newGroup->setId( groupid );
	else {
		newGroup->setId(d->groupDB->addGroup(name, date, idtask, errMsg));	
	
		if(!errMsg.isEmpty()) {
			qDebug() << errMsg;
			delete newGroup;
			return NULL;
		}
	}

	insertGroup(newGroup);
	
	// Emit signal to get Group name colorized
	emit signalGroupRenamed(newGroup);
	
	// Create the new OSD Object for this group 
	OSDWidget *osd = new OSDWidget();
	osd->setText( name );
	osd->setType( OSDWidget::Information );
	osd->setProgress( 0 );
	
	d->pGroupOsdMap[newGroup->getId()] = osd;

	return newGroup;
}

bool GroupManager::renameGroup(CGroup *group, const QString &newName, QString &errMsg)
{
	if(!group) {
		errMsg = tr("The group pointer is not a valid pointer.");
		return false;
	}
     
	if(newName.isEmpty()) {
		errMsg = tr("The group new name can't be empty.");
		return false;
	}
     
	QString tmpName = newName;
     
	if(!findGroup(tmpName)) {
		group->setName(newName);
		d->groupDB->setGroupName(group->getId(), group->getName(), errMsg);
		emit signalGroupRenamed(group);
		return true;
	}
	else {
		errMsg = QString("Can't rename group %1 because already exists a group called %2")
				.arg(group->getName(), newName);
		errMsg = tr(errMsg.toStdString().c_str());

		return false;
	}

	return false;  // Will be not reached
}

bool GroupManager::deleteGroup(CGroup *group, QString &errMsg)
{
	if(!group) {
		errMsg = tr("The group pointer is not a valid pointer.");
		return false;
	}

	if(!d->pGroupMap.contains(group->getId())) {     
		errMsg = tr("The group doesn't exist on group list.");
		return false;
	}
	
	if(findGroup(group->getId()) != NULL) 
		d->pGroupMap.remove(group->getId());	
	
	d->groupDB->deleteGroup(group->getId(), errMsg);
	if(!errMsg.isEmpty())
		qDebug() << errMsg;

	emit(signalGroupDeleted(group));
	TaskManager::instance()->removeGroupFromTask(group);
	
	foreach(CClient *client, group->getAllClientsFromMap()) 
		client->setGroup(0);
	
	delete group;

	return true;
}

bool GroupManager::deleteGroup(CGroup *group) 
{
	if(!group)
		return false;
	
	if(!d->pGroupMap.contains(group->getId()))
		return false;
	
	if(group->getStatus(CGroup::STATUS_THROWED))
		return false;
	
	QString error;
	if(!deleteGroup(group, error)) {
		qDebug() << error;
		return false;
	}
	
	return true;
}

PGroupMap::iterator GroupManager::getGroupIterator(CGroup *group)
{
	if(!group) 
		return d->pGroupMap.end();

	PGroupMap::iterator i = d->pGroupMap.begin();	
	while(i != d->pGroupMap.end()) {
		if( (*i)->getId() == group->getId() ) {
			return i;
		}
		i++;
	}
     
	return d->pGroupMap.end();
}

void GroupManager::insertGroup(CGroup *group) 
{
	d->pGroupMap.insert(group->getId(), group);	
	group->setStatus(CGroup::STATUS_LOADED, true);
	group->setStatus(CGroup::STATUS_THROWED, false);
	
	emit(signalGroupAdded(group));
}

QByteArray GroupManager::toBase64(QString string)
{
	QByteArray base64 = QByteArray(string.toStdString().c_str()).toBase64();
	return base64;
}

void GroupManager::drawGroup(CGroup *group)
{
	emit(signalDrawGroup(group));
}

void GroupManager::loadGroup(int groupId, QString &error)
{
	if(groupId <= 0) {
		error = "The group ID is not valid";
		return;
	}
	
	d->groupDB->loadGroupFromDB(groupId, error);
	
}

void GroupManager::loadGroupClients(int groupId, QString &error)
{
	if(groupId <= 0) {
		error = "The group ID is not valid";
		return;
	}
	
	d->groupDB->loadClientsFromGroup(groupId, error);
}

void GroupManager::loadGroupConfig(int groupId, QString &error)
{
	if(groupId <= 0) {
		error = "The group ID is not valid";
		return;
	}
	
	d->groupDB->loadGroupConfigFromDB(groupId, error);
}

void GroupManager::saveGroupConfig(CGroup *group, QString &error)
{
	if(!group) {
		error = "The group is null.";
		return;
	}
	
	QString groupSettings = group->getSettings();
	
	if(group->isConfigured()) 
		d->groupDB->updateGroupConfig(groupSettings, error);
	else
		d->groupDB->saveGroupConfig(groupSettings, error);
}

void GroupManager::saveGroupClientsRelation(CGroup *group)
{
	if(!group)
		return;
	
	for(int i = 0; i < group->getAllClientsFromMap().count(); ++i) {
		d->groupDB->addClientsRelation(group->getId(), QString(group->getClientFromMap(group->getAllClientsFromMap().keys().at(i))->getID()));
	}
}

bool GroupManager::throwGroup(CGroup *group, QString &error)
{
	if(!group) {
		error = tr("Can not throw group because the group pointer is null.");
		return false;
	}
	
	if(!group->getStatus(CGroup::STATUS_THROWED)) {
		group->setStatus(CGroup::STATUS_THROWED, true);
	}
	else {
		error = tr("You can not rethrow an already throwed multicast group.");
		return false;
	}
	
	for(int i = 0; i < group->getAllClientsFromMap().count(); ++i) {
		CClient *tmpClient = group->getClientFromMap(group->getAllClientsFromMap().keys().at(i));
		if(tmpClient->getStatus(CLIENT_STATUS_CONNECTED)) {
			MonitorClient *xmlRpcClient = new MonitorClient(this);
			xmlRpcClient->sendRequest(tmpClient->getInformation("ip_address"), 7776, group);
			delete xmlRpcClient;
			
			group->setNumThrowedClients(group->getThrowedClients() +1);
		}
	}
	
#ifdef __DEBUG_MONITOR
	qDebug() << "Throwed clients " << group->getThrowedClients();
#endif
	
	// Setup the OSDs and Show to us	
	OSDWidget *osd = getGroupOsd( group );
	if( osd != NULL ) {
		QString msg = group->getName() + " " + tr( "throwed out." ) + " " + tr( "Client(s) number: " ) + QString::number( group->getThrowedClients() );
		osd->setText( msg );
		osd->show();
	}	
	
	return true;
}

void GroupManager::updateGroupRelation(CGroup *group, QString clientName)
{
	if(!group) {
#ifdef __DEBUG_MONITOR
		qDebug() << "Can not update the group clients relation because the group pointer is null";
#endif
		return;
	}
	
	d->groupDB->deleteClientsRelation(group->getId(), QString(ClientManager::instance()->findClient(clientName)->getID()));
}

void GroupManager::rebootGroup(CGroup *group)
{
	if(!group) {
#ifdef __DEBUG_MONITOR
		qDebug() << "Can not reboot the group clients because the group pointer is null";
#endif
		return;
	}

	for(int i = 0; i < group->getAllClientsFromMap().count(); ++i) {
		CClient *tmpClient = group->getClientFromMap(group->getAllClientsFromMap().keys().at(i));
		if(tmpClient->getStatus(CLIENT_STATUS_CONNECTED)) {
			MonitorClient *xmlRpcClient = new MonitorClient(this);
			xmlRpcClient->sendReboot(tmpClient->getInformation("ip_address"), 7776, group);
			delete xmlRpcClient;
		}
	}
}

void GroupManager::setConfigured(CGroup *group)
{
	QString error;
	d->groupDB->setConfigured(group, error);
}

OSDWidget *GroupManager::getGroupOsd( const CGroup *group )
{	
	if( group != NULL ) {
		if( d->pGroupOsdMap.contains( group->getId() ) )
			return d->pGroupOsdMap[group->getId()];
	}
	
	return NULL;
}

void GroupManager::addOsdToMap( const CGroup *group, OSDWidget *osd )
{
	if( group && osd ) {
		if( d->pGroupOsdMap.contains( group->getId() ) ) {
			d->pGroupOsdMap[group->getId()] = osd;
		}
	}
}

void GroupManager::removeOsdFromMap( const CGroup *group )
{
	if( group ) {
		if( d->pGroupOsdMap.contains( group->getId() ) ) {
			d->pGroupOsdMap.remove( group->getId() );
		}
	}
}

bool GroupManager::osdContains( const CGroup *group )
{
	if( group ) {
		return d->pGroupOsdMap.contains( group->getId() );
	}
	
	return false;
}

bool GroupManager::osdContains( const unsigned int groupId )
{
	return d->pGroupOsdMap.contains( groupId );	
}

void GroupManager::osdUpdateProgress( const CGroup *group )
{	
	if( !group )
		return;
	
	int clients = group->getThrowedClients();	
	int totalPercent = 0;
	
	foreach( CClient *tmpClient, group->getAllClientsFromMap() ) {
		totalPercent += ClientManager::instance()->getClientOsd(tmpClient)->progress();
	}
	
	d->pGroupOsdMap[group->getId()]->setProgress(totalPercent / clients);
}

void GroupManager::completeGroup(CGroup *group)
{
	if(!group) {
		qDebug() << "Error: GroupManager::completeGroup(CGroup *" << group << "): Invalid client pointer";
		return;
	}
	
	QString msg;
	if(group->GetFailedClientsFromMap().empty()) 
		msg = tr(" Done. All clients success.");
	else
		msg = tr(" Done, Failed Clients: ") + QString::number(group->GetFailedClientsFromMap().count());
	
	d->pGroupOsdMap[group->getId()]->setText( msg );
	d->pGroupOsdMap[group->getId()]->setProgress(0); 
}

CGroup *GroupManager::findGroupByClientName(const QString &clientName)
{
	if(clientName.isEmpty() || clientName.isNull())
		return NULL;

	foreach(CGroup *group, d->pGroupMap) {
		if(group->getClientFromMap(clientName) != NULL)
			return group;
	}

	return NULL;
}

void GroupManager::completeGroupClient(CClient *client)
{
	if(!client) {
		qDebug() << "Error: GroupManager::completeGroupClient(CClient *" << client << "): Invalid client pointer";
		return;
	}

	QString msg, clientName;
	clientName = client->getName();
	
	CGroup *group = findGroupByClientName(clientName);

	if(!group) {
		qDebug() << "Error: GroupManager::completeGroupClient(CClient *" << client << "): Invalid group pointer (" << group << ")" ;
		return;
	}
	
	group->completeClient();	
}

QListWidgetItem *GroupManager::getGroupItem( const CGroup *group )
{
	if( group ) {
		if( d->pGroupItemMap.contains( group->getId() ) ) {
			return d->pGroupItemMap[group->getId()];
		}
	}
	
	return NULL;
}

void GroupManager::addItemToMap( const CGroup *group, QListWidgetItem *item)
{
	if( group ) {
		if( !d->pGroupItemMap.contains( group->getId() ) )
			d->pGroupItemMap[group->getId()] = item;
	}
}

void GroupManager::removeItemFromMap( const CGroup *group )
{
	if( group ) {
		if( d->pGroupItemMap.contains( group->getId() ) ) {
			QListWidgetItem *delItem = d->pGroupItemMap[group->getId()];
			d->pGroupItemMap.remove( group->getId() );
			delete delItem;
		}
	}
}

bool GroupManager::itemContains( const CGroup *group )
{
	if( group ) {
		return d->pGroupItemMap.contains( group->getId() );
	}
	
	return false;
}

bool GroupManager::itemContains( const unsigned int groupId )
{
	return d->pGroupItemMap.contains( groupId );
}



