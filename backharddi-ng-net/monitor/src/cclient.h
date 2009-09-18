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
#ifndef _cclient_h_
#define _cclient_h_

#include "Common.h"

#include <QBitArray>
#include <shellwidget.h>

class CGroup;
class OSDWidget;

enum CLIENT_STATUS_FLAG {
	CLIENT_STATUS_DISCONNECTED	= 0x00,       // Client is not connected to monitor
 	CLIENT_STATUS_CONNECTED		= 0x01,       // Client just connected to monitor
  	CLIENT_STATUS_WAITING		= 0x02,       // Client is just waiting 
   	CLIENT_STATUS_MASTERING		= 0x03,       // Client is in mastering process
    	CLIENT_STATUS_UPLOADING		= 0x04,       // Client is uploadign a Master to Server
	CLIENT_STATUS_JUST_DONE		= 0x05,       // Client just done activity without errors
 	CLIENT_STATUS_JUST_ERROR		= 0x06,       // Client just done activity with errors
  	CLIENT_STATUS_SHELL_ATTACHED	= 0x07,	    // Client has an attached shell xterm
  	MAX_CLIENT_STATUS			= CLIENT_STATUS_SHELL_ATTACHED + 0x01
};

/**
CClient is filled with the Backharddi-Net clients who connect to the service. The clients are stored into a List before they are used to create any Multicast Group.


     @author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
 */

class CClient : public QObject 
{
	Q_OBJECT
public:
     
     /// CClient() Default Constructor     
     CClient();
     CClient(QByteArray Id);

     
     /// ~CClient() Default Destructor     
     virtual ~CClient();
     
     
     /// setName(QString clientName) Sets the client's name.     
     /// @arg clientName 
     /// @return void     
     void setName(QString clientName);
	void setFakeName(const QString &name);
     
     
     /// getName() const Returns the client's Name
     /// @return QString     
     QString getName() const;
	QString getFakeName();
     
     
     /// setGroup(CGroup* group) Sets the client's multicast group.
     /// @arg CGroup* pGroup;  Pointer to multicast group
     /// @return void      
     void setGroup(CGroup* pGroup);
     
     
     /// getGroup() const Returns the client's group if any, if not return
     /// null.
     /// @return CGroup pointer     
     CGroup* getGroup() const;	
     
     /// getStatus() const Return the client's state 
     /// @return CLIENT_STATUS enum member     
     bool getStatus(CLIENT_STATUS_FLAG bit) const;          
     
     /// addClientToMap() Insert the client into the clients map if is not
     /// inserted yet, if fails return ERROR_CODE
     /// @return ERROR_CODE
     ERROR_CODE addCLientToMap();
     
     /// isClientInMap() Return TRUE if the client is part of the map
     /// @return bool
     bool isClientInMap() const;
     
     /// static getClientFromMap() Return a pointer to a client if 
     /// is stored on map
     /// @arg QString
     /// @return CClient
     CClient* getClientFromMap(QString clientName);
     
     /// removeClientFromMap() Removes the client from the clients map
     /// @return ERROR_CODE
     ERROR_CODE removeClientFromMap();
     
     /// setTitle(QString title) Sets the client's title.
     /// @arg QString
     /// @return void
     void setTitle(QString title);
     
     /// getTitle() Return the client's title.     
     /// @return QString;
     QString getTitle() const;	
     
     /// setInformation(QString Information) Sets an client information key;value pair
     /// @param QString key
     /// @param QString value
     /// @return void
     void setInformation(QString key, QString value);
     
     /// getInformation(QString key) Return a value from key of hash map
     /// @param QString key
     /// @return QString
     QString getInformation(QString key);    
     
     /// getID() const Return the client's Unique Identificator
     /// @return QString     
     QByteArray getID() const;
     
     
     /// setID(QString uniqueID) Sets the client Unique Identificator
     /// @arg QString
     /// @return bool     
     bool setID(QByteArray uniqueID);
	
	/// getMac() Returns the client NIC Mac
	/// @return QString
     QString getMac();
	
     void setStatus(CLIENT_STATUS_FLAG status, bool value);	
     quint32 getStatusBitmap();
	
     QString getHardwareList();
     QStringList getStatusFlagCode();
	
	void setShell(ShellWidget *sh, int shId);
	void unsetShell();
	int getShellId();
	
	void setToolTip(QString tooltip) {
		m_toolTip = tooltip;
	}
	
	QString toolTip() const {
		return m_toolTip;
	}
	
	ShellWidget *Shell() const;	
	
	QTimer *getTimer();
	void setWaitingForPing(bool b);
	
public slots:
	void ping();
	void clientDisconnect();

private:  
     //====================================================================
     // Private Member vars
     //====================================================================
     CGroup*        	m_Group;
     QString        	m_Name;
     QString        	m_Titulo;
     QByteArray     	m_ID;
	QString			m_toolTip;
     QHash<QString, QString> m_Information;
     QBitArray          m_StatusFlag;
     QStringList	m_StatusFlagCode;
	ShellWidget 		*m_Shell;	
	int				m_shId;
	bool				m_pingWait;
	
	QTimer 			*m_timer;
	QTimer			*m_waitTimer;
};

CClient* getClientFromMap(QString clientName);

#endif    // Endif _cclient_h_
