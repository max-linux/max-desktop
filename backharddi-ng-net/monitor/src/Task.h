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
#ifndef Task_h
#define Task_h

#include <QMap>
#include <QHash>
#include <QDateTime>
#include <QDate>
#include <QString>
#include <QBitArray>

class CGroup;
class TaskManager;

typedef QHash<QString, CGroup*> TASK_GROUP_MAP;

enum STATUS_FLAG {          
  STATUS_PRELOAD      = 0x00,
  STATUS_NEW          = 0x01,
  STATUS_LOADED       = 0x02,
  STATUS_WAITING      = 0x03,
  STATUS_ACTIVE       = 0x04,
  STATUS_MODIFIED     = 0x05,
  STATUS_BADNAME      = 0x06,
  STATUS_BADDATE      = 0x07,
  STATUS_EMPTY        = 0x08,
  STATUS_DISCARD      = 0x09,
  STATUS_DUPLICATED   = 0x0A,
  STATUS_RUNNING	  = 0x0B,
  STATUS_DELETED      = 0x0C,
  MAX_STATES          = STATUS_DELETED +0x01
};

/**
 * \class CTask
 * \brief Tasks class 
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
 */
class CTask : public QObject
{  
  Q_OBJECT
  public:
     enum TASK_FIELDS {
          FIELD_ID,
          FIELD_DATE,
          FIELD_NAME
     };     
     
     CTask();
    
     CTask(unsigned int id, QDate *date, TASK_GROUP_MAP *groups, bool db=true);
     virtual ~CTask();
     
     void setId(unsigned int id);
     void setDate(QString date, QString format);
     void setDate(QString date);
     void setName(QString name);
     void setStorageType(bool db);
	
	bool isValid();
     
     void addGroupToMap(CGroup *group);
     bool isGroupInMap(QString groupName);
     void removeGroupFromMap(QString groupName);
	CGroup *takeGroupFromMap(QString groupName) const;
	CGroup *takeGroupFromMap(unsigned int groupId) const;
     
     unsigned int getId() const;
     QDate* getDate() const;  
     QString getName() const;   
     QString getDateString() const;
     TASK_GROUP_MAP* retrieveTaskGroups() const;
     bool getStorageType();         
     
     //===================================================================
     // State control Method
     //===================================================================
     void setState(STATUS_FLAG status, bool value);
     bool getState(STATUS_FLAG status) const;
     QBitArray retrieveStatus() const;
  
  private:
      unsigned int       m_Id;
      QString            m_Name;      
      
      QDate              *m_Date;
      QString            m_DateString;
      TASK_GROUP_MAP     *m_vGroups;     
      QBitArray          m_StatusFlag;
      
      QMap<const void*, void*> m_extraMap;
      
      bool               m_storageDatabase;
      
      friend class TaskManager;
};

#endif
