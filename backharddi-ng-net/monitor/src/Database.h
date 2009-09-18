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
#ifndef Database_h
#define Database_h

#include <QtSql>

class QSqlDatabase;
class QSqlTableModel;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class CDatabase;
class CDatabase
{
public:
     CDatabase();
     virtual ~CDatabase() {}

     void StartDatabase();
     void StopDatabase();
     bool databaseInit(QString driver, QString hostname, QString database, QString username, QString password, quint16 port=6667);

     void CreateTables();
     quint32 rowsCount(QString table, QString key);

     inline bool isRunning() const {
          return m_Database.isOpen();
     }

     bool hasErrors() const;
     QSqlError lastError() const;

     QString getVersion() const;

public /*slots*/:
     void forceTransStrict();
     void update();
     QSqlDatabase getDatabase();

private:
     bool           m_isCreated;
     QSqlDatabase   m_Database;
     QSqlTableModel m_DatabaseModel;

     QString        m_HostName;
     QString        m_DatabaseName;
     QString        m_UserName;
     QString        m_Password;
     QString        m_Driver;
     quint16        m_Port;

     QString        m_Version;
};

#endif
