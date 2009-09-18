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


#include <QtUiTools>
#include <QtGui>
#include <QtCore>
#include <QtGui/QAction>
#include <QDate>

#include "backharddinet_monitor.h"
#include "ui_MainForm.h"
#include "TaskManager.h"
#include "Loader.h"
#include "preferencesdialog.h"
#include "Database.h"
#include "opentaskform.h"
#include "generaltab.h"
#include "taskstree.h"
#include "cclient.h"
#include "clientdatabase.h"
#include "MonitorServer.h"
#include "clientslist.h"
#include "clientstree.h"
#include "clientmanager.h"
#include "groupslist.h"
#include "groupmanager.h"
#include "groupconfigform.h"
#include "shellwidget.h"

extern MonitorServer *PServer;

class BackharddiNet_MonitorPriv
{
public:
     BackharddiNet_MonitorPriv()
     {     
          m_initialTaskId     = 0;
          m_taskManager       = 0;
     }
     
     int            m_initialTaskId;     
     TaskManager    *m_taskManager;    
};

BackharddiNet_Monitor::BackharddiNet_Monitor() : QMainWindow()
{
     QString error = "";
     d = new BackharddiNet_MonitorPriv;
     d->m_taskManager = TaskManager::instance();
     d->m_initialTaskId = d->m_taskManager->getLastTaskID(error);
     if(!error.isEmpty()) 
          qDebug() << error;

	m_Modified = FALSE;	
	setupUi(this);

     QRect rect(0, 0, 1024, 600);
     rect.moveCenter(QApplication::desktop()->screenGeometry(QApplication::desktop()->primaryScreen()).center());
     this->setGeometry(rect);
     this->setMinimumSize(1024, 600);

     // Hide the tab 
     monitorTab->hide();
     monitorTab->removeTab(0);
	monitorTab->setUsesScrollButtons(true);


     // Create tray icon
     createActions();
     createTrayIcon();

	// ShortCuts
	CreateShortCuts();

     setIcon();

     CreateConnections();

     trayIcon->show();

	// Clients list Widgets
	ClientManager::instance()->setGraphicList(dockClients);
	clientsListWidget = ClientManager::instance()->getGraphicList();
	dockClients->setWidget(clientsListWidget);	
	
	// Groups list Widgets		
	QWidget *groupsWidget = new QWidget(dockGroups);
	GroupsList *groupsListWidget = new GroupsList(groupsWidget);	
	QVBoxLayout *groupsLayout = new QVBoxLayout(groupsWidget);
	groupsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);	
	groupsListWidget->setDragEnabled(false);
	groupsListWidget->setAcceptDrops(false);	
	groupsLayout->addWidget(groupsListWidget);
	groupsWidget->setLayout(groupsLayout);
	dockGroups->setWidget(groupsWidget);	

     // Status Bar
     statusbar->showMessage(tr("Backharddi-Net Monitor (MoJITO 1.0) is Ready"));
}

QWidget* BackharddiNet_Monitor::loadUiFile()
{
	QUiLoader loader;
	
	QFile file(":/MainForm.ui");
	file.open(QFile::ReadOnly);
	
	QWidget *formWidget = loader.load(&file, this);
	file.close();

	return formWidget;
}

void BackharddiNet_Monitor::About() 
{
	QMessageBox::about(this, tr("About BackharddiNet_Monitor"),
		tr("<b>BackharddiNet_Monitor</b> is the server side frontend "
		   "to manage the backharddi net activity.<br/><br/>"
		   "You can use the monitor to create or manage new multicast"
		   "groups, reboot machines and much more things."));	
}

void BackharddiNet_Monitor::DocumentModified() 
{
	m_Modified = TRUE;
}

void BackharddiNet_Monitor::open() 
{
	if(!maybeSave()) {
		QString fileName;
		QString dir("./");
		QFileDialog dialog(this);
		dialog.setFileMode(QFileDialog::ExistingFile);
		dialog.setFilter(tr("Backharddi Net Files (*.bnm)"));
		dialog.setViewMode(QFileDialog::Detail);
		dialog.setDirectory(dir);
		
		if(dialog.exec()) {
			QStringList files = dialog.selectedFiles();
			fileName = files[0];
		}
			
		if(!fileName.isEmpty())
			loadFile(fileName);
	}
}

void BackharddiNet_Monitor::openSQL() 
{
	if(TaskManager::instance()->currentTask() && TaskManager::instance()->currentTask()->getState(STATUS_RUNNING)) {
		QMessageBox::critical(this, "Backharddi Net", tr("The task you are trying to close, contains throwed group(s), you can not close it while those groups are running."), QMessageBox::Ok);
		return;
	}
	
     if(!maybeSave()) {
          OpenTaskForm *openTask = new OpenTaskForm();
          openTask->exec();
          delete openTask;
     }
	
	if(TaskManager::instance()->currentTask()) {
		actionCloseTask->setEnabled(true);
	}
}

bool BackharddiNet_Monitor::IsModified() 
{
	return m_Modified;
}

bool BackharddiNet_Monitor::maybeSave()
{
	if (IsModified()) {
		int ret = QMessageBox::warning(this, tr("Backharddi Net"
		"Monitor"),
	       	tr("This task group configuration has been modified.\n"
		       "Do you want to save your changes?"),
	      	QMessageBox::Yes | QMessageBox::Default,
       		QMessageBox::No,
       		QMessageBox::Cancel | QMessageBox::Escape);
		if (ret == QMessageBox::Yes)
			return save();
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	return false;
}

void BackharddiNet_Monitor::loadFile(const QString &filename) 
{	
	Q_UNUSED(filename);
     showMessage(tr("Backharddi Tips"), tr("This function is not implemented yet.\nVisit us at: http://backharddi.ideseneca.es"));
}

bool BackharddiNet_Monitor::save() 
{
	return saveAs();
}

bool BackharddiNet_Monitor::saveAs() 
{
     extern CDatabase *BDatabase;     
     
     if(BDatabase->isRunning()) {
          if(d->m_taskManager->currentTask()->getStorageType()) {
               bool ok;
               QString text = QInputDialog::getText(this, tr("Backharddi-Net Monitor"), tr("Task Name:"), QLineEdit::Normal, QDir::home().dirName(), &ok);
               
               if(ok && !text.isEmpty()) {
                    QString error;
                    d->m_taskManager->createTask(text, QDate::currentDate(), error);
                    if(!error.isEmpty()) {
					
                    }
               } 
          }
     }
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Config Task"), "./", tr("Backharddi Net Files (*.bnm)"));
	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

bool BackharddiNet_Monitor::saveFile(const QString &fileName) 
{
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Backharddi Net Monitor"),
				     tr("Cannot write file %1:\n%2.")
						     .arg(fileName)
						     .arg(file.errorString()));
		return false;
	}	

	QTextStream out(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QApplication::restoreOverrideCursor();

	statusBar()->showMessage(tr("File saved"), 2000);
     m_Modified = FALSE;
	return true;
}

void BackharddiNet_Monitor::createTrayIcon()
{
     trayIconMenu = new QMenu(this);
     trayIconMenu->addAction(minimizeAction);
     trayIconMenu->addAction(maximizeAction);
     trayIconMenu->addAction(restoreAction);
     trayIconMenu->addSeparator();
     trayIconMenu->addAction(quitAction);

     trayIcon = new QSystemTrayIcon(this);
     trayIcon->setContextMenu(trayIconMenu);
     trayIcon->setToolTip(tr("Backharddi Monitor is currently running."));
}

void BackharddiNet_Monitor::createActions()
{
     
	minimizeAction = new QAction(tr("Mi&nimize"), this);
     connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

     maximizeAction = new QAction(tr("Ma&ximize"), this);
     connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

     restoreAction = new QAction(tr("&Restore"), this);
     connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

	quitAction = new QAction(QIcon(QString::fromUtf8(":/icons/exit.png")), tr("&Quit"), this);
     connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void BackharddiNet_Monitor::setVisible(bool visible)
{
     minimizeAction->setEnabled(visible);
     maximizeAction->setEnabled(!isMaximized());
     restoreAction->setEnabled(isMaximized() || !visible);
     QWidget::setVisible(visible);
}

void BackharddiNet_Monitor::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
     switch(reason) {
          case QSystemTrayIcon::Trigger:
			(isHidden()) ? showNormal() : hide();
			break;
          case QSystemTrayIcon::DoubleClick:
               break;
          case QSystemTrayIcon::MiddleClick:
               showMessage(tr("BackharddiNet-Monitor"), tr("http://backharddi.ideseneca.es"));
               break;
          default:
               ;
     }
}

void BackharddiNet_Monitor::showMessage(const QString title, const QString msg)
{
     trayIcon->showMessage(title, msg);
}

void BackharddiNet_Monitor::closeEvent(QCloseEvent *event)
{     
     if(trayIcon->isVisible()) {
          hide();
          event->ignore();
     }
}

void BackharddiNet_Monitor::setIcon()
{
     QIcon icon(":/icons/MainWindow/pango128x128.png");   
     trayIcon->setIcon(icon);
     setWindowIcon(icon);    
}

void BackharddiNet_Monitor::messageClicked()
{     
     showMaximized();
}

void BackharddiNet_Monitor::CreateConnections()
{
     // Connections
     connect(actionAbout, SIGNAL(triggered()), this, SLOT(About()));
     connect(actionAbout_Qt4, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
     connect(actionLoad, SIGNAL(triggered()), this, SLOT(open()));   
     connect(actionLoadSQL, SIGNAL(triggered()), this, SLOT(openSQL()));  
     connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));
     connect(actionRemove, SIGNAL(triggered()), this, SLOT(deleteGroup()));
     connect(actionThrow, SIGNAL(triggered()), this, SLOT(throwGroup()));
	connect(actionEdit, SIGNAL(triggered()), this, SLOT(editGroupDialog()));
	connect(actionReboot, SIGNAL(triggered()), this, SLOT(rebootGroup()));
	connect(actionCloseTask, SIGNAL(triggered()), this, SLOT(closeTask()));

     // TaskManager Connections
     connect(d->m_taskManager, SIGNAL(signalTaskCurrentModified(CTask *)), this, SLOT(redrawTasksWidgetTab(CTask *)));
	
	// GroupManager Connections
	connect(GroupManager::instance(), SIGNAL(signalConfigureGroup(CGroup *)), this, SLOT(newGroupDialog(CGroup *)));	

	// Docks
	connect(ClientManager::instance(), SIGNAL(signalDrawClient(CClient*)), this, SLOT(addClientToDockBar(CClient*)));
	connect(ClientManager::instance(), SIGNAL(signalClientDeleted(CClient*)), this, SLOT(removeClientFromDockBar(CClient*)));
	
	connect(PServer, SIGNAL(clientAdded(CClient*)), this, SLOT(saveToDatabase(CClient*)));

     // Preferences
     connect(actionSQLConfig, SIGNAL(triggered()), this, SLOT(prefsSqlDialog()));
     connect(actionTaskConfig, SIGNAL(triggered()), this, SLOT(prefsTaskDialog()));

     // SystemTrayIcon
     connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
     connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void BackharddiNet_Monitor::closeTask()
{	
	if(TaskManager::instance()->currentTask()->getState(STATUS_RUNNING)) {
		QMessageBox::critical(this, "Backharddi Net", tr("The task you are trying to close, contains throwed group(s), you can not close it while those groups are running."), QMessageBox::Ok);
		return;
	}
	
	QString error;
	TaskManager::instance()->setCurrentTask(NULL);
	actionCloseTask->setEnabled(false);
}

void BackharddiNet_Monitor::prefsSqlDialog()
{
	if(TaskManager::instance()->currentTask()) {
		QMessageBox::critical(this, tr("Backharddi-Net Monitor"), tr("You can not configure the SQL Storage while there are Tasks loaded.\nPlease, shutdown the application and try to configure before to load any task."), QMessageBox::Ok);
		
		return;
	}
     PreferencesSQLDialog *dialog = new PreferencesSQLDialog(this);
     dialog->show();
}

void BackharddiNet_Monitor::prefsTaskDialog()
{     
     qDebug() << "Task Prefs display";
     return;
}

void BackharddiNet_Monitor::redrawTasksWidgetTab(CTask *task)
{
	if(task == NULL) {
		monitorTab->hide();
		return;
	}
	
	if(monitorTab->isHidden())
          monitorTab->show(); 
	
	if(!menu_Config->isEnabled())
		menu_Config->setEnabled(true);
	
	if(!menu_Run->isEnabled())
		menu_Run->setEnabled(true);

     if(!monitorTab->isTabEnabled(0)) {
          monitorTab->insertTab(0, new GeneralTab(task), tr("General"));
     }
     else {
          delete monitorTab->widget(0);
          monitorTab->insertTab(0, new GeneralTab(task), tr("General"));
     }

     QString msg;
     msg.append("Task ").append(task->getName()).append(" loaded.");

     statusbar->showMessage(tr(msg));
}

void BackharddiNet_Monitor::addRemoteShellToClient(CClient *client)
{
	if(!client)
		return;
		
	ShellWidget *sh = new ShellWidget(this, client->getName(), client->getID());	
	sh->setHistorySize(2500);	
	
	QString cmd = "export TERM=\"linux\"\n";
	sh->sendText(cmd);
	cmd = "ssh root@" + client->getInformation("ip_address") + " -o StrictHostKeyChecking=no\n";
	sh->sendText(cmd);
	cmd = "clear";
	sh->sendText(cmd);
			
	
	
	client->setStatus(CLIENT_STATUS_SHELL_ATTACHED, true);
	client->setShell(sh, monitorTab->count() + 1);
	sh->setClient(client->getName());
	
	QString str = client->getFakeName() + " " + tr("Shell");
	
	monitorTab->setTabIcon(monitorTab->count() + 1, QIcon(":/icons/konsole.png"));
	monitorTab->insertTab(monitorTab->count() + 1, sh, str);
	monitorTab->setCurrentWidget(monitorTab->widget(monitorTab->count() + 1));
	monitorTab->setIconSize(QSize(16, 16));
	
	connect(sh, SIGNAL(finished(QTermWidget *)), this, SLOT(closeTab(QTermWidget *)));
}

bool BackharddiNet_Monitor::removeRemoteShellFromClient(CClient *client)
{
	if(!client)
		return false;		
	
	client->unsetShell();
	client->setStatus(CLIENT_STATUS_SHELL_ATTACHED, false);
	return true;
}

void BackharddiNet_Monitor::CreateShortCuts()
{
	// File
 	actionLoadSQL->setShortcut(tr("Ctrl+O"));
	actionQuit->setShortcut(tr("Ctrl+Q"));
	
	// Edit
	actionEdit->setShortcut(tr("Ctrl+E"));
	actionRemove->setShortcut(tr("Ctrl+R"));
	
	// Run
	actionThrow->setShortcut(tr("Ctrl+T"));
	actionThrow_All->setShortcut(tr("Ctrl+Shift+T"));
	actionReboot->setShortcut(tr("Ctrl+B"));
	actionReboot_All->setShortcut(tr("Ctrl+Shift+B"));
	
	// Help
	actionAbout->setShortcut(tr("F1"));
}

void BackharddiNet_Monitor::addClientToDockBar(CClient *client)
{
	if(client->getGroup() != NULL)
		ClientManager::instance()->getClientItem(client)->setHidden(true);
	
	clientsListWidget->getList()->addItem(ClientManager::instance()->getClientItem(client));
}

void BackharddiNet_Monitor::removeClientFromDockBar(CClient *client)
{
	clientsListWidget->getList()->removeItemWidget(ClientManager::instance()->getClientItem(client));	
}

ClientsList *BackharddiNet_Monitor::takeClientsList()
{
	return clientsListWidget;
}

void BackharddiNet_Monitor::saveToDatabase(CClient *client)
{
	ClientDatabase *cDatabase = new ClientDatabase();
	QString error;
	
	if(cDatabase->getClientName(QString(client->getID()), error).isEmpty()) {
		if(cDatabase->addClient( client->getFakeName(), QDate::fromString(client->getInformation("date"), "yyyymmdd"), QString(client->getID()), client->getName(), error ) == -1) {
			qDebug() << "Debug: Error inserting client " << client->getFakeName() << "to the database, the error string is:\n" << error;
		}
	}
	
	delete cDatabase;
}

void BackharddiNet_Monitor::newGroupDialog(CGroup *group)
{
	if(group == NULL) {
		QMessageBox::critical(this, tr("Backharddi-Net Monitor"), tr("The group will exists already."), QMessageBox::Ok);
		return;
	}
		
	GroupConfigForm *form = new GroupConfigForm(group, false, this);
	form->exec();
	delete form;
}

void BackharddiNet_Monitor::editGroupDialog()
{
	if((monitorTab->widget(0) != NULL) && (monitorTab->widget(0) == monitorTab->currentWidget())) {
		GeneralTab *general = static_cast<GeneralTab*>(monitorTab->currentWidget());
		TasksTree *tree = general->getTree();
		QTreeWidgetItemIterator it(tree, QTreeWidgetItemIterator::Selected);
		
		if(!*it) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To edit a group from the current task you will select it first."), QMessageBox::Ok);
			return;
		}
		
		QTreeWidgetItem *tmpItem = (*it);
		if(tmpItem->data(4,0).toString() != "group") {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To edit a group from the current task you will select it first."), QMessageBox::Ok);
			return;
		}
		
		CGroup *group = GroupManager::instance()->findGroup(tmpItem->text(1));
		
		if(group->getStatus(CGroup::STATUS_THROWED)) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("This group is throwed, you can not edit a throwed group.\nYou will wait until the performed action finish."), QMessageBox::Ok);
			return;
		}
		
		GroupConfigForm *form = new GroupConfigForm(group, true, this);
		form->exec();
		delete form;
	}
}

void BackharddiNet_Monitor::deleteGroup()
{
	QString error;
	if((monitorTab->widget(0) != NULL) && (monitorTab->widget(0) == monitorTab->currentWidget())) {
		GeneralTab *general = static_cast<GeneralTab*>(monitorTab->currentWidget());
		TasksTree *tree = general->getTree();
		QTreeWidgetItemIterator it(tree, QTreeWidgetItemIterator::Selected);
		
		if(!*it) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To delete a group from the current task you will select it first."), QMessageBox::Ok);
			return;
		}
		
		QTreeWidgetItem *tmpItem = (*it);
		if(tmpItem->data(4,0).toString() != "group") {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To delete a group from the current task you will select it first."), QMessageBox::Ok);
			return;
		}
		
		CGroup *group = GroupManager::instance()->findGroup(tmpItem->text(1));
		
		if(group->getStatus(CGroup::STATUS_THROWED)) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("This group is throwed, you can not delete a throwed group."), QMessageBox::Ok);
			return;
		}
		
		if(!GroupManager::instance()->deleteGroup(group, error)) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", error, QMessageBox::Ok);
			return;
		}
		
		QTreeWidgetItemIterator child(tree);
		while(*child) {
			QTreeWidgetItem *childItem = (*child);
			++child;
			
			if(childItem->parent() == tmpItem) {
				CClient *client = ClientManager::instance()->findClient(childItem->data(5, 0).toByteArray());
				tmpItem->removeChild(childItem);
				ClientManager::instance()->getClientItem(client)->setHidden(false);

				if(client->getStatus(CLIENT_STATUS_DISCONNECTED)) 
					if(!client->getStatus(CLIENT_STATUS_CONNECTED))
						ClientManager::instance()->getClientItem(client)->setTextColor(QColor::fromRgb(255, 48, 48));
					else
						client->setStatus(CLIENT_STATUS_CONNECTED, true);
				
				ClientManager::instance()->removeItemFromMap(true, client);
			}
		}
		
		tree->removeItemWidget(tmpItem, 0);
		delete tmpItem;
	}
}

void BackharddiNet_Monitor::throwGroup()
{
	QString error;
	int disconnected, connected = 0;
	disconnected = connected;
	
	if((monitorTab->widget(0) != NULL) && (monitorTab->widget(0) == monitorTab->currentWidget())) {
		GeneralTab *general = static_cast<GeneralTab*>(monitorTab->currentWidget());
		TasksTree *tree = general->getTree();
		QTreeWidgetItemIterator it(tree, QTreeWidgetItemIterator::Selected);
		
		if(!*it) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To throw a group from the current task you will select it first."), QMessageBox::Ok);
			return;
		}
		
		QTreeWidgetItem *tmpItem = (*it);
		if(tmpItem->data(4,0).toString() != "group") {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To throw a group from the current task you will select it first."), QMessageBox::Ok);
			return;
		}
		
		CGroup *group = GroupManager::instance()->findGroup(tmpItem->text(1));
		
		if(!group->isConfigured()) {
			QMessageBox::StandardButton ret = QMessageBox::critical(this, "Backharddi-Net Monitor", tr("This group is not configured yet.\nDo you want to configure it now?."), QMessageBox::Yes | QMessageBox::No);
			
			if(ret == QMessageBox::Yes) {
				editGroupDialog();
			}
			
			return;
		}
		
		QTreeWidgetItemIterator child(tree);
		while(*child) {
			QTreeWidgetItem *childItem = (*child);
			++child;
			
			if(childItem->parent() == tmpItem) {
				CClient *client = ClientManager::instance()->findClient(childItem->data(5, 0).toByteArray());
				if(client->getStatus(CLIENT_STATUS_DISCONNECTED))
					disconnected++;
				else
					connected++;
			}
		}
		
		if(connected == 0) {
			QMessageBox::warning(this, "Backharddi-Net Monitor",
			     tr("You can not throw a multicast group without connected clients. Boot PXE Backharddi clients and try again."),
				QMessageBox::Ok);
			return;
		}
		
		if(disconnected != 0) {
			QMessageBox::StandardButton ret = QMessageBox::warning(this, "Backharddi-Net Monitor",
					tr("There are clients disconnected on this group.\nIf you throw it before they get connected, those missing clients will not be masterized.\n\nAre you sure do you want continue with the group throwing?"),
					   QMessageBox::Yes | QMessageBox::No);
				
			if(ret == QMessageBox::No)
				return;
		}
		
		if(!GroupManager::instance()->throwGroup(group, error)) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", error, QMessageBox::Ok);
			return;
		}
		
		TaskManager::instance()->currentTask()->setState(STATUS_RUNNING, true);
	}
}

void BackharddiNet_Monitor::throwAll()
{
	QString error;
	bool success = false;
	
	TASK_GROUP_MAP::iterator it;	
	
	for(it = TaskManager::instance()->currentTask()->retrieveTaskGroups()->begin(); it != TaskManager::instance()->currentTask()->retrieveTaskGroups()->end(); ++it) {	
	
		CGroup *group = *it;
		if(group->isConfigured()) {
			int disconnected = 0;
			int connected = 0;
			foreach(CClient *client, group->getAllClientsFromMap()) {
				if(client->getStatus(CLIENT_STATUS_DISCONNECTED))
					disconnected++;
				else
					connected++;
			}
			
			if(connected == 0)
				continue;
		}
		
		
		if(!GroupManager::instance()->throwGroup(group, error)) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", error, QMessageBox::Ok);
			continue;
		}
		
		if(!success)
			success = true;
	}
	
	if(success)
		TaskManager::instance()->currentTask()->setState(STATUS_RUNNING, true);
}

void BackharddiNet_Monitor::rebootGroup()
{
	QString error;
	
	if((monitorTab->widget(0) != NULL) && (monitorTab->widget(0) == monitorTab->currentWidget())) {
		GeneralTab *general = static_cast<GeneralTab*>(monitorTab->currentWidget());
		TasksTree *tree = general->getTree();
		QTreeWidgetItemIterator it(tree, QTreeWidgetItemIterator::Selected);
		
		if(!*it) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To reboot clients from the current task's group you will select it first."), QMessageBox::Ok);
			return;
		}
		
		QTreeWidgetItem *tmpItem = (*it);
		if(tmpItem->data(4,0).toString() == "client") {
			QMessageBox::StandardButton ret = QMessageBox::critical(this, "Backharddi-Net Monitor", tr("Are you sure do you want to reboot this client?."), QMessageBox::Yes | QMessageBox::No);			
			
			if(ret == QMessageBox::Yes) {
				if(!ClientManager::instance()->findClient(tmpItem->data(6,0).toString())->getStatus(CLIENT_STATUS_CONNECTED)) {
					QMessageBox::critical(this, "Backharddi-Net Monitor", tr("You can't reboot a disconnected client."), QMessageBox::Ok);
					return;
				}
				
				if(ClientManager::instance()->findClient(tmpItem->data(6,0).toString())->getStatus(CLIENT_STATUS_UPLOADING) || ClientManager::instance()->findClient(tmpItem->data(6,0).toString())->getStatus(CLIENT_STATUS_UPLOADING)) {
					QMessageBox::critical(this, "Backharddi-Net Monitor", tr("You can not reboot a client that is performing any operation."), QMessageBox::Ok);
					return;
				}
				
				if(ClientManager::instance()->findClient(tmpItem->data(6,0).toString())->getStatus(CLIENT_STATUS_SHELL_ATTACHED)) {
					closeTab(ClientManager::instance()->findClient(tmpItem->data(6,0).toString())->Shell());
				}
				rebootClient(ClientManager::instance()->findClient(tmpItem->data(6,0).toString()));
				
				// Update Client Status
				ClientManager::instance()->findClient(tmpItem->data(6,0).toString())->setStatus(CLIENT_STATUS_CONNECTED, false);
				ClientManager::instance()->findClient(tmpItem->data(6,0).toString())->setStatus(CLIENT_STATUS_DISCONNECTED, true);
				TaskManager::instance()->redrawTask();
			}
			
			return;
		}
		else if(tmpItem->data(4,0).toString() != "group") {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To reboot clients from the current task's group you will select it first."), QMessageBox::Ok);
			return;
		}
		
		CGroup *group = GroupManager::instance()->findGroup(tmpItem->text(1));
		
		QMessageBox::StandardButton ret = QMessageBox::critical(this, "Backharddi-Net Monitor", tr("Are you sure do you want to reboot the clients on this group?."), QMessageBox::Yes | QMessageBox::No);
		
		if(ret == QMessageBox::Yes)
			GroupManager::instance()->rebootGroup(group);
	}
}

void BackharddiNet_Monitor::closeTab(QTermWidget *shw)
{
	ShellWidget *tmpSh = static_cast<ShellWidget *>(shw);
	removeRemoteShellFromClient(ClientManager::instance()->findClient(tmpSh->getClient()));	
}

void BackharddiNet_Monitor::rebootClient(CClient *client)
{
	ClientManager::instance()->rebootClient(client);
}

void BackharddiNet_Monitor::loadLastTask()
{
	// Load the last task
	if(d->m_initialTaskId != 0) {		
		QString error = "";
		CTask *lastTask = TaskManager::instance()->getLastTask(error);
		if(!error.isEmpty()) {
			qDebug() << "Error: BackharddiNet::Monitor::loadTask(): " << error;
		}
		if(lastTask != NULL) {
			d->m_taskManager->insertTask(lastTask);
			d->m_taskManager->setCurrentTask(lastTask);
		}
	}
}

void BackharddiNet_Monitor::removeClient()
{
	QString error;
	
	if((monitorTab->widget(0) != NULL) && (monitorTab->widget(0) == monitorTab->currentWidget())) {
		GeneralTab *general = static_cast<GeneralTab*>(monitorTab->currentWidget());
		TasksTree *tree = general->getTree();
		QTreeWidgetItemIterator it(tree, QTreeWidgetItemIterator::Selected);
		
		if(!*it) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To remove a client from any group you will select it first."), QMessageBox::Ok);
			return;
		}
		
		QTreeWidgetItem *tmpItem = (*it);
		if(tmpItem->data(4,0).toString() != "client") {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("To remove a client from any group you will select it first."), QMessageBox::Ok);
			return;
		}
		
		CClient *tmpClient = ClientManager::instance()->findClient(tmpItem->data(6,0).toString());
		
		if(!tmpClient)
			return;
		
		if(tmpClient->getStatus(CLIENT_STATUS_MASTERING) || tmpClient->getStatus(CLIENT_STATUS_UPLOADING)) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("You can not remove this client because it's working yet."), QMessageBox::Ok);
			return;
		}
		
		QTreeWidgetItem *parentItem = tmpItem->parent();
		
		if(!parentItem)
			return;
		
		// Remove the item from the tasks widget 
		parentItem->removeChild(tmpItem);
		
		// Make the item visible on items list
		QListWidgetItem *tmpListItem = ClientManager::instance()->getClientItem(tmpClient);
		if(tmpListItem != NULL && tmpListItem->isHidden())
			tmpListItem->setHidden(false);
		
		if(tmpClient->getStatus(CLIENT_STATUS_DISCONNECTED))
			if(!tmpClient->getStatus(CLIENT_STATUS_CONNECTED))
				ClientManager::instance()->getClientItem(tmpClient)->setTextColor(QColor::fromRgb(255, 48, 48));
		else
			tmpClient->setStatus(CLIENT_STATUS_CONNECTED, true);
		
		ClientManager::instance()->removeItemFromMap(true, tmpClient);
		
		QTreeWidgetItemIterator child(tree);
		while(*child) {
			QTreeWidgetItem *childItem = (*child);
			++child;
			
			if(childItem->parent() == tmpItem) {
				CClient *client = ClientManager::instance()->findClient(childItem->data(5, 0).toByteArray());
				tmpItem->removeChild(childItem);
				ClientManager::instance()->getClientItem(client)->setHidden(false);

				if(client->getStatus(CLIENT_STATUS_DISCONNECTED)) 
					if(!client->getStatus(CLIENT_STATUS_CONNECTED))
						ClientManager::instance()->getClientItem(client)->setTextColor(QColor::fromRgb(255, 48, 48));
				else
					client->setStatus(CLIENT_STATUS_CONNECTED, true);
				
				ClientManager::instance()->removeItemFromMap(true, client);
			}
		}
		
		// TODO: Eliminar el cliente del grupo de forma real!!!!!!!
		CGroup *tmpGroup = tmpClient->getGroup();
		
		if((tmpGroup) && tmpGroup->removeClientFromMap(tmpClient->getName()) != ERR_NO_ERROR) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("Can not remove client from group because errors."), QMessageBox::Ok);
			return;
		}
		
		delete tmpItem;
	}
}


