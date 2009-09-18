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
#include "taskstree.h"
#include "variantdelegate.h"
#include "TaskManager.h"
#include "groupmanager.h"
#include "clientmanager.h"
#include "Task.h"
#include "cgroup.h"
#include "cclient.h"
#include "Loader.h"
#include "backharddinet_monitor.h"
#include "shellwidget.h"
#include "osdwidget.h"

#include <QtCore>
#include <QtGui>
#include <QDate>
#include <QMimeData>
#include <QStringList>

extern Loader *settings;


TasksTree::TasksTree(QWidget *parent) : QTreeWidget(parent)
{
	extern BackharddiNet_Monitor *PBMonitor;
	setItemDelegate(new VariantDelegate(this));
	
	QStringList labels;
	labels << tr("Creation Date") << tr("Name or Description") << tr("Summary");
	setHeaderLabels(labels);
	header()->setResizeMode(0, QHeaderView::Stretch);
	header()->setResizeMode(1, QHeaderView::Stretch);
	
	task = 0;
	refreshTimer.setInterval(2000);
	autoRefresh = false;
	
	groupIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon), QIcon::Normal, QIcon::Off);
	groupIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon), QIcon::Normal, QIcon::On);
	
	clientIcon.addPixmap(style()->standardPixmap(QStyle::SP_ComputerIcon));	
	
	setColumnCount(3);
	
	setMouseTracking(true);	
	
	createActions();
	createContextMenu();
	
	connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(maybeRefresh()));	
	connect(TaskManager::instance(), SIGNAL(signalTaskRedraw()), this, SLOT(redrawTask()));
	connect(removeGroup, SIGNAL(triggered()), PBMonitor, SLOT(deleteGroup()));
	connect(throwGroup, SIGNAL(triggered()), PBMonitor, SLOT(throwGroup()));
	connect(reboot, SIGNAL(triggered()), PBMonitor, SLOT(rebootGroup()));
	connect(openShell, SIGNAL(triggered()), this, SLOT(startShell()));
	connect(throwAll, SIGNAL(triggered()), PBMonitor, SLOT(throwAll()));
	connect(removeClient, SIGNAL(triggered()), PBMonitor, SLOT(removeClient()));
}

void TasksTree::createActions()
{
	removeGroup = new QAction(tr("Remove Group"), this);
	removeGroup->setObjectName(QString::fromUtf8("actionRemoveGroup"));
	removeGroup->setIcon(QIcon(QString::fromUtf8(":/icons/editdelete.png")));
	
	removeClient = new QAction(tr("Remove Client"), this);
	removeClient->setObjectName(QString::fromUtf8("actionRemoveClient"));
	removeClient->setIcon(QIcon(QString::fromUtf8(":/icons/editdelete.png")));
	
	throwGroup = new QAction(tr("Throw Group"), this);
	throwGroup->setObjectName(QString::fromUtf8("actionThrowGroup"));
	throwGroup->setIcon(QIcon(QString::fromUtf8(":/icons/build.png")));
	
	throwAll = new QAction(tr("Throw All"), this);
	throwAll->setObjectName(QString::fromUtf8("actionThrowGroup"));
	throwAll->setIcon(QIcon(QString::fromUtf8(":/icons/down.png")));
	
	reboot = new QAction(tr("Reboot Group"), this);
	reboot->setObjectName(QString::fromUtf8("actionReboot"));
	reboot->setIcon(QIcon(QString::fromUtf8(":/icons/reload.png")));
	
	openShell = new QAction(tr("Open Shell"), this);
	openShell->setObjectName(QString::fromUtf8("actionOpenShell"));
	openShell->setIcon(QIcon(QString::fromUtf8(":/icons/konsole.png")));
}

void TasksTree::createContextMenu()
{
	contextMenu = new QMenu(this);
	contextMenu->addAction(removeGroup);
	contextMenu->addAction(removeClient);
	contextMenu->addAction(openShell);
	contextMenu->addAction(throwGroup);
	contextMenu->addAction(throwAll);
	contextMenu->addAction(reboot);
}

void TasksTree::contextMenuEvent(QContextMenuEvent *event)
{
	if(!event)
		return;
	
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);
	if(!*it) {
		removeGroup->setEnabled(false);
		removeClient->setEnabled(false);
		openShell->setEnabled(false);
		throwGroup->setEnabled(false);
		throwAll->setEnabled(false);
		reboot->setEnabled(false);
	}
	else {
		if((*it)->data(4,0).toString() == "group") {
			removeClient->setEnabled(false);
			openShell->setEnabled(false);
			removeGroup->setEnabled(true);
			
			if(GroupManager::instance()->findGroup((*it)->text(1))->isConfigured()) 
				throwGroup->setEnabled(true);
			else
				throwGroup->setEnabled(false);
			
			reboot->setText(tr("Reboot Group"));
		}
		else {
			removeClient->setEnabled(true);	
			removeGroup->setEnabled(false);
			throwGroup->setEnabled(false);
			reboot->setText(tr("Reboot this client"));
			if(ClientManager::instance()->findClient(currentItem()->data(6,0).toString())->getStatus(CLIENT_STATUS_CONNECTED)) {
				openShell->setEnabled(true);
				if(ClientManager::instance()->findClient(currentItem()->data(6,0).toString())->getStatus(CLIENT_STATUS_SHELL_ATTACHED))
					openShell->setText(tr("Close remote Shell"));
			}
			else {
				openShell->setEnabled(false);
				if(ClientManager::instance()->findClient(currentItem()->data(6,0).toString())->getStatus(CLIENT_STATUS_SHELL_ATTACHED))
					openShell->setText(tr("Open Shell"));
			}
		}
		
		throwAll->setEnabled(true);
		reboot->setEnabled(true);
	}
	contextMenu->exec(event->globalPos());
}

void TasksTree::setTaskObject(CTask *pTask)
{
	task = pTask;
	clear();
	
	if(task) {
		refresh();
		if(autoRefresh)
			refreshTimer.start();
	}
	else {
		refreshTimer.stop();
	}
}

QSize TasksTree::sizeHint() const
{
	return QSize(800, 600);
}

void TasksTree::setAutoRefresh(bool autoRefresh)
{
	this->autoRefresh = autoRefresh;
	if(task) {
		if(autoRefresh) {
			maybeRefresh();
			refreshTimer.start();
		}
		else {
			refreshTimer.stop();
		}
	}
}

void TasksTree::maybeRefresh()
{
	if(state() != EditingState)
		refresh();
}

void TasksTree::refresh()
{
	if(!task)
		return;
	
	disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(updateGroup(QTreeWidgetItem *)));
		
	updateChildItems(0);
	
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int )), this, SLOT(updateGroup(QTreeWidgetItem*)));
}

bool TasksTree::event(QEvent *event)
{
	if(event->type() == QEvent::WindowActivate) {
		if(isActiveWindow() && autoRefresh)
			maybeRefresh();
	}
	
	return QTreeWidget::event(event);
}

void TasksTree::updateGroup(QTreeWidgetItem *item)
{	
	QString key = item->text(0);
	QTreeWidgetItem *ancestor = item->parent();
	while(ancestor) {
		key.prepend(ancestor->text(0) + "/");
		ancestor = ancestor->parent();
	}
	
	if(autoRefresh)
		refresh();
}

QStringList TasksTree::mimeTypes() const
{
	QStringList types;
	types << "application/vnd.text.list";
	return types;
}

QMimeData *TasksTree::mimeData(const QList<QTreeWidgetItem*> items) const
{
	Q_UNUSED(items);
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;	

	mimeData->setData("application/vnd.text.list", encodedData);
	return mimeData;
}

bool TasksTree::dropMimeData (QTreeWidgetItem * parent, int index, const QMimeData * data, Qt::DropAction action)
{
	Q_UNUSED(index);
	if (action == Qt::IgnoreAction)
		return true;	
	
	if (!data->hasFormat("application/vnd.text.list") && !data->hasFormat("application/vnd.client.item") && !data->hasFormat("application/vnd.group.item")) {		
		qDebug() << "No data spcified";
		return false;	
	}
	
	if(data->hasFormat("application/vnd.client.item")) {
		
		QString errMsg;
		// If there are no groups configured yet on this task
		if(TaskManager::instance()->currentTask()->retrieveTaskGroups()->isEmpty()) {
			CGroup *newGroup = GroupManager::instance()->createGroup(tr("<New Group>"), QDate::currentDate(), TaskManager::instance()->currentTask()->getId(), errMsg);
				
			if(!newGroup)
				return false;
				
			for(int i = 0; i < ClientManager::instance()->getCurrent().count(); ++i) {
				// Get the client object
				CClient *client = ClientManager::instance()->findClient(ClientManager::instance()->getCurrent().at(i)->data(1).toString());
				
				// Add the client to the new group
				newGroup->addCLientToMap(ClientManager::instance()->getCurrent().at(i)->data(1).toString(), client);
				
				// Hide the left client gui element
				ClientManager::instance()->getClientItem(ClientManager::instance()->findClient(ClientManager::instance()->getCurrent().at(i)->data(1).toString()))->setHidden(true);
			}
				
			GroupManager::instance()->emitConfigure(newGroup);
		}
		else {
			// If there is a parent item
			if(parent) {
				// If parent item is just a group
				if(parent->data(4,0).toString() == "group") {
					// Get the group object
					CGroup *group = GroupManager::instance()->findGroup(parent->text(1));
					
					// Check the pointer
					if(!group)
						return false;
					
					// Check if group is configured and group os configured as generation one
					if((group->isConfigured()) && (group->getSettings().split(",").at(0).toInt() == 0)) {
						QMessageBox::warning(this, "Backharddi-Net Monitor", tr("You can not add a new client to a multicast group configured for generation."), QMessageBox::Ok);
						
						return false;
					}
					
					// Iterate over droped items
					for(int i = 0; i < ClientManager::instance()->getCurrent().count(); ++i) {
						// Get the client object 
						CClient *client = ClientManager::instance()->findClient(ClientManager::instance()->getCurrent().at(i)->data(1).toString());
								
						// Add client to group clients map
						group->addCLientToMap(ClientManager::instance()->getCurrent().at(i)->data(1).toString(), client);
						
						// Hide the left client gui element
						ClientManager::instance()->getClientItem(ClientManager::instance()->findClient(ClientManager::instance()->getCurrent().at(i)->data(1).toString()))->setHidden(true);
					}
					
					GroupManager::instance()->saveGroupClientsRelation(group);
					TaskManager::instance()->redrawTask();
				}
			}
			else {
				// IF the user does not droped the client item into an already configured group, just create a new one
				CGroup *newGroup = GroupManager::instance()->createGroup(tr("<New Group>"), QDate::currentDate(), TaskManager::instance()->currentTask()->getId(), errMsg);
				
				if(!newGroup)
					return false;
				
				for(int i = 0; i < ClientManager::instance()->getCurrent().count(); ++i) {
					// Get the client object
					CClient *client = ClientManager::instance()->findClient(ClientManager::instance()->getCurrent().at(i)->data(1).toString());
					
					// Add client to group clients map
					newGroup->addCLientToMap(ClientManager::instance()->getCurrent().at(i)->data(1).toString(), client);
					
					// Hide the left client gui element
					ClientManager::instance()->getClientItem(ClientManager::instance()->findClient(ClientManager::instance()->getCurrent().at(i)->data(1).toString()))->setHidden(true);
				}
				
				GroupManager::instance()->emitConfigure(newGroup);
			}
		}
		
		return true;
	}
	
	if(!parent)
		return false;	
	
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);
	
	if(!*it)
		return false;
	else {
		while(*it) {
			QTreeWidgetItem *tmpItem = (*it);
			++it;
			if(tmpItem->data(4,0).toString() == "group")
				continue;
		
			if((tmpItem->parent() == parent) || (tmpItem == parent))
				continue;
			
			CGroup *sourceGroup;
			CGroup *destinationGroup;
			
			if((parent) && parent->data(4,0).toString() == "client") {
				if(parent->parent()) {
					if(parent->parent() == tmpItem->parent())
						continue;
					
					sourceGroup = TaskManager::instance()->currentTask()->retrieveTaskGroups()->value(tmpItem->parent()->text(1));
					destinationGroup = TaskManager::instance()->currentTask()->retrieveTaskGroups()->value(parent->parent()->text(1));
					
					if(destinationGroup->getSettings().split(",").at(0).toInt() == 0) {
						QMessageBox::warning(this, "Backharddi-Net Monitor", tr("You can not add a new client to a multicast group configured for generation."), QMessageBox::Ok);
						return false;
					}
					
					CClient *tmpClient = ClientManager::instance()->findClient(tmpItem->data(6,0).toString());
					sourceGroup->removeClientFromMap(tmpItem->data(6,0).toString());
					destinationGroup->addCLientToMap(tmpItem->data(6,0).toString(), tmpClient); 
					
					GroupManager::instance()->saveGroupClientsRelation(sourceGroup);
					GroupManager::instance()->saveGroupClientsRelation(destinationGroup);
				}
				
				continue;
			}
			else {
				sourceGroup = TaskManager::instance()->currentTask()->retrieveTaskGroups()->value(tmpItem->parent()->text(1));
				destinationGroup = TaskManager::instance()->currentTask()->retrieveTaskGroups()->value(parent->text(1));
				
				if(destinationGroup->getSettings().split(",").at(0).toInt() == 0) {
					QMessageBox::warning(this, "Backharddi-Net Monitor", tr("You can not add a new client to a multicast group configured for generation."), QMessageBox::Ok);
					return false;
				}
				
				CClient *tmpClient = ClientManager::instance()->findClient(tmpItem->data(6,0).toString());
				sourceGroup->removeClientFromMap(tmpItem->data(6,0).toString());
				destinationGroup->addCLientToMap(tmpItem->data(6,0).toString(), tmpClient); 
					
				GroupManager::instance()->saveGroupClientsRelation(sourceGroup);
				GroupManager::instance()->saveGroupClientsRelation(destinationGroup);
				
			}
			
			TaskManager::instance()->redrawTask();
		}
	}
	
	return true;
}

void TasksTree::updateChildItems(QTreeWidgetItem *parent)
{	
	QTreeWidgetItem *groupItem;
	QTreeWidgetItem *item;
	QStringList key = task->retrieveTaskGroups()->keys();
	
	foreach (CGroup *group, task->retrieveTaskGroups()->values()) {
		if (parent) {
			groupItem = new QTreeWidgetItem(parent);
		} else {
			groupItem = new QTreeWidgetItem(this);
		}
		
		groupItem->setText(0, group->getDate());
		groupItem->setText(1, group->getName());
		groupItem->setText(2, tr("Inactive"));
		groupItem->setIcon(0, groupIcon);	
		groupItem->setData(4, 0, QVariant("group"));
		groupItem->setData(5, 0, QVariant(group->getId()));
		groupItem->setData(6, 0, QVariant(group->isConfigured()));
		
		qDebug() << group->getAllClientsFromMap();
		
		foreach (CClient *client, group->getAllClientsFromMap()) {	
			if (groupItem) {
				item = new QTreeWidgetItem(groupItem);
			} else {
				item = new QTreeWidgetItem(this);
			}

			item->setText(0, client->getInformation("date"));
			item->setText(1, client->getFakeName());
			item->setText(2, client->getStatusFlagCode().join(" "));
			item->setIcon(0, clientIcon);
			item->setData(4, 0, QVariant("client"));
			item->setData(5, 0, QVariant(client->getID()));
			item->setData(6, 0, QVariant(client->getMac()));
			
			ClientManager::instance()->addItemToMap(true, client, item);
			//client->setTaskGraphicEntity(item);	
			paintStatusColors(item, client);
		}
	}	
}

void TasksTree::redrawTask()
{
	QTreeWidgetItem *groupItem;
	QTreeWidgetItem *item;
	
	foreach (CGroup *group, task->retrieveTaskGroups()->values()) {
		if((groupItem = findItem(group->getId())) != NULL) {
			if(group->getStatus(CGroup::STATUS_COMPLETE))
				groupItem->setText(2, tr("Completed"));
			else
				groupItem->setText(2, group->getStatus(CGroup::STATUS_THROWED) ? tr("Throwed") : tr("Inactive"));
			
			foreach (CClient *client, group->getAllClientsFromMap()) {
				if((item = findItem(client->getID())) != NULL) {
					item->setText(2, client->getStatusFlagCode().join(" "));
					item->setToolTip(0, client->toolTip());
					item->setToolTip(1, client->toolTip());
					item->setToolTip(2, client->toolTip());
					paintStatusColors(item, client);
				}
				else {
					item = new QTreeWidgetItem(groupItem);

					item->setText(0, client->getInformation("date"));
					item->setText(1, client->getFakeName());
					item->setText(2, client->getStatusFlagCode().join(" "));
					item->setIcon(0, clientIcon);
					item->setData(4, 0, QVariant("client"));
					item->setData(5, 0, QVariant(client->getID()));
					item->setData(6, 0, QVariant(client->getMac()));
			
					item->setToolTip(0, client->toolTip());
					item->setToolTip(1, client->toolTip());
					item->setToolTip(2, client->toolTip());
			
					ClientManager::instance()->addItemToMap(true, client, item);
					paintStatusColors(item, client);
				}
			}
		}
		else {
			groupItem = new QTreeWidgetItem(this);
			
			groupItem->setText(0, group->getDate());
			groupItem->setText(1, group->getName());
			groupItem->setIcon(0, groupIcon);	
			groupItem->setData(4, 0, QVariant("group"));
			groupItem->setData(5, 0, QVariant(group->getId()));
			groupItem->setData(6, 0, QVariant(group->isConfigured()));
		
			if(group->getStatus(CGroup::STATUS_COMPLETE)) 
				groupItem->setText(2, tr("Completed"));
			else
				groupItem->setText(2, group->getStatus(CGroup::STATUS_THROWED) ? tr("Throwed") : tr("Inactive"));
		
		
			foreach (CClient *client, group->getAllClientsFromMap()) {
				item = new QTreeWidgetItem(groupItem);

				item->setText(0, client->getInformation("date"));
				item->setText(1, client->getFakeName());
				item->setText(2, client->getStatusFlagCode().join(" "));
				item->setIcon(0, clientIcon);
				item->setData(4, 0, QVariant("client"));
				item->setData(5, 0, QVariant(client->getID()));
				item->setData(6, 0, QVariant(client->getMac()));
			
				item->setToolTip(0, client->toolTip());
				item->setToolTip(1, client->toolTip());
				item->setToolTip(2, client->toolTip());
			
				ClientManager::instance()->addItemToMap(true, client, item);
				paintStatusColors(item, client);
			
			}
		
			groupItem->setExpanded(true);
		}
	}
}

/*void TasksTree::redrawTask()
{	
	QTreeWidgetItemIterator it(this);
	if(*it) {
		while(*it) {
			delete *it;
		}
	}		
	
	QTreeWidgetItem *groupItem;
	QTreeWidgetItem *item;
	QStringList key = task->retrieveTaskGroups()->keys();
	
	foreach (CGroup *group, task->retrieveTaskGroups()->values()) {
		groupItem = new QTreeWidgetItem(this);
		
		groupItem->setText(0, group->getDate());
		groupItem->setText(1, group->getName());
		groupItem->setIcon(0, groupIcon);	
		groupItem->setData(4, 0, QVariant("group"));
		groupItem->setData(5, 0, QVariant(group->getId()));
		groupItem->setData(6, 0, QVariant(group->isConfigured()));
		
		if(group->getStatus(CGroup::STATUS_COMPLETE)) 
			groupItem->setText(2, tr("Completed"));
		else
			groupItem->setText(2, group->getStatus(CGroup::STATUS_THROWED) ? tr("Throwed") : tr("Inactive"));
		
		
		foreach (CClient *client, group->getAllClientsFromMap()) {
			item = new QTreeWidgetItem(groupItem);

			item->setText(0, client->getInformation("date"));
			item->setText(1, client->getFakeName());
			item->setText(2, client->getStatusFlagCode().join(" "));
			item->setIcon(0, clientIcon);
			item->setData(4, 0, QVariant("client"));
			item->setData(5, 0, QVariant(client->getID()));
			item->setData(6, 0, QVariant(client->getMac()));
			
			item->setToolTip(0, client->toolTip());
			item->setToolTip(1, client->toolTip());
			item->setToolTip(2, client->toolTip());
			
			ClientManager::instance()->addItemToMap(true, client, item);
			//client->setTaskGraphicEntity(item);
			paintStatusColors(item, client);
			
		}
		
		groupItem->setExpanded(true);
	}	
}*/

void TasksTree::paintStatusColors(QTreeWidgetItem *item, CClient *client)
{
	if(client->getStatus(CLIENT_STATUS_DISCONNECTED))
		item->setTextColor(2, QColor::fromRgb(255, 48, 48));	// Red
	if(client->getStatus(CLIENT_STATUS_CONNECTED))
		item->setTextColor(2, QColor::fromRgb(50, 205, 50));	// Green
	if(client->getStatus(CLIENT_STATUS_UPLOADING))
		item->setTextColor(2, QColor::fromRgb(255, 127, 0));	// Orange
	if(client->getStatus(CLIENT_STATUS_MASTERING))
		item->setTextColor(2, QColor::fromRgb(0, 197, 205));	// Turquoise
	if(client->getStatus(CLIENT_STATUS_JUST_DONE))
		item->setTextColor(2, QColor::fromRgb(50, 205, 50));	// Green
	if(client->getStatus(CLIENT_STATUS_JUST_ERROR))
		item->setTextColor(2, QColor::fromRgb(255, 48, 48));	// Red
}

QTreeWidgetItem *TasksTree::createItem(const QString &text, QTreeWidgetItem *parent, int index)
{
	QTreeWidgetItem *after = 0;
	if(index != 0)
		after = childAt(parent, index - 1);
	
	QTreeWidgetItem *item;
	if(parent)
		item = new QTreeWidgetItem(parent, after);
	else
		item = new QTreeWidgetItem(this, after);

	item->setText(0, text);
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	return item;
}

QTreeWidgetItem *TasksTree::childAt(QTreeWidgetItem *parent, int index)
{
	if (parent)
		return parent->child(index);
	else
		return topLevelItem(index);
}

int TasksTree::childCount(QTreeWidgetItem *parent)
{
	if (parent)
		return parent->childCount();
	else
		return topLevelItemCount();
}

int TasksTree::findChild(QTreeWidgetItem *parent, const QString &text, int startIndex)
{
	for (int i = startIndex; i < childCount(parent); ++i) {
		if (childAt(parent, i)->text(0) == text)
			return i;
	}
	return -1;
}

void TasksTree::moveItemForward(QTreeWidgetItem *parent, int oldIndex, int newIndex)
{
	for (int i = 0; i < oldIndex - newIndex; ++i)
		delete childAt(parent, newIndex);
}

void TasksTree::mouseDoubleClickEvent(QMouseEvent *event)
{
	if(!event) {
#ifdef __MONITOR_DEBUG
		qDebug() << "TaskTree Debug: mouseDoubleClickEvent triggered without a valid event object";
#endif
		return;
	}
		
	if(event->button() == Qt::LeftButton) {
		if( !currentItem() )
			return;
		
		if(currentItem()->data(4,0).toString() == "client") {
			CClient *tmpClient = ClientManager::instance()->findClient( currentItem()->data(6,0).toString() );
			OSDWidget *osd = ClientManager::instance()->getClientOsd( tmpClient );
			
			if( osd == NULL ) {
				event->ignore();
				return;
			}
			
			QPoint pos = event->globalPos();
			
			osd->setlocation( pos );
			
			if( tmpClient->getStatus( CLIENT_STATUS_DISCONNECTED ) )
				osd->setText( tmpClient->getFakeName() + " " + tr( "This client is disconnected" ) );	
			
			if( osd->isVisible() )
				osd->hide();
			else
				osd->show();
		}
	}
	
	event->accept();
}

void TasksTree::dragEnterEvent(QDragEnterEvent *event)
{	
	if(event->mimeData()->hasFormat("application/vnd.client.item") || event->mimeData()->hasFormat("application/vnd.text.list"))
		event->acceptProposedAction();
	else
		event->ignore();
}

void TasksTree::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

Qt::DropActions TasksTree::supportedDropActions() const
{
	// returns what actions are supported when dropping
	return Qt::CopyAction | Qt::MoveAction;
}

void TasksTree::startShell()
{
	extern BackharddiNet_Monitor *PBMonitor;
	if(currentItem()->data(4,0).toString() == "client") {
		if(!currentItem() || !ClientManager::instance()->findClient(currentItem()->data(6,0).toString())->getStatus(CLIENT_STATUS_CONNECTED))
			return;
			
		if(!ClientManager::instance()->findClient(currentItem()->data(6,0).toString())->getStatus(CLIENT_STATUS_SHELL_ATTACHED)) {
			PBMonitor->addRemoteShellToClient(ClientManager::instance()->findClient(currentItem()->data(6,0).toString()));
			openShell->setText(tr("Close remote shell"));
		}
		else {
			PBMonitor->closeTab(ClientManager::instance()->findClient(currentItem()->data(6,0).toString())->Shell());
			openShell->setText(tr("Open Shell"));
		}
	}
}

QTreeWidgetItem *TasksTree::findItem(QByteArray itemId)
{
	QTreeWidgetItemIterator it(this);
	if(*it) {
		while(*it) {
			if((*it)->data(4,0).toString() == "client") {
				if((*it)->data(5,0).toByteArray() == itemId) {
					return *it;
				}
			}
			
			it++;
		}
	}
	
	return NULL;
}

QTreeWidgetItem *TasksTree::findItem(unsigned int itemId)
{
	QTreeWidgetItemIterator it(this);
	if(*it) {
		while(*it) {
			if((*it)->data(4,0).toString() == "group") {
				if((*it)->data(5,0).toInt() == itemId) {
					return *it;
				}
			}
			
			it++;
		}
	}
	
	return NULL;
}

