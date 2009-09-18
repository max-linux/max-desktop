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
 
#ifndef TaskManager_h
#define TaskManager_h

#include <QObject>

#include "Task.h"

class TaskManagerPriv;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class TaskManager : public QObject
{
Q_OBJECT
public:
    TaskManager();

    ~TaskManager();

    //=====================================================================
    // Returns a instance of the TaskManager
    //=====================================================================
    static TaskManager* instance();

    //=====================================================================
    // Returns a pointer to the current Task
    //=====================================================================
    CTask* task() const;   

    //=====================================================================
    // Find a Task in list and return a constant reference to it
    //=====================================================================
    const CTask* findTask(unsigned int id) const;

    //=====================================================================
    // Find a Task in list and return a reference to it
    //=====================================================================
    const CTask* findTask(QString &taskName);

    //=====================================================================
    // Create a new Task 
    //=====================================================================
    CTask* createTask(const QString &name, const QDate &date, QString &errMsg);

    //=====================================================================
    // Sets the current Task
    //=====================================================================
    void setCurrentTask(CTask *task);

    //=====================================================================
    // Returns the current task
    //=====================================================================
    CTask* currentTask() const;

    //=====================================================================
    // Renames a Task
    //=====================================================================
    bool renameTask(CTask *task, const QString &newName, QString &errMsg);

    //=====================================================================
    // Deletes a Task
    //=====================================================================
    bool deleteTask(CTask *task, QString &errMsg);

    //=====================================================================
    // Returns the last task
    //=====================================================================
    CTask *getLastTask(QString &error);

    //=====================================================================
    // Returns the last task ID
    //=====================================================================
    int getLastTaskID(QString &error);

    //=====================================================================
    // Returns a task from the Database
    //=====================================================================
    CTask *getTaskFromDB(int taskId, QString &error);

    //=====================================================================
    // Returns an iterator to a given task 
    //=====================================================================
    QList<CTask*>::iterator getTaskIterator(CTask *task);

    void insertTask(CTask *task);
    void redrawTask();

private:
     
    static TaskManager*  m_instance;
    TaskManagerPriv*     d;    
        
    void loadTasksFromDB();

public slots:
    void removeGroupFromTask(CGroup *group);

signals:
  
    void signalTaskAdded(CTask *task);
    void signalTaskDeleted(CTask *task);
    void signalTaskCleared(CTask *task);
    void signalTaskCurrentModified(CTask *task);
    void signalAllTasksLoaded();
    void signalTaskRenamed(CTask *task);
    void signalTaskRedraw();
};

#endif
