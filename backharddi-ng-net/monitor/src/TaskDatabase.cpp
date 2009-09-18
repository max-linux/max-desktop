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
#include "TaskDatabase.h"
#include "TaskManager.h"
#include "groupmanager.h"
#include "clientmanager.h"
#include "Loader.h"
#include "Database.h"
#include "cgroup.h"
#include "cclient.h"

#include <QtSql>
#include <QtCore>
#include <QDate>
#include <qdebug.h>

extern Loader *settings;
extern CDatabase *BDatabase;

class TaskDBPriv {
  public:
    TaskDBPriv()
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

TaskDatabase::TaskDatabase()
{
     d = new TaskDBPriv;
     d->d_settings = settings;
     d->m_database = BDatabase;
     d->query = new QSqlQuery(d->m_database->getDatabase());
}

TaskDatabase::~TaskDatabase()
{
     delete d;
}

bool TaskDatabase::exec(const QString &sql, QString &error, const bool debug)
{
     if(debug)
       qDebug() << "SQL-exec: " << sql;
     
     if(sql.isEmpty()) {
          qDebug() << "SQL-exec: The query is empty!";
          return false;
     }
     
     if(sql.contains("SELECT", Qt::CaseInsensitive) && !sql.contains("now()", Qt::CaseInsensitive)) {
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

bool TaskDatabase::select(const QString &sql,  QString &error, QStringList *const values, const bool debug)
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

int TaskDatabase::addTask(const QString &name, const QDate &date, QString &error)
{
     if(!d) {
          error = QString("No database handler pointer detected.");
          return -1;
     }
     
     if(name.isEmpty())
          return -1;
     
     if(exec( QString("INSERT INTO `tasks` (task_name, task_date) "
          "VALUES('%1', '%2');").arg(
               escapeString(name),
               date.toString(Qt::ISODate)), error)) {
          
          return d->query->lastInsertId().toInt();
     }     
     else
          return -1;
}

int TaskDatabase::addTask(const QString &name, QString &error)
{
	if(!d) {
		error = QString("No database handler pointer detected.");
		return -1;
	}
	
	if(name.isEmpty())
		return -1;
	
	if(exec(QString("INSERT INTO `tasks` (task_name, task_date) VALUES('%1', '%2');").arg(escapeString(name), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")), error)) {
		return d->query->lastInsertId().toInt();
	}
	else 
		return -1;
}

void TaskDatabase::setTaskName(int taskId, const QString &name, QString &error)
{
     if(!d) {
          error = QString("No database handler pointer detected.");
          return;
     }
     
     if(name.isEmpty())
          return;
     
     exec( QString("UPDATE `%1`.`tasks` SET `task_name`='%2' WHERE `idtask`=%3")
               .arg(d->d_settings->getKey("database").toString(), escapeString(name), QString::number(taskId)), error);
}

void TaskDatabase::setTaskDate(int taskId, const QDate &date, QString &error)
{
     if(!d) {
          error = QString("No database handler pointer detected.");
          return;
     }
     
     exec( QString("UPDATE `%1`.`tasks` SET `task_date`='%2' WHERE `idtask`=%3")
               .arg(d->d_settings->getKey("database").toString(), date.toString(Qt::ISODate), QString::number(taskId)), error);
}

QString TaskDatabase::getTaskName(int taskId, QString &error)
{
     if(!d) {
          error = QString("No database handler pointer detected.");
          return QString();
     }
     
     QStringList *values=NULL;
     
     if(select( QString("SELECT `task_name` FROM `%1`.`tasks` WHERE `idtask`=%2")
                    .arg(d->d_settings->getKey("database").toString(), QString::number(taskId)), error, values))
     {
          if(values->isEmpty())
               return QString();
          
          return values->at(0);
     }
     
     return QString();
}

int TaskDatabase::getTaskId(const QString &name, QString &error)
{
     if(!d) {
          error = QString("No database handler pointer detected.");
          return -1;
     }

     if(name.isEmpty())
          return -1;

     QStringList *values = NULL;
     if(select( QString("SELECT `idtask` FROM `%1`.`tasks` WHERE `task_name`='%2'")
                    .arg(d->d_settings->getKey("database").toString(), escapeString(name)), error, values))
     {
          if(values->isEmpty())
               return -1;

          return values->at(0).toInt();
     }

     return -1;
}

QDate TaskDatabase::getDate(int taskId, QString &error)
{
     if(!d) {
          error = QString("No database handler pointer detected.");
          return QDate();
     }
     
     QStringList *values = NULL;
     if(select( QString("SELECT `task_date` FROM `%1`.`tasks` WHERE `idtask`=%2")
          .arg(d->d_settings->getKey("database").toString(), QString::number(taskId)), error, values))
     {
          if(values->isEmpty())
               return QDate();
          
          
          return QDate::fromString(values->at(0), "yyyymmdd");
     }
     
     return QDate();
}

void TaskDatabase::deleteTask(int taskId, QString &error)
{
     if(!d) {
          error = QString("No database handler pointer detected.");
          return;
     }
     
     exec( QString("DELETE FROM `tasks` WHERE `idtask`=%1")
               .arg(QString::number(taskId)), error);
}

QString TaskDatabase::escapeString(QString str) const 
{
     str.replace( "'", "''" );
     return str;
}


void TaskDatabase::loadTaskGroups(QString &error, unsigned int taskid)
{
     QStringList *values = new QStringList();
     if(select(QString("SELECT `idgroup`, `group_name`, `creation_date`, `configured` FROM groups WHERE `idtask`=%1").arg(taskid), error, values)) {
          if(values->isEmpty()) {
               delete values;
               return;
          }
		
		for(int i=0; i < (values->count() / 4); i++) {
			GroupManager::instance()->loadGroup(values->at(0 + (i*4)).toInt(), error);
			
			// Get the task pointer 
			CTask * tmpTask = (CTask*)TaskManager::instance()->findTask(taskid);
			
			// Add the group to the task
			tmpTask->addGroupToMap(GroupManager::instance()->findGroup(values->at(0 + (i*4)).toInt()));
			
			// Load the group config
			GroupManager::instance()->loadGroupConfig(values->at(0 + (i*4)).toInt(), error);
			
			// Load the group clients
			GroupManager::instance()->loadGroupClients(values->at(0 +(i*4)).toInt()
			, error);
		}
     }

     delete values;
}

void TaskDatabase::loadTaskGroupsClients(QString &error, CGroup *pGroup)
{
	// Perform the SQL Query
     QStringList *values = new QStringList();
     if(select(QString("SELECT c.client_name, c.creation_date, c.client_status, c.idclient, c.client_mac, r.idgroup, r.idclient FROM `clients` AS c JOIN `groups_relation` AS r ON (r.idgroup=%1 AND r.idclient=c.idclient) ORDER BY c.idclient").arg(pGroup->getId()), error, values)) {

		// Check if we get some result from the query
		if(values->isEmpty()) {
               delete values;
               return;
          }

		// Create new client objects for the loaded group
          for(int i=0; i < (values->count() / 7); i++) {
			// Create the new client
			CClient *tmpClient = ClientManager::instance()->createClient(values->at(0 + (i*7)), values->at(3 + i*7), QDate::fromString(values->at(1 + (i*7)), "yyyy-MM-dd"), values->at(4 + (i*7)), error);
			
			// Set the client status
			tmpClient->setInformation("status", values->at(2 + (i*7)));
			
			// Set the client Group
			tmpClient->setGroup(pGroup);

			if( tmpClient->addCLientToMap() == ERR_CLIENT_ALREADY_ON_MAP ) {
#ifdef __MONITOR_DEBUG
				qDebug() << "TaskDatabase::loadTaskGroupsClients Debug: Tried to add Client " << tmpClient->getFakeName() << " but key is already at map.";
#endif
				// Hide the Graphic entity
				ClientManager::instance()->getClientItem(tmpClient)->setHidden(true);
			}
			// Add the client to the group clients map
			pGroup->addCLientToMap(tmpClient->getName(), tmpClient);
          }
     }

     delete values;
}

bool TaskDatabase::isRunning()
{
     return d->m_database->isRunning();
}
