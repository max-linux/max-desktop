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
#include "groupdatabase.h"
#include "Loader.h"
#include "Database.h"
#include "cgroup.h"
#include "cclient.h"
#include "groupmanager.h"
#include "clientmanager.h"

#include <QtSql>
#include <QtCore>
#include <QDate>
#include <qdebug.h>

extern Loader *settings;
extern CDatabase *BDatabase;

class GroupDBPriv {
public:
	GroupDBPriv()
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

GroupDatabase::GroupDatabase()
{	
	d = new GroupDBPriv;
	d->d_settings = settings;
	d->m_database = BDatabase;
	d->query = new QSqlQuery(d->m_database->getDatabase());
}

GroupDatabase::~GroupDatabase()
{
	delete d;
}

int GroupDatabase::addGroup(const QString &name, const QDate &date, int taskId, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return -1;
	}
	
	if(name.isEmpty())
		return -1;
     
	if(exec( QString("INSERT INTO `groups` (group_name, creation_date, idtask, configured) " 
	  "VALUES('%1', '%2', '%3', '%4');")
	  .arg(escapeString(name),
	  date.toString(Qt::ISODate),
	  QString::number(taskId),
	  QString::number(0)), error)) {
		  return d->query->lastInsertId().toInt();
	}
	else
		return -1;
}

bool GroupDatabase::setConfigured(bool configured, QString &error)
{
	if(!exec(QString("UPDATE `groups` SET `configured`=%1;").arg(configured ? QString::number(1) : QString::number(0)), error)) 
		return false;	
	
	return true;	
}

void GroupDatabase::loadGroupFromDB(const int idgroup, QString &error)
{
	if(GroupManager::instance()->findGroup(idgroup) != NULL) {
		error = "The group is already loaded";
		return;
	}
	
	if(idgroup <= 0) {
		error = "The group id is not a valid one.";
		return;
	}
	
	QStringList values;
	QString sql = QString("SELECT `idgroup`, `group_name`, `creation_date`, `idtask`, `configured` FROM `groups` WHERE `idgroup`=").append(QString::number(idgroup));
	
	if(select(sql, error, &values)) {
		if(values.isEmpty()) {
			error = "The group is not stored on database.";
			return;
		}
		
		GroupManager::instance()->createGroup( values.at( 1 ), QDate::fromString( values.at( 2 ), Qt::ISODate ), values.at(3).toInt(), error, idgroup );
	}
}

bool GroupDatabase::exec(const QString &sql, QString &error, const bool debug)
{
	if(debug)
		qDebug() << "SQL-exec: " << sql;

	if(sql.isEmpty()) {
		qDebug() << "SQL-exec: The query is empty!";
		return false;
	}

	if(sql.contains("SELECT", Qt::CaseInsensitive)) {
		qDebug() << "SQL-exec: can't process the requested query, executed queries can't be SELECT queries. Use select method instead";
		return false;
	}

	d->query->exec(sql);
	if(!d->query->isActive()) {
		qDebug() << "SQL-exec: " << d->query->lastError();
		error = d->query->lastError().text();
		return false;
	}

	return true;
}

bool GroupDatabase::select(const QString &sql,  QString &error, QStringList *const values, const bool debug)
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
		
		if(numCols == 1)
			values->append("");

	}

	return true;
}

void GroupDatabase::deleteGroup(int groupId, QString &errMsg)
{
	if(!d) {
		errMsg = QString("No database handler pointer detected.");
		return;
	}
	
	exec( QString("DELETE FROM `groups` WHERE `idgroup`=%1")
			.arg(QString::number(groupId)), errMsg);
	exec( QString("DELETE FROM `groups_relation` WHERE `idgroup`=%1")
			.arg(QString::number(groupId)), errMsg);
	exec( QString("DELETE FROM `groups_configuration` WHERE `idgroup`=%1")
			.arg(QString::number(groupId)), errMsg);
}

QString GroupDatabase::escapeString(QString str) const
{
	str.replace( "'", "''" );
	return str;
}

bool GroupDatabase::isRunning()
{
	return d->m_database->isRunning();
}

bool GroupDatabase::isRegistered(QString table, QString target, QString value)
{
	QStringList *values;
	QString error;	
	if(select(QString("SELECT * FROM `%1` WHERE `%2`='%3'").arg(table, target, value), error, values)) {
		if(!values->isEmpty()) {
			return true;
		}
	}
	
	return false;
}

void GroupDatabase::loadClientsFromGroup(const int idgroup, QString &errMsg)
{	
	QString error;
	QStringList *values = new QStringList();
	
	if(select(QString("SELECT c.client_name, c.creation_date, c.client_status, c.idclient, c.client_mac, r.idgroup, r.idclient FROM `clients` AS c JOIN `groups_relation` AS r ON (r.idgroup=%1 AND r.idclient=c.idclient) ORDER BY c.idclient").arg(idgroup), errMsg, values)) {
		if(values->isEmpty()) {
			delete values;
			return;
		}

		for(int i=0; i < (values->count() / 7); i++) {
			CClient *tmpClient;
			// Check if the client already exists
			if(ClientManager::instance()->findClient(values->at(4 + (i*7))) == NULL) {
				// Create a new Client object using the ClientManager
				tmpClient = ClientManager::instance()->createClient(values->at(0 + (i*7)), values->at(3 + (i*7)), QDate::fromString(values->at(1 + (i*7)), "yyyy-MM-dd"), values->at(4 + (i*7)), error);
			}
			else {
				tmpClient = ClientManager::instance()->findClient(values->at(4 + (i*7)));
			}
			
			// Set the client's status
			tmpClient->setInformation("status", values->at(2 + (i*7)));
			
			// Set the client's group 
			tmpClient->setGroup(GroupManager::instance()->findGroup(idgroup));
			
			// Add the client to the client group map
			if( tmpClient->addCLientToMap() == ERR_CLIENT_ALREADY_ON_MAP ) {
#ifdef __MONITOR_DEBUG
				qDebug() << "GroupDatabase::loadClientsFromGroup Debug: Tried to add Client " << tmpClient->getFakeName() << " but key is already at map.";	
#endif
				// Hide the Graphic entity
				ClientManager::instance()->getClientItem(tmpClient)->setHidden(true);
			}

			GroupManager::instance()->findGroup(idgroup)->addCLientToMap(tmpClient->getName(), tmpClient);
		}
	}

	delete values;
}


void GroupDatabase::setGroupName(int id, const QString &newName, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
	
	if(newName.isEmpty())
		return;
     
	exec( QString("UPDATE `groups` SET `group_name`='%1' WHERE `idgroup`='%2'")
			.arg(escapeString(newName), QString::number(id)), error);
}

void GroupDatabase::setGroupDate(int id, const QDate &newDate, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
	
	exec( QString("UPDATE `groups` SET `creation_date`='%1' WHERE `idgroup`='%2'")
			.arg(newDate.toString(Qt::ISODate), QString::number(id)), error);
}

void GroupDatabase::loadGroupConfigFromDB(const int idgroup, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
	
	QStringList values;
	select(QString("SELECT `idconfig`, `idgroup`, `modo`, `imagenes` FROM `groups_configuration` WHERE `idgroup`=%1").arg(QString::number(idgroup)), error, &values, true);
	
	if(values.isEmpty()) {
		error = QString("No register found with this ID on database.");
		return;
	}	
	
	CGroup *tmpGroup = GroupManager::instance()->findGroup(values.at(1).toInt());	
	QString finalString;
	finalString.append(values.at(2));
	finalString.append(",");
	finalString.append(values.at(3));
	finalString.append(",");
	finalString.append(values.at(0));
	
	tmpGroup->setSettings(finalString);
	tmpGroup->setConfigured(true);
}

void GroupDatabase::saveGroupConfig(QString groupSettings, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
	
	if(groupSettings.isEmpty()) {
		error = QString("The settings are void.");
		return;
	}
	
	exec(QString("INSERT INTO `groups_configuration` (`idgroup`, `modo`, `imagenes`) VALUES (%1, '%2', '%3')").arg(groupSettings.split(",").at(2), groupSettings.split(",").at(0), groupSettings.split(",").at(1)), error);
}

void GroupDatabase::updateGroupConfig(QString groupSettings, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return;
	}
	
	if(groupSettings.isEmpty()) {
		error = QString("The settings are void.");
		return;
	}
	
	exec(QString("UPDATE `groups_configuration` SET `modo`='%1', `imagenes`='%2' WHERE `idgroup`=%3").arg(groupSettings.split(",").at(0), groupSettings.split(",").at(1), groupSettings.split(",").at(2)), error);
}

void GroupDatabase::addClientsRelation(const int groupId, const QString &clientId)
{
	if(!d)
		return;
	
	QString error;
	QStringList values;
	
	select(QString("SELECT `idgroup`, `idclient` FROM `groups_relation` WHERE `idgroup`=%1 AND `idclient`='%2'").arg(QString::number(groupId), clientId), error, &values, true);
	
	if(values.isEmpty())
		exec(QString("INSERT INTO `groups_relation` (idgroup, idclient) VALUES (%1, '%2')").arg(QString::number(groupId), clientId), error);	
	
	if(!error.isEmpty())
		qDebug() << error;
}

void GroupDatabase::deleteClientsRelation(const int groupId, const QString &clientId)
{
	if(!d)
		return;
	
	QString error;
	
	exec(QString("DELETE FROM `groups_relation` WHERE `idgroup`=%1 AND `idclient`='%2'").arg(QString::number(groupId), clientId), error);
}


