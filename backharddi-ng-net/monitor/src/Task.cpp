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

#include "Task.h"
#include "cgroup.h"

CTask::CTask(unsigned int id, QDate *date, TASK_GROUP_MAP *groups, bool db)
{
     m_Id = id;
     m_Date = date;
     m_Name = "";
     m_vGroups = groups;
     setStorageType(db);	
}

CTask::CTask()
{
     m_Id = 0;
     m_Date = 0;
     m_Name = "";
     m_vGroups = new TASK_GROUP_MAP;              
}

CTask::~CTask()
{
     if(!m_extraMap.isEmpty()) {
          m_extraMap.clear();
     }
     
     if(!m_vGroups->isEmpty()) {
          m_vGroups->clear();
          delete m_vGroups;
     }     
}

void CTask::setId(unsigned int id)
{
     m_Id = id;
}

void CTask::setDate(QString date, QString format)
{
     if(m_Date == NULL) 
          m_Date = new QDate();     
     
     m_Date->fromString(date, format);
}

void CTask::setDate(QString date)
{
     m_DateString = date;
}

void CTask::setName(QString name)
{
     m_Name = name;
}

unsigned int CTask::getId() const
{
     if(!this)
          return 0;
     
     return m_Id;
}

QDate* CTask::getDate() const
{
     return m_Date;
}

QString CTask::getDateString() const
{
     return m_DateString;
}

QString CTask::getName() const
{
     return m_Name;
}

TASK_GROUP_MAP* CTask::retrieveTaskGroups() const
{
     return m_vGroups;
}

void CTask::setState(STATUS_FLAG status, bool value)
{     
     if(m_StatusFlag.isEmpty()) {
          m_StatusFlag.resize(MAX_STATES);
          m_StatusFlag.fill(true, MAX_STATES);
     }
     m_StatusFlag.setBit(status, value);
}

bool CTask::getState(STATUS_FLAG bit) const
{
     if(m_StatusFlag.isEmpty())
          return false;
     
     return m_StatusFlag.testBit(bit);
}

QBitArray CTask::retrieveStatus() const
{
     if(m_StatusFlag.isEmpty())
          return QBitArray();
     
     return m_StatusFlag;
}

void CTask::addGroupToMap(CGroup *group)
{
     if(group != NULL) {
#ifdef __MONITOR_DEBUG
	     qDebug() << "Debug: Inserting group " << group->getName() << " into task " << getName();
#endif
	     
	     if(!isGroupInMap(group->getName()))
	     		m_vGroups->insert(group->getName(), group);
     }
}

bool CTask::isGroupInMap(QString groupName)
{
     return m_vGroups->contains(groupName);
}

void CTask::removeGroupFromMap(QString groupName) 
{ 
     if(isGroupInMap(groupName))
          m_vGroups->remove(groupName);
}

void CTask::setStorageType(bool db)
{
     m_storageDatabase = db;
}

bool CTask::getStorageType()
{
     return m_storageDatabase;
}

CGroup *CTask::takeGroupFromMap(QString groupName) const
{
	if(!m_vGroups->contains(groupName))
		return 0;
	
	if(m_Id == 0)
		return 0;
	
	TASK_GROUP_MAP *groupList = retrieveTaskGroups();
	TASK_GROUP_MAP::iterator it = groupList->begin();
	
	while(it != groupList->end()) {
		CGroup * pGroup = (*it);
		if(pGroup->getName() == groupName) {
			return pGroup;
		}
	}	
	
	return 0;
}

CGroup *CTask::takeGroupFromMap(unsigned int groupId) const
{	
	if(m_Id == 0)
		return 0;
	
	TASK_GROUP_MAP *groupList = retrieveTaskGroups();
	TASK_GROUP_MAP::iterator it = groupList->begin();
	
	while(it != groupList->end()) {
		CGroup * pGroup = (*it);
		if(pGroup->getId() == groupId) {
			return pGroup;
		}
	}
	
	return 0;
}

bool CTask::isValid() 
{
	return (m_Id != 0) ? true : false;
}

