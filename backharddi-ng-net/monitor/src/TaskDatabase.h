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
#ifndef TaskDatabase_h
#define TaskDatabase_h

class QStringList;
class QString;
class QDate;
class TaskDBPriv;
class CGroup;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class TaskDatabase 
{
public:
     TaskDatabase();
     ~TaskDatabase();
     
     int addTask(const QString &name, const QDate &date, QString &error);
	int addTask(const QString &name, QString &error);
     void setTaskName(int taskId, const QString &name, QString &error);
     void setTaskDate(int taskId, const QDate &date, QString &error);
     
     QString getTaskName(int taskId, QString &error);
     int getTaskId(const QString &name, QString &error);
     QDate getDate(int taskId, QString &error);   
     
     
     void deleteTask(int taskId, QString &error);
     
     bool exec(const QString &sql, QString &error, const bool debug = false);
     bool select(const QString &sql,  QString &error, QStringList *const values = 0, const bool debug = false);
     
     bool isRunning();
     
     void loadTaskGroups(QString &error, unsigned int taskid);
     void loadTaskGroupsClients(QString &error, CGroup *pGroup);

private:
     QString escapeString(QString str) const;
     
     TaskDBPriv* d;
};

#endif
