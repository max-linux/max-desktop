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
#include <QStringList>

#include "TaskManager.h"
#include "TaskDatabase.h"
#include "groupmanager.h"
#include "Task.h"
#include "cgroup.h"
#include "Loader.h"

#include <QDate>

typedef QList<CTask*>    PTaskList;

class TaskManagerPriv
{
  public:
    CTask           *task;
    TaskDatabase    *taskDB;

    bool            modified;

    CTask           *currentTask;
    QStringList     dirtyTasks;

    PTaskList       pTaskList;
};

TaskManager* TaskManager::m_instance = 0;

TaskManager* TaskManager::instance()
{
     return m_instance;
}

TaskManager::TaskManager()
{
     m_instance     = this;

     d = new TaskManagerPriv;

     d->task = new CTask;
     d->taskDB = new TaskDatabase();
     d->currentTask = 0;

     d->modified = false;
}

TaskManager::~TaskManager()
{
     delete d->task;
     delete d->taskDB;
	d->pTaskList.clear();
     delete d;

     m_instance = 0;
}

CTask* TaskManager::task() const
{
     return d->task;
}

void TaskManager::setCurrentTask(CTask *task)
{
	if(!task) {
		d->currentTask = 0;
		emit signalTaskCurrentModified(0);
		return;
	}
	
	extern Loader *settings;
     if(d->currentTask) {
          d->currentTask->setState(STATUS_WAITING, true);
          d->currentTask->setState(STATUS_ACTIVE, false);
		d->currentTask->setState(STATUS_RUNNING, false);
     }

     d->currentTask = task; 
     task->setState(STATUS_WAITING, false);
     task->setState(STATUS_ACTIVE, true);
	d->currentTask->setState(STATUS_RUNNING, false); 
	
	settings->setKey("LastTask/taskId", task->getId());
	
     emit signalTaskCurrentModified(task);
}

CTask* TaskManager::currentTask() const
{
     return d->currentTask;
}

const CTask* TaskManager::findTask(unsigned int id) const
{
     for(int i=0; i < d->pTaskList.size(); ++i) 
     {
          if(d->pTaskList.at(i)->getId() == id) 
          {
               return d->pTaskList.at(i);
          }
     }

     return 0;
}

const CTask* TaskManager::findTask(QString &taskName)
{
  for(int i=0; i < d->pTaskList.size(); ++i) 
  {
    if(d->pTaskList.at(i)->getName() == taskName) 
    {
      return d->pTaskList.at(i);
    }
  }

  return 0;
}

CTask* TaskManager::createTask(const QString &name, const QDate &date, QString &errMsg)
{
     if(name.isEmpty())
     {
          errMsg = tr("Task name cannot be empty.");
          return 0;
     }

     if(name.contains("[',;--]"))
     {
          errMsg = tr("You used illegal characters on name.");
          return 0;
     }

     PTaskList::iterator i = d->pTaskList.begin();
     while(i != d->pTaskList.end()) 
     {
          if( (*i)->getName() == name )
          {
               errMsg = tr("An existing task has the same name.");
               return 0;
          }
          i++;
     }

     CTask *newTask = new CTask();     
     newTask->setDate(date.toString("yyyy-mm-dd") , QString("yyyy-mm-dd"));     
     newTask->setName(name);
     newTask->setState(STATUS_PRELOAD, true);
     int taskid = d->taskDB->addTask(name, date, errMsg);
     if(taskid == -1) 
          return NULL;

     newTask->setId(taskid);

     return newTask;
}

CTask *TaskManager::getLastTask(QString &error)
{     
     extern Loader *settings;
     int taskid = settings->getKey("LastTask/taskId").toInt();

     if(taskid != 0) {
		CTask *lastTask = getTaskFromDB(taskid, error);
		
		if(lastTask != NULL)
          	return lastTask;
     }

	if(error.isEmpty())
     	error = "Last task doesn't exists.";
     
	return 0;
}

int TaskManager::getLastTaskID(QString &error)
{
	extern Loader *settings;
	int taskid = settings->getKey("LastTask/taskId").toInt();
	
	if(taskid == 0)
		error = "Last task doesn't exists.";
	
	return taskid;
}

CTask *TaskManager::getTaskFromDB(int taskId, QString &error)
{	
	if(taskId != 0) {
		QStringList *values = new QStringList();
		QString error;
		
		if(d->taskDB->select(QString("SELECT `task_name`, `task_date`, `idtask`  FROM `tasks` WHERE `idtask`=%1").arg(taskId), error, values))
		{
			if(values->isEmpty()) {
				error.append("Error, no Task register found at Database.");
				return NULL;
			}

			CTask *task = new CTask();

			task->setStorageType(true);
			task->setId(values->at(2).toInt());
			task->setName(values->at(0));
			task->setDate(values->at(1));
			
			return task;
		}
	}
	
	error.append("Error, task ID can not be 0");
	return NULL;
}

bool TaskManager::renameTask(CTask *task, const QString &newName, QString &errMsg)
{
     if(!task) {
          errMsg = tr("The task pointer is not a valid pointer.");
          return false;
     }
     
     if(newName.isEmpty()) {
          errMsg = tr("The task new name can't be empty.");
          return false;
     }

     QString tmpName = newName;

     if(findTask(tmpName) == 0) {
          task->setName(newName);
		d->taskDB->setTaskName(task->getId(), newName, errMsg);
          return true;
     }
     else {
          errMsg = QString("Can't rename task %1 because already exists a task called %2")
              .arg(task->getName(), newName);
          errMsg = tr(errMsg.toStdString().c_str());

          return false;
     }

     return false;  // Will be not reached
}

bool TaskManager::deleteTask(CTask *task, QString &errMsg)
{
     if(!task) {
          errMsg = tr("The task pointer is not a valid pointer.");
          return false;
     }

     if(!d->pTaskList.contains(task)) {
          errMsg = tr("The task doesn't exist on task list.");
          return false;
     }

     d->pTaskList.removeAt(task->getId());
     if(task == d->currentTask) {
          d->currentTask = 0;
          emit signalTaskCurrentModified(0);
     }

     emit signalTaskDeleted(task);
     delete task;

     return true;
}

QList<CTask*>::iterator TaskManager::getTaskIterator(CTask *task)
{
     if(!task) 
          return 0;

     QList<CTask*>::iterator i = d->pTaskList.begin();
     while(i != d->pTaskList.end()) {
       if( (*i)->getId() == task->getId() ) {
               return i;
       }
       i++;
     }

     return 0;
}

void TaskManager::insertTask(CTask *task) 
{
     d->pTaskList.append(task);
     task->setState(STATUS_LOADED, true);
     task->setState(STATUS_WAITING, true);

     TaskDatabase *mDatabase = new TaskDatabase();

     if(!mDatabase->isRunning()) {
          qDebug() << "Error loading the Task Groups, the error Message was: The Database is not running."; 
          delete mDatabase;
          return;
     }

     QString error = "";
     mDatabase->loadTaskGroups(error, task->getId());

     if(!error.isEmpty()) {
         qDebug() << "Error loading the Task Groups, the error Message was: " << error; 
     }

     delete mDatabase;
}

void TaskManager::removeGroupFromTask(CGroup *group)
{
	d->currentTask->removeGroupFromMap(group->getName());
}

void TaskManager::redrawTask()
{
	emit signalTaskRedraw();
}


