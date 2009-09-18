/***************************************************************************
 *   Copyright (C) 2007 by Oscar Campos Ruiz-Adame   *
 *   oscar.campos@edmanufacturer.es   *
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
#include "Database.h"

#include <QtSql>
#include <QFile>
#include <QByteArray>
#include <QDateTime>

#include "Loader.h"

extern Loader *settings;

CDatabase::CDatabase()
{     
}

void CDatabase::StopDatabase()
{
     m_Database.close();     
     m_Database.removeDatabase("Backharddi");
     m_isCreated = false;
}

void CDatabase::StartDatabase()
{
     if(!m_isCreated) {
          m_isCreated = databaseInit(settings->getKey("SQL/driver").toString(),
               settings->getKey("SQL/hostName").toString(),
               settings->getKey("SQL/database").toString(),
               settings->getKey("SQL/userName").toString(),
               settings->getKey("SQL/password").toString(),
               settings->getKey("SQL/port").toInt()
          );
    }
}

bool CDatabase::databaseInit(QString driver, QString hostname, QString database, QString username, QString password, quint16 port)
{  
     m_Database = QSqlDatabase::addDatabase(driver, "Backharddi");     
     m_Database.setHostName(hostname);
     if(driver.contains("SQLITE")) 
          m_Database.setDatabaseName(database.prepend(QDir::homePath().append("/.")));
     else
          m_Database.setDatabaseName(database);
     
     m_Database.setUserName(username);
     m_Database.setPassword(password);
     m_Database.setPort((int)port);
     
     return m_Database.open();     // Return lastError if connection fails
}

void CDatabase::CreateTables()
{
     if(m_isCreated) {
          if(m_Database.driverName().contains("MYSQL")) {
               QString sql;
		
	       sql = "CREATE TABLE IF NOT EXISTS `groups_configuration` ("
   			"`idconfig` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,"
			"`idgroup` INT(10) UNSIGNED NOT NULL,"
			"`modo` TINYINT UNSIGNED NOT NULL,"
	   		"`imagenes` TEXT  NOT NULL,"
    			"PRIMARY KEY(`idconfig`)"
			")ENGINE=InnoDB DEFAULT CHARSET=latin1";
			
		m_Database.exec(sql);

               sql = "CREATE TABLE IF NOT EXISTS `clients` ("
                       "`idclient` varchar(36) NOT NULL, "
                       "`client_name` text NOT NULL, "
                       "`creation_date` date NOT NULL, "
                       "`client_status` tinyint(1) NOT NULL default '0', "
				   "`client_mac` text NOT NULL, "
                       "PRIMARY KEY  (`idclient`)"
                       ") ENGINE=InnoDB DEFAULT CHARSET=latin1";
          
               m_Database.exec(sql);
          
               sql = "CREATE TABLE IF NOT EXISTS`groups` ("
                       "`idgroup` int(10) unsigned NOT NULL auto_increment,"
                       "`group_name` text NOT NULL,"
                       "`creation_date` date NOT NULL,"
                       "`idtask` int(10) unsigned NOT NULL default '0',"
				   "`configured` bool default false,"
                       "PRIMARY KEY  (`idgroup`)"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Groups configuration at Backharddi'";
               m_Database.exec(sql);
          
               sql = "CREATE TABLE IF NOT EXISTS`groups_relation` ("
                         "`idrelation` int(10) unsigned NOT NULL auto_increment,"
                         "`idgroup` int(10) unsigned NOT NULL,"
                         "`idclient` char(36) NOT NULL,"
                         "PRIMARY KEY  (`idrelation`)"
                         ") ENGINE=InnoDB DEFAULT CHARSET=latin1";
               m_Database.exec(sql);
          
               sql = "CREATE TABLE IF NOT EXISTS `log` ("
                         "`idlog` int(10) unsigned NOT NULL auto_increment,"
                         "`success_type` tinyint(3) unsigned NOT NULL,"
                         "`success_title` text NOT NULL,"
                         "`success_desc` text NOT NULL,"
                         "`success_date` datetime NOT NULL,"
                         "PRIMARY KEY  (`idlog`)"
                         ") ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED";
               m_Database.exec(sql);

               sql = "CREATE TABLE IF NOT EXISTS `log_relation` ("
                         "`idrelation` int(10) unsigned NOT NULL auto_increment,"
                         "`idlog` int(10) unsigned NOT NULL,"
                         "`idgroup` int(10) unsigned NOT NULL default '0',"
                         "`idclient` char(36) NOT NULL,"
                         "PRIMARY KEY  (`idrelation`)"
                         ") ENGINE=InnoDB DEFAULT CHARSET=latin1";
               m_Database.exec(sql);

               sql = "CREATE TABLE IF NOT EXISTS`tasks` ("
                         "`idtask` int(10) unsigned NOT NULL auto_increment,"
                         "`task_date` date default NULL,"
                         "`task_name` varchar(32) default NULL,"
                         "PRIMARY KEY  (`idtask`)"
                         ") ENGINE=InnoDB DEFAULT CHARSET=latin1";
               m_Database.exec(sql);

               sql = QString("INSERT INTO `tasks` (`task_date`, `task_name`) VALUES(NOW(), 'Default')");
               m_Database.exec(sql);
          }
          else {            
               QString sql = "CREATE TABLE IF NOT EXISTS `clients` ("
							"`idsqlitekey` INTEGER PRIMARY KEY,"
                                   "`idclient` TEXT NOT NULL UNIQUE,"
                                   "`client_name` TEXT NOT NULL,"
                                   "`creation_date` TEXT NOT NULL,"
                                   "`client_status` INTEGER NOT NULL default '0',"
							"`client_mac` TEXT NOT NULL) ";
               m_Database.exec(sql);

               sql = "CREATE TABLE IF NOT EXISTS`groups` ("
                         "`idgroup` INTEGER PRIMARY KEY,"
                         "`group_name` TEXT NOT NULL,"
                         "`creation_date` TEXT NOT NULL,"
                         "`idtask` INTEGER NOT NULL default '0',"
					"`configured` INTEGER default '0')";
               m_Database.exec(sql);

               sql = "CREATE TABLE IF NOT EXISTS`groups_relation` ("
                         "`idrelation` INTEGER PRIMERY KEY,"
                         "`idgroup` INTEGER NOT NULL,"
                         "`idclient` TEXT NOT NULL)";
               m_Database.exec(sql);

               sql = "CREATE TABLE IF NOT EXISTS `log` ("
                         "`idlog` INTEGER PRIMARY KEY,"
                         "`success_type` INTEGER NOT NULL,"
                         "`success_title` TEXT NOT NULL,"
                         "`success_desc` TEXT NOT NULL,"
                         "`success_date` TEXT NOT NULL)";

               m_Database.exec(sql);

               sql = "CREATE TABLE IF NOT EXISTS `log_relation` ("
                         "`idrelation` INTEGER PRIMARY KEY,"
                         "`idlog` INTEGER NOT NULL,"
                         "`idgroup` INTEGER NOT NULL default '0',"
                         "`idclient` TEXT NOT NULL)";

               m_Database.exec(sql);

               sql = "CREATE TABLE IF NOT EXISTS`tasks` ("
                         "`idtask` INTEGER PRIMARY KEY,"
                         "`task_date` TEXT default NULL,"
                         "`task_name` TEXT default NULL)";

               m_Database.exec(sql);
			
			sql = "CREATE TABLE IF NOT EXISTS `groups_configuration` ("
					"`idconfig` INTEGER PRIMARY KEY,"
					"`idgroup` INTEGER NOT NULL,"
					"`modo` INTEGER NOT NULL,"
					"`imagenes` text NOT NULL)";
			m_Database.exec(sql);
		
               
               sql = QString("INSERT INTO `tasks` (`task_date`, `task_name`) VALUES('%1', 'Default')").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));                  
               m_Database.exec(sql);
          }         
     }
}

bool CDatabase::hasErrors() const 
{
     return (m_isCreated) ? false : true;
}

QSqlError CDatabase::lastError() const
{
     return m_Database.lastError();
}

QString CDatabase::getVersion() const
{
     QSqlQuery result = m_Database.exec("SELECT VERSION()");
     
     if(!result.isActive())
          return m_Database.lastError().text();
     
     return result.value(0).toString();
}

void CDatabase::forceTransStrict()
{
     if(m_Database.driverName().contains("MYSQL")) {          
          if(getVersion().contains("5.")) {
               qDebug() << "Setting transt strict tables as SQL MODE MySQL >=5.0";
               m_Database.exec("SET sql_mode='STRICT_TRANS_TABLES'");
          }
     }
}

void CDatabase::update()
{
     extern Loader *settings;     
     m_Database.setUserName(settings->getKey("SQL/userName").toString());
     m_Database.setHostName(settings->getKey("SQL/hostName").toString());
     m_Database.setPassword(settings->getKey("SQL/password").toString());
     m_Database.setDatabaseName(settings->getKey("SQL/database").toString());
     m_Database.setPort(settings->getKey("SQL/port").toInt());   
}

QSqlDatabase CDatabase::getDatabase() {
     return m_Database;
}

quint32 CDatabase::rowsCount(QString table, QString key) 
{
     if(table.isEmpty() || key.isEmpty())
          return 0;

     QString sql = QString("SELECT COUNT(").append(key).append(") AS count FROM `").append(table).append("`");
     QSqlQuery query = m_Database.exec(sql);
     if(!query.isActive()) {
          return 0;
     }
     else {
          int numCols = 0;
          while(query.next()) {
               numCols = query.record().count();
          }

          return numCols;
     }

     return 0;
}
