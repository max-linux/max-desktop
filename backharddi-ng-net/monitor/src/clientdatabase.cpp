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
#include "Loader.h"
#include "Database.h"
#include "groupmanager.h"
#include "cgroup.h"
#include "clientmanager.h"
#include "cclient.h"

#include <QtSql>
#include <QtCore>
#include <QDate>
#include <qdebug.h>

extern Loader *settings;
extern CDatabase *BDatabase;

class ClientDBPriv {
public:
	ClientDBPriv()
	{
		valid = false;
	}

	bool valid;
	int numRows;
	QSqlQuery *query;
	QSqlTableModel *model;
	Loader *d_settings;
	CDatabase *m_database;
};

ClientDatabase::ClientDatabase()
{
	d = new ClientDBPriv;
	d->d_settings = settings;
	d->m_database = BDatabase;
	d->query = new QSqlQuery(d->m_database->getDatabase());
}

ClientDatabase::~ClientDatabase()
{
	delete d;
}

bool ClientDatabase::exec(const QString &sql, QString &error, const bool debug)
{
	if(debug)
		qDebug() << "SQL-exec: " << sql;
     
	if(sql.isEmpty()) {
#ifdef __MONITOR_DEBUG
		qDebug() << "SQL-exec: The query is empty!";
#endif
		return false;
	}
     
	if(sql.contains("SELECT", Qt::CaseInsensitive)) {
#ifdef __MONITOR_DEBUG
		qDebug() << "SQL-exec: can't process the requested query, executed queries can't be SELECT queries. Use select method instead";
#endif
		return false;
	}     
     
	d->query->exec(sql);
	if(!d->query->isActive()) {
#ifdef __MONITOR_DEBUG
		qDebug() << "SQL-exec: " << d->query->lastError();
#endif
		error = d->query->lastError().text();
		return false;
	}    
     
	return true;
}

bool ClientDatabase::select(const QString &sql,  QString &error, QStringList *const values, const bool debug)
{
	if(debug)
		qDebug() << "SQL-select: " << sql;
     
	if(sql.isEmpty()) {
		error.append("SQL-select: The sql query is empty.");
		return false;
	}
     
	if(!sql.contains("SELECT", Qt::CaseInsensitive)) {
		error.append("SQL-select: The query is not or has not a SELECT statement, please use exec method to execute non select queries.");
		return false;
	}
     
	d->query->exec(sql);
	if(!d->query->isActive()) {
		error.append("SQL-select: ");
		error.append(d->query->lastError().text());
		return false;
	}
     
	int numCols = 0;
	while(d->query->next()) {
		QSqlRecord rec = d->query->record();
		numCols = rec.count();

		for(int i=0; i < numCols; i++)
			values->append(rec.value(i).toString());

	}
     
	return true;
}

int ClientDatabase::addClient(const QString &name, const QDate &date, const QString id, const QString mac, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return -1;
	}
	
	if(name.isEmpty())
		return -1;
	
	if(exec( QString("INSERT INTO `clients` (idclient, client_name, creation_date, client_status, client_mac) " 
		  "VALUES('%1', '%2', '%3', '%4', '%5');")
		  .arg(escapeString(id),
			  escapeString(name),
			  date.toString(Qt::ISODate),
			  "0",
			  escapeString(mac)), error)) {

		return d->query->lastInsertId().toInt();
	}
	else 
		return -1;
	
}

void ClientDatabase::setClientName(const QString &clientId, const QString &name, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
	
	if(name.isEmpty())
		return;
	
	if(!exec( QString("UPDATE `clients` SET `client_name`='%1' WHERE `idclient`='%2'")
			.arg(escapeString(name), clientId), error))
		error = d->m_database->lastError().databaseText();
}

void ClientDatabase::setClientDate(const QString &clientId, const QDate &date, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
	
	exec( QString("UPDATE `clients` SET `creation_date`='%1' WHERE `idclient`='%2'")
			.arg(date.toString(Qt::ISODate), clientId), error);
}

QString ClientDatabase::getClientName(QString clientId, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return QString();
	}
     
	QStringList *values = NULL;
     
	if(select( QString("SELECT `client_name, idclient` FROM `clients` WHERE `idclient`='%1'")
		  .arg(clientId), error, values))
	{
		if(values->isEmpty())
			return QString();
          
		return values->at(0);
	}
     
	return QString();
}

QString ClientDatabase::getClientId(const QString &name, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return QString();
	}
     
	if(name.isEmpty())
		return QString();
     
	QStringList *values = NULL;     
	if(select( QString("SELECT `idclient` FROM `clients` WHERE `client_name`='%1'")
		  .arg(escapeString(name)), error, values))
	{
		if(values->isEmpty())
			return QString();
          
		return values->at(0);
	}
     
	return QString();
}

QDate ClientDatabase::getDate(QString clientId, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return QDate();
	}
     
	QStringList *values = NULL;
	if(select( QString("SELECT `creation_date` FROM `clients` WHERE `idclient`='%1'")
		  .arg(clientId), error, values))
	{
		if(values->isEmpty())
			return QDate();
          
          
		return QDate::fromString(values->at(0), "yyyymmdd");
	}
     
	return QDate();
}

void ClientDatabase::deleteClient(QString clientId, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
     
	exec( QString("DELETE FROM `clients` WHERE `idclient`='%1'")
			.arg(clientId), error);
}

QString ClientDatabase::escapeString(QString str) const 
{
	str.replace( "'", "''" );
	return str;
}

bool ClientDatabase::isRunning()
{
	return d->m_database->isRunning();
}

void ClientDatabase::loadClientGroup(QString &error, QString clientId)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
	
	QStringList *values = NULL;
	if(select( QString("SELECT `idgroup`, `idclient` FROM `groups_relation` WHERE `idclient`='%1'")
		  .arg(QString(ClientManager::toBase64(clientId))), error, values))
	{
		if(values->isEmpty())
			return;

		ClientManager::instance()->findClient(ClientManager::toBase64(clientId))->setGroup(GroupManager::instance()->findGroup(values->at(0)));
	}
}

void ClientDatabase::loadClientFromDB(QString &error, QString clientId)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
	
	QStringList *values = NULL;
	if(select( QString("SELECT `idclient`, `client_name`, `creation_date`, `client_status`, `client_mac` FROM `clients` WHERE `idclient`='%1';").arg( clientId), error, values) ) {
		if(values->isEmpty()) {
			error = "Client " + clientId + " not found on database.";
			return;
		}
		
		// Create the new client using ClientManager
		CClient *tmpClient = ClientManager::instance()->createClient( values->at(1), values->at(0), QDate::fromString(values->at(2), "yyyy-MM-dd"), values->at(4), error  );
		if(tmpClient == NULL) 
			return;
		
		// Set client status
		tmpClient->setInformation("status", values->at(3));
		
		// Insert client on the client group map 
		if( tmpClient->addCLientToMap() == ERR_CLIENT_ALREADY_ON_MAP ) {
#ifdef __MONITOR_DEBUG
			qDebug() << "ClientDatabase::loadClientFromDB Debug: Tried to add Client " << tmpClient->getFakeName() << " but key is already at map.";	
#endif
		}
	}
	else {
		error = "Client " + clientId + " not found on database.";
#ifdef __MONITOR_DEBUG
		qDebug() << error;
#endif
		return;
	}
}
	
