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
#ifndef _cgroup_h_
#define _cgroup_h_

#include "Common.h"

#include <QBitArray>

class CClient;
class CTask;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/

//====================================================================
// Container definitions
//====================================================================
typedef QHash<QString, CClient*> CLIENTS_MAP;
typedef CLIENTS_MAP clientMap;
typedef QVector<CClient*> FVClients;

class CGroup : public QObject
{
	Q_OBJECT
public:
     enum STATUS_FLAG {
          STATUS_LOADED            = 0x00,
          STATUS_MODIFIED          = 0x01,
          STATUS_THROWED           = 0x02,
          STATUS_DISCARD           = 0x03,
          STATUS_DUPLICATED        = 0x04,
          STATUS_DELETED_FROM_TASK = 0x05,
		STATUS_COMPLETE		= 0x06,
          STATUS_DELETED           = 0x07,
          MAX_STATES               = STATUS_DELETED +0x01 
     };
     
     //====================================================================
     // Default Constructor
     //====================================================================
     CGroup();
     
     //====================================================================
     // Default Destructor
     //====================================================================
     virtual ~CGroup();
     
     void setName(QString groupName);
     QString getName() const;    
     void setDate(QString date);
     QString getDate() const; 
     void setId(unsigned int id);
     unsigned int getId() const;
     void setTask(unsigned int task);
     unsigned int getTask() const;
     
     //====================================================================
     // Boolean Methods
     //====================================================================
     void GroupChanged(bool val);
     bool IsChanged() const;
     void GroupThrowed(bool val);
     bool IsThrowed() const;
     bool isConfigured() const;
     void setConfigured(bool configured);
     
     //====================================================================
     // Groups HashMap Methods
     //====================================================================
     bool isGroupOnMap();
     bool addGroupToMap();

     //====================================================================
     // Clients on Group HasMap Methods
     //====================================================================
     bool isClientOnMap(QString clientName);
     ERROR_CODE addCLientToMap(QString clientName, CClient* newCLient);
     CClient* getClientFromMap(QString clientName) const; 
     ERROR_CODE removeClientFromMap(QString clientName);
     void addFailedClientToMap(CClient *client);
     void removeFailedClientFromMap(CClient *client);
     FVClients GetFailedClientsFromMap();
     CLIENTS_MAP getAllClientsFromMap() const;

     //====================================================================
     // BIT MASK
     //====================================================================
	void setStatus(CGroup::STATUS_FLAG flag, bool value) 
	{
		m_StatusFlag->setBit(flag, value);
	}
	
	bool getStatus(CGroup::STATUS_FLAG flag)
	{
		return m_StatusFlag->testBit(flag);
	}
	
	//===================================================================
	// Settings
	//===================================================================
	void setSettings(QString settings);
	QString getSettings() const;
	
	inline void setNumThrowedClients(unsigned int num) {
		m_numThrowClients = num;
	}
	
	inline unsigned int getThrowedClients() const {
		return m_numThrowClients;
	}
	
	void completeClient();	
	
signals:
	void signalUpdateGroupRelation(CGroup *group, QString clientName);
	void signalUpdateGroupConfigured(CGroup *group);
	void signalUpdateGroupComplete(CGroup *group);
    
private:
     QString        m_Name;
     QString        m_CreationDate;
     clientMap      m_vCLients;
     unsigned int   m_Task;
     unsigned int   m_Id;
     unsigned int   m_numThrowClients;
     unsigned int   m_numClompleteClients;

     QBitArray      *m_StatusFlag;
     bool           m_Changed;
     bool           m_Throwed;
     bool	    m_Configured;
	
     QString	    m_Settings;
	
     FVClients	    m_fvClients;

public:
     QString	    m_generateIP;
};

CGroup* getGroupFromMap(QString groupName);

#endif	// _cgroup_h_
