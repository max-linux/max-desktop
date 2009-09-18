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
#ifndef OPENTASKFORM_H
#define OPENTASKFORM_H

#include <QDialog>

class QSqlQueryModel;
class QPushButton;
class QTableView;
class QSqlRecord;
class QModelIndex;
class TaskDatabase;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class OpenTaskForm : public QDialog
{
Q_OBJECT
public:
     OpenTaskForm();
     ~OpenTaskForm();
    
private slots:
     void addTask();
     void deleteTask();
     void loadTask();
     void refreshTaskViewHeader();
     void currentChanged(const QModelIndex &index);
	void rename(const QModelIndex &index);

private:
     void createConnections();
     
     enum {
          Task_Name, Task_Date
     };
     
     QSqlQueryModel *taskModel;
     QSqlQueryModel *groupModel;
     QTableView *taskTableView;
     QTableView *groupTableView;
     QPushButton *addTaskButton;
     QPushButton *deleteTaskButton;
     QPushButton *loadTaskButton;
     QPushButton *quitButton;
	QHBoxLayout *mainLayout;
     int curIndex;
};

#endif
