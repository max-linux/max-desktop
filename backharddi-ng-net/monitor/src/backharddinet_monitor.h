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


#ifndef BACKHARDDINET_MONITOR_H
#define BACKHARDDINET_MONITOR_H

#include "ui_MainForm.h"
#include <QSystemTrayIcon>
#include <QListWidget>
#include <QHash>

class CTask;
class CGroup;
class CClient;
class ClientsList;
class QByteArray;
class QTermWidget;
class BackharddiNet_MonitorPriv;

class BackharddiNet_Monitor : public QMainWindow, protected Ui::mainWindow {
	Q_OBJECT
public:
	BackharddiNet_Monitor();
	
     // System Tray
     void setVisible(bool visible);     
	void showMessage(const QString title, const QString msg);	
	ClientsList *takeClientsList();
	
	// Remote Shells
	void addRemoteShellToClient(CClient *client);
	bool removeRemoteShellFromClient(CClient *client);

public slots:
	void closeTab(QTermWidget *);
	void rebootClient(CClient *client);
	void loadLastTask();
	
private slots:
	void About();
	void DocumentModified();
	void open();
     void openSQL();  
     void prefsSqlDialog();
     void prefsTaskDialog();
	void newGroupDialog(CGroup *group);
	void editGroupDialog();
	void deleteGroup();	
	void throwGroup();
	void throwAll();
	void rebootGroup();
	void removeClient();
	void closeTask();

     // TaskManager
     void redrawTasksWidgetTab(CTask *task);	
     
     // System Tray     
     void iconActivated(QSystemTrayIcon::ActivationReason reason);
     void messageClicked();
	 
	// Docks
	void addClientToDockBar(CClient *client);
	void removeClientFromDockBar(CClient *client);	
	
	void saveToDatabase(CClient *client);	

protected:
     void closeEvent(QCloseEvent *event);
     
signals:
     void maximixed();    

private:
	void CreateConnections();
	void CreateShortCuts();
	QWidget* loadUiFile();
	void  loadFile(const QString &fileName);
	bool  IsModified();
	bool  maybeSave();
	bool  save();
	bool  saveAs();
	bool  saveFile(const QString &fileName);
     
     void createTrayIcon();    
     void createActions(); 
     void setIcon();
	
	bool	m_Modified;

     QSystemTrayIcon *trayIcon;
     QMenu *trayIconMenu;

     QAction *minimizeAction;
     QAction *maximizeAction;
     QAction *restoreAction;
     QAction *quitAction;	
	ClientsList *clientsListWidget;

     BackharddiNet_MonitorPriv     *d;
};

#endif
