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
#include "cclient.h"
#include "cgroup.h"
#include "MonitorServer.h"
#include "clientdatabase.h"
#include "clientmanager.h"
#include "osdwidget.h"
#include "TaskManager.h"

extern wchar_t* g_ErrorString[64][MAX_ERROR_CODE];
extern MonitorServer *PServer;
ClientDatabase *cDatabase;

CClient::CClient() : QObject(0) {
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: CClient " << this << " constructor called.";
#endif
     setName(QString("<New Client>"));
     m_StatusFlagCode << tr("Disconnected") << tr("Connected") << tr("Waiting") << tr("Mastering") << tr("Uploading Image") << tr("Done") << tr("Failed");
     m_Group = NULL;
	m_pingWait = false;
	m_timer = new QTimer(this);
	m_timer->start(20000);
	m_waitTimer = new QTimer(this);
	
	connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(clientDisconnect()));
}

CClient::CClient(QByteArray Id) : QObject(0) {
     m_ID = Id;
     m_StatusFlagCode << tr("Disconnected") << tr("Connected") << tr("Waiting") << tr("Mastering") << tr("Uploading Image") << tr("Done") << tr("Failed");
     m_Group = NULL;
	m_pingWait = false;
	m_timer = new QTimer(this);
	m_timer->start(20000);
	m_waitTimer = new QTimer(this);
	
	connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(clientDisconnect()));
}


CClient::~CClient() {
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: CClient " << this << " destructor called.";
#endif
     // Remove self from clients map if there yet
     ERROR_CODE err;
     if(isClientInMap()) {
          if((err = removeClientFromMap()) != ERR_NO_ERROR) {
               qWarning() << "Warning: CClient " << getFakeName() << " tried to delete iself from clients map but failed with error code " << g_ErrorString[err] << "!!!";
          }
     }
	
	delete m_timer;
}

void CClient::setName(QString clientName)
{
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: Setting ClientName as " << clientName << ".";
#endif
     m_Name = clientName;
}

QString CClient::getName() const
{
     return m_Name;
}

QString CClient::getFakeName()
{
	return getInformation("name");
}

void CClient::setFakeName(const QString &name)
{
	setInformation("name", name);
}

void CClient::setGroup(CGroup* pGroup) {
#ifdef __MONITOR_DEBUG
	if(pGroup)
     	qDebug() << "Debug: Setting client Group " << pGroup->getName() << " to client " << getName() << ".";
#endif
     m_Group = pGroup;
}

CGroup* CClient::getGroup() const
{
     return m_Group;
}

void CClient::setShell(ShellWidget *sh, int shId)
{
	m_Shell = sh;
	m_shId = shId;
}

void CClient::unsetShell()
{
	delete m_Shell;
	m_Shell = NULL;
	m_shId = -1;
}

ShellWidget *CClient::Shell() const
{
	return m_Shell;
}

int CClient::getShellId()
{
	return m_shId;
}

bool CClient::getStatus(CLIENT_STATUS_FLAG bit) const
{
	if(m_StatusFlag.isEmpty())
		return false;
	
	return m_StatusFlag.testBit(bit);
}

void CClient::setStatus(CLIENT_STATUS_FLAG status, bool value)
{
	if(m_StatusFlag.isEmpty()) {
		m_StatusFlag.resize(MAX_CLIENT_STATUS);
		m_StatusFlag.fill(false, MAX_CLIENT_STATUS);
	}
	m_StatusFlag.setBit(status, value);
}

ERROR_CODE CClient::addCLientToMap()
{
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: Trying to add client " << getFakeName() << " to clients map with ID " << getID();
#endif
	if(ClientManager::instance()->findClient(getID()) == NULL) {
		ClientManager::instance()->insertClient(this);
          return ERR_NO_ERROR;
     }
     else {
          qWarning() << "Warning: Client " << getFakeName() << " tried to be added on clients map but its already on it!!!";
          return ERR_CLIENT_ALREADY_ON_MAP;
     }
}

bool CClient::isClientInMap() const 
{
	if(ClientManager::instance()->findClient(getID()) == NULL)
		return false;
	
	return true;
}

CClient* CClient::getClientFromMap(QString clientName)
{
	return ClientManager::instance()->findClient(clientName); 
}

ERROR_CODE CClient::removeClientFromMap()
{
#ifdef __MONITOR_DEBUG
     qDebug() << "Debug: Trying to remove client " << getFakeName() << " from clients map.";
#endif
	
	QString errMsg;
	if(ClientManager::instance()->deleteClient(this, errMsg))
          return ERR_NO_ERROR;
     else 
          return ERR_CLIENT_UNKNOWN;
}

QByteArray CClient::getID() const
{
     return m_ID;
}

bool CClient::setID(QByteArray uniqueID)
{
     m_ID = uniqueID;
     return TRUE;
}

void CClient::setTitle(QString title) {
     m_Titulo = title;    
}

QString CClient::getTitle() const {
     return m_Titulo;
}

void CClient::setInformation(QString key, QString value) 
{
     // If key already exists remove it 
     if( m_Information.contains(key) ) 
          m_Information.remove(key);

     // Insert the new key
     m_Information.insert(key, value);
}

QString CClient::getInformation(QString key)
{
     if( m_Information.contains(key))
          return m_Information[key];
     else
          return QString("");
}

QString CClient::getMac()
{
	return getInformation("mac");
}

quint32 CClient::getStatusBitmap()
{
	if(m_StatusFlag.isEmpty())
		return 0;
	
	quint32 bitMap = 0;
	
	for(int i = 0; i < MAX_CLIENT_STATUS; ++i) {
		if(m_StatusFlag.testBit(i)) {	
			if(m_StatusFlag[i])
				bitMap |= i;
			else
				bitMap &= ~i;
		}
	}
	
	return bitMap;
}

QString CClient::getHardwareList()
{
	QString information;
	foreach(QString key, m_Information.keys()) {
		information += key;
		information += "=";
		information += m_Information[key];
		information += ",";
	}
	
	return information;
}

QStringList CClient::getStatusFlagCode()
{
	QStringList tmpList;
	if(m_StatusFlag.isEmpty()) {
		tmpList.append(tr("Status Unknown"));
		return tmpList;
	}
	
	for(int i = 0; i < MAX_CLIENT_STATUS; ++i) {
		if(m_StatusFlag.testBit(i))
			tmpList << m_StatusFlagCode.at(i);
	}
	
	return tmpList;
}

QTimer *CClient::getTimer()
{
	return m_timer;
}

void CClient::ping()
{	
	if(getStatus(CLIENT_STATUS_CONNECTED))
		ClientManager::instance()->pingClient(this);
}

void CClient::setWaitingForPing(bool b)
{
	m_pingWait = b;
	
	if(m_pingWait)
		m_waitTimer->start(4000);
	else
		m_waitTimer->stop();
}

void CClient::clientDisconnect()
{
	m_StatusFlag.fill(false, m_StatusFlag.size());	
	setStatus(CLIENT_STATUS_DISCONNECTED, true);
	
	TaskManager::instance()->redrawTask();
	setWaitingForPing(false);
}

