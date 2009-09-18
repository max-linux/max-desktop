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
#include "opentaskform.h"
#include "Database.h"
#include "TaskManager.h"
#include "TaskDatabase.h"

#include <QtSql>
#include <QtGui>
#include <QStringList>
#include <QString>

OpenTaskForm::OpenTaskForm() : QDialog()
{
     extern CDatabase *BDatabase;
     taskModel = new QSqlQueryModel(this);
     taskModel->setQuery("SELECT task_name, task_date, idtask FROM tasks", BDatabase->getDatabase());
     taskModel->setHeaderData(Task_Name, Qt::Horizontal, tr("Task Name"));
     taskModel->setHeaderData(Task_Date, Qt::Horizontal, tr("Creation Date"));

     taskTableView = new QTableView;
     taskTableView->setModel(taskModel);
     taskTableView->setWindowTitle(tr("Stored Tasks"));
     taskTableView->setSelectionMode(QAbstractItemView::SingleSelection);
     taskTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
     taskTableView->resizeColumnsToContents();

     /*groupModel = new QSqlQueryModel(this);
     groupModel->setQuery("SELECT group_name, creation_date FROM groups WHERE idtask ");*/

     loadTaskButton = new QPushButton(tr("Load Task"));
	addTaskButton = new QPushButton(tr("Add Task"));
	deleteTaskButton = new QPushButton(tr("Delete Task"));
     quitButton = new QPushButton(tr("Quit"));
     loadTaskButton->setDefault(true);

     QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical);
     buttonBox->addButton(loadTaskButton, QDialogButtonBox::ActionRole);
	buttonBox->addButton(addTaskButton, QDialogButtonBox::ActionRole);
	buttonBox->addButton(deleteTaskButton, QDialogButtonBox::ActionRole);
     buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

     mainLayout = new QHBoxLayout;
     mainLayout->addWidget(taskTableView);
     mainLayout->addWidget(buttonBox);	
	
     setLayout(mainLayout);

     createConnections();
	
	setMinimumSize(600,400);

     curIndex = -1;
}


OpenTaskForm::~OpenTaskForm()
{
}


void OpenTaskForm::createConnections()
{
     connect(loadTaskButton, SIGNAL(clicked()), this, SLOT(loadTask()));
	connect(addTaskButton, SIGNAL(clicked()), this, SLOT(addTask()));
	connect(deleteTaskButton, SIGNAL(clicked()), this, SLOT(deleteTask()));
     connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
     connect(taskTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &)));
}

void OpenTaskForm::deleteTask()
{
	if(curIndex == -1) {
		QMessageBox::information(this, tr("BackharddiNet-Monitor"), tr("Please select a valid task."));
		close();
		return;
	}

	if(TaskManager::instance()->currentTask()->getId() == static_cast<unsigned int>(curIndex)) {
		close();
		return;
	}
		
	QString error;
	TaskDatabase *taskDB = new TaskDatabase();
	taskDB->deleteTask(curIndex, error);
	
	if(!error.isEmpty()) {
		qDebug() << error;
		return;
	}
	
	disconnect(taskTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &)));
	
	extern CDatabase *BDatabase;
	taskModel = new QSqlQueryModel(this);
	taskModel->setQuery("SELECT task_name, task_date, idtask FROM tasks", BDatabase->getDatabase());
	taskModel->setHeaderData(Task_Name, Qt::Horizontal, tr("Task Name"));
	taskModel->setHeaderData(Task_Date, Qt::Horizontal, tr("Creation Date"));
		
	taskTableView->setModel(taskModel);
			
	refreshTaskViewHeader();
	
	connect(taskTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &)));
}

void OpenTaskForm::loadTask()
{
     if(curIndex == -1) {
          QMessageBox::information(this, tr("BackharddiNet-Monitor"), tr("Please select a valid task."));
          close();
          return;
     }
	
     if(TaskManager::instance()->currentTask()->getId() == static_cast<unsigned int>(curIndex)) {
          close();
          return;
     }

     TaskDatabase *taskDB = new TaskDatabase();
     QStringList *values = new QStringList();
     QString error;

     if(taskDB->select( QString("SELECT `task_name`, `task_date`, `idtask`  FROM `tasks` WHERE `idtask`=%1").arg(QString::number(curIndex)), error, values)) {
          if(values->isEmpty())
               return;

		if(TaskManager::instance()->findTask(values->at(2).toInt()) == 0) {
          	CTask *task = new CTask();

          	task->setStorageType(true);
          	task->setId(values->at(2).toInt());
          	task->setName(values->at(0));
          	task->setDate(values->at(1));

               TaskManager::instance()->insertTask(task);
			TaskManager::instance()->setCurrentTask(task);
		}
		else {
			const CTask *task = TaskManager::instance()->findTask(values->at(2).toInt());
			TaskManager::instance()->setCurrentTask(const_cast<CTask*>(task));
		}
     }

     close();

     delete taskDB;
     delete values;
}

void OpenTaskForm::refreshTaskViewHeader()
{
     taskTableView->horizontalHeader()->setVisible(taskModel->rowCount() > 0);
	taskTableView->resizeColumnsToContents();;	
}

void OpenTaskForm::addTask()
{
	TaskDatabase *taskDB = new TaskDatabase();
	bool ok;
	QString name = QInputDialog::getText(this, tr("Create new Task"), tr("Insert a name for the new task."), QLineEdit::Normal, "", &ok);
	QString error;
	if(ok && !name.isEmpty()) {
		if(taskDB->addTask(name, error) >=0) {
			
			disconnect(taskTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &)));
			
			extern CDatabase *BDatabase;
			taskModel = new QSqlQueryModel(this);
			taskModel->setQuery("SELECT task_name, task_date, idtask FROM tasks", BDatabase->getDatabase());
			taskModel->setHeaderData(Task_Name, Qt::Horizontal, tr("Task Name"));
			taskModel->setHeaderData(Task_Date, Qt::Horizontal, tr("Creation Date"));
		
			taskTableView->setModel(taskModel);
			
			refreshTaskViewHeader();
			
			connect(taskTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &)));
		}
	}
	
	delete taskDB;
}

void OpenTaskForm::currentChanged(const QModelIndex &index)
{
     if (index.isValid()) {
          QSqlRecord record = taskModel->record(index.row());
          curIndex = record.value("idtask").toInt();
     }
     else
          curIndex = -1;
     
     refreshTaskViewHeader();
}

void OpenTaskForm::rename(const QModelIndex &index)
{
	if(index.isValid()) {
		QSqlRecord record = taskModel->record(index.row());
		curIndex = record.value("idtask").toInt();
		
		bool ok;
		QString name = QInputDialog::getText(this, tr("Change task name"), tr("Input the new name for the task."), QLineEdit::Normal, "", &ok);
		
		if(ok && !name.isEmpty()) {
			QString error;
			TaskDatabase *taskDB = new TaskDatabase();
			taskDB->setTaskName(record.value("idtask").toInt(), name, error);
			
			if(!error.isEmpty()) {
				return;
			}
			
			disconnect(taskTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &)));
			
			extern CDatabase *BDatabase;
			taskModel = new QSqlQueryModel(this);
			taskModel->setQuery("SELECT task_name, task_date, idtask FROM tasks", BDatabase->getDatabase());
			taskModel->setHeaderData(Task_Name, Qt::Horizontal, tr("Task Name"));
			taskModel->setHeaderData(Task_Date, Qt::Horizontal, tr("Creation Date"));
		
			taskTableView->setModel(taskModel);
			
			refreshTaskViewHeader();
			
			connect(taskTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &)));
		}
	}
	else
		curIndex = -1;
	
	refreshTaskViewHeader();
}


