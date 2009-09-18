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
#include "cgroup.h"
#include "Database.h"
#include "clientmanager.h"
#include "groupmanager.h"
#include "cclient.h"

extern CDatabase *BDatabase;

//====================================================================
// Default Constructor
//====================================================================
CGroup::CGroup() {
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: CGroup " << this << " constructor called.";
#endif
     setName(QString("<New Group>"));
	m_StatusFlag = new QBitArray(CGroup::MAX_STATES);	
	m_StatusFlag->fill(false, CGroup::MAX_STATES);
	setNumThrowedClients(0);
	m_numClompleteClients = 0;
	
     GroupChanged(FALSE);
     GroupThrowed(FALSE);
	
	connect(this, SIGNAL(signalUpdateGroupRelation(CGroup*, QString)), GroupManager::instance(), SLOT(updateGroupRelation(CGroup*, QString)));
	connect(this, SIGNAL(signalUpdateGroupConfigured(CGroup*)), GroupManager::instance(), SLOT(setConfigured(CGroup*)));
	connect(this, SIGNAL(signalUpdateGroupComplete(CGroup*)), GroupManager::instance(), SLOT(completeGroup(CGroup*)));
}


CGroup::~CGroup()
{
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: CGroup " << getName() << " destructor called.";
#endif
     // Remove all Items on Hash Map if not Empty 
     if(!m_vCLients.empty()) {
#ifdef __MONITOR_DEBUG
          qDebug() << "Debug: CGroup " << getName() << " Clients Map is not empty, erasing it.";
#endif
          m_vCLients.clear();
     }    
}

QString CGroup::getName() const 
{
     return m_Name;
}

void CGroup::setName(QString groupName)
{
#ifdef __MONITOR_DEBUG 
     qDebug() << "Debug: Setting GroupName as " << groupName << ".";  
#endif
     m_Name = groupName;
}

void CGroup::GroupChanged(bool val)
{
#ifdef __MONITOR_DEBUG
     if(val)
       qDebug() << "Debug: Setting GroupChanged value as TRUE.";
     else
       qDebug() << "Debug: Setting GroupChanged value as FALSE.";
#endif
     m_Changed = val;
}

bool CGroup::IsChanged() const
{
     return m_Changed;
}

void CGroup::GroupThrowed(bool val)
{
#ifdef __MONITOR_DEBUG
     if(val)
          qDebug() << "Debug: Setting GroupThrowed value as TRUE.";
     else
          qDebug() << "Debug: Setting GroupThrowed value as FALSE.";
#endif
     m_Throwed = val;
}

bool CGroup::IsThrowed() const 
{
     return m_Throwed;
}

bool CGroup::isGroupOnMap()
{
	return (GroupManager::instance()->findGroup(getId()) != NULL) ? true : false;
}

bool CGroup::addGroupToMap()
{
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: Adding " << getName() << " to groups map.";
#endif     
	if(!isGroupOnMap()) {
		GroupManager::instance()->insertGroup(this);
		return true;
	}
	
	return false;
}

bool CGroup::isClientOnMap(QString clientName)
{
     return m_vCLients.contains(clientName);
}

ERROR_CODE CGroup::addCLientToMap(QString clientName, CClient* newClient)
{     
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: Trying to add " << clientName << " to the group clients map";
#endif
     if( clientName.size() < MIN_CLIENT_NAME_LENGTH ) {
          return ERR_NAME_TOO_SHORT;     
     }
     else if( clientName.size() > MAX_CLIENT_NAME_LENGTH ) {
          return ERR_NAME_TOO_LONG;
     }
     else if( newClient == NULL ) {
          return ERR_CLIENT_NULL;
     }
     else {
          m_vCLients[clientName] = newClient;
          return ERR_NO_ERROR;
     }
}

CClient* CGroup::getClientFromMap(QString clientName) const 
{
  if(m_vCLients.contains(clientName)) {
          return m_vCLients.value(clientName);
     }
     
     return NULL;
}

ERROR_CODE CGroup::removeClientFromMap(QString clientName)
{
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: Removing " << clientName << " from clients map.";
#endif
     if(m_vCLients.remove(clientName) > 0) {
		emit signalUpdateGroupRelation(this, clientName);
          return ERR_NO_ERROR;
     }
     else {
          return ERR_CLIENT_UNKNOWN;
     }
}

void CGroup::addFailedClientToMap(CClient* client)
{
	if(!client)
		return;
	
	if(client->getName().isEmpty())
		return;
	
	if(m_fvClients.contains(client))
		return;
	
	m_fvClients.append(client);
}

void CGroup::removeFailedClientFromMap(CClient* client)
{
	if(!client)
		return;
	
	if(client->getName().isEmpty())
		return;
	
	if(m_fvClients.contains(client)) {
		for(FVClients::iterator it = m_fvClients.begin(); it != m_fvClients.end(); ++it) {
			if((*it) == client)
				m_fvClients.erase(it);
		}
	}
}

FVClients CGroup::GetFailedClientsFromMap()
{
     return m_fvClients;
}

void CGroup::setDate(QString date)
{
     m_CreationDate = date;
}

QString CGroup::getDate() const
{
     return m_CreationDate;
}

void CGroup::setId(unsigned int id)
{
#ifdef __MONITOR_DEBUG
	qDebug() << "Debug: Setting groupID to " << id;
#endif
     m_Id = id;
}

unsigned int CGroup::getId() const
{
     return m_Id;
}

CLIENTS_MAP CGroup::getAllClientsFromMap() const
{
    return m_vCLients; 
}

bool CGroup::isConfigured() const
{
	return m_Configured;
}

void CGroup::setConfigured(bool configured)
{
	m_Configured = configured;
	emit signalUpdateGroupConfigured(this);
}

void CGroup::setTask(unsigned int task)
{
	m_Task = task;
}

unsigned int CGroup::getTask() const
{
	return m_Task;
}

void CGroup::setSettings(QString settings)
{
	m_Settings = settings;
}

QString CGroup::getSettings() const
{
	return m_Settings;
}

void CGroup::completeClient() 
{
	// This will be never hit
	if(m_numThrowClients == 0) {
		if(getStatus(CGroup::STATUS_THROWED)) {
			setStatus(CGroup::STATUS_THROWED, false);
			setStatus(CGroup::STATUS_COMPLETE, true);
			emit signalUpdateGroupComplete(this);
			m_numClompleteClients = 0;
		}
		return;
	}
		
	m_numClompleteClients++;	

	if(m_numThrowClients == m_numClompleteClients) {
		setStatus(CGroup::STATUS_THROWED, false);
		setStatus(CGroup::STATUS_COMPLETE, true);
		emit signalUpdateGroupComplete(this);
		m_numThrowClients = 0;
		m_numClompleteClients = 0;
	}
}

