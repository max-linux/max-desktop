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
#include "clientstree.h"
#include "clientmanager.h"
#include "cclient.h"
#include "cgroup.h"
#include "Loader.h"
#include "backharddinet_monitor.h"

#include <QtCore>
#include <QtGui>
#include <QMimeData>
#include <QListWidget>
#include <QListWidgetItem>

extern Loader *settings;
extern BackharddiNet_Monitor *PBMonitor;

ClientsListWidget::ClientsListWidget(QWidget *parent) : QListWidget(parent)
{	
	clientIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
	setMouseTracking(true);	
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	
	createActions();
	createContextMenu();	

	connect(ClientManager::instance(), SIGNAL(signalClientAdded(CClient*)), this, SLOT(createItem(CClient*)));
	connect(this, SIGNAL(signalDrawClient(CClient*)), ClientManager::instance(), SLOT(drawClient(CClient*)));
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
	connect(rebootAction, SIGNAL(triggered()), this, SLOT(rebootClient()));
	connect(renameAction, SIGNAL(triggered()), this, SLOT(renameClient()));
}

QSize ClientsListWidget::sizeHint() const
{
	return QSize(200, 272);
}

bool ClientsListWidget::event(QEvent *event)
{
	// Added for future features extensions
	return QListWidget::event(event);
}

QStringList  ClientsListWidget::mimeTypes() const
{
	QStringList types;
	types << "application/vnd.client.item";
	return types;
}

QMimeData *ClientsListWidget::mimeData(const QList<QListWidgetItem*> items) const
{
	Q_UNUSED(items);
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;
	
	mimeData->setData("application/vnd.client.item", encodedData);
	return mimeData;
}

void ClientsListWidget::dropEvent(QDropEvent *event)
{
	/*if(event->mimeData()->hasFormat("application/vnd.client.list")) {
		QByteArray itemData = event->mimeData()->data("application/vnd.client.list");
		QDataStream dataStream(&itemData, QIODevice::ReadOnly);
		
		QString name;
		QString tooltip;
		
		dataStream >> name >> tooltip;
		
		if(event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {*/
		event->ignore();
	//}
}

void ClientsListWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	extern BackharddiNet_Monitor *PBMonitor;
	if(!currentItem())
		return;
	
	if(event->button() == Qt::LeftButton) {
		if(!ClientManager::instance()->findClient(currentItem()->data(1).toString())->getStatus(CLIENT_STATUS_CONNECTED)) {
			event->ignore();
			return;
		}
			
		if(!ClientManager::instance()->findClient(currentItem()->text())->getStatus(CLIENT_STATUS_SHELL_ATTACHED)) {
			PBMonitor->addRemoteShellToClient(ClientManager::instance()->findClient(currentItem()->data(1).toString()));
		}
	}
	
	event->accept();
}

void ClientsListWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("application/vnd.text.list")) {
		if (event->source() == this) {
			event->ignore();
		} else {
			event->acceptProposedAction();
		}
	} else {
		event->ignore();
	}
}

void ClientsListWidget::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->mimeData()->hasFormat("application/vnd.text.list")) {
		if (event->source() == this) {
			event->ignore();
		} else {
			event->acceptProposedAction();
		}
	} else {
		event->ignore();
	}
}

void ClientsListWidget::selectionChanged()
{
	ClientManager::instance()->setCurrent(selectedItems());
}

void ClientsListWidget::createItem(CClient *client)
{	
	QListWidgetItem *tmpItem = new QListWidgetItem(this);
	tmpItem->setTextAlignment(Qt::AlignHCenter);
	tmpItem->setTextColor(QColor::fromRgb(50, 205, 50));
	tmpItem->setText(client->getFakeName());
	tmpItem->setData(1, QVariant(client->getName()));
	tmpItem->setFlags(tmpItem->flags() | Qt::ItemIsEditable);
	tmpItem->setToolTip(QString(""));	
	
	ClientManager::instance()->addItemToMap(false, client, 0, 
	tmpItem);
	
	emit(signalDrawClient(client));
}

void ClientsListWidget::createActions()
{
	renameAction = new QAction(tr("Rename client"), this);
	renameAction->setObjectName(QString::fromUtf8("actionRenameClient"));
	
	rebootAction = new QAction(tr("Reboot client"), this);
	rebootAction->setObjectName(QString::fromUtf8("actionRebootClient"));
	rebootAction->setIcon(QIcon(QString::fromUtf8(":/icons/reload.png")));
}

void ClientsListWidget::createContextMenu()
{
	contextMenu = new QMenu(this);
	
	contextMenu->addAction(renameAction);
	contextMenu->addAction(rebootAction);
}

void ClientsListWidget::contextMenuEvent(QContextMenuEvent *event)
{
	if(!currentItem())
		return;
	
	CClient *client = ClientManager::instance()->findClient(currentItem()->data(Qt::DisplayRole).toString());
	
	if(client) {
		if(client->getStatus(CLIENT_STATUS_CONNECTED))
			rebootAction->setEnabled(true);
		
		if(client->getStatus(CLIENT_STATUS_DISCONNECTED))
			rebootAction->setEnabled(false);
		
		if(client->getGroup() != NULL) {
			if(client->getGroup()->getStatus(CGroup::STATUS_THROWED))
				rebootAction->setEnabled(false);
		}
	}
	contextMenu->exec(event->globalPos());
}

void ClientsListWidget::rebootClient()
{
	if(!currentItem())
		return;
	
	PBMonitor->rebootClient(ClientManager::instance()->findClient(currentItem()->data(1).toString()));
}

void ClientsListWidget::renameClient()
{
	if(!currentItem())
		return;
	
	bool ok;
	QString error;
	QString newName = QInputDialog::getText(this, "Backharddi-Net Monitor", tr("Input a new client name."), QLineEdit::Normal, "", &ok);
	
	if(ok && !newName.isEmpty()) {
		ClientManager::instance()->renameClient(ClientManager::instance()->findClient(currentItem()->data(1).toString()), newName, error);
	 
		if(!error.isEmpty()) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", error, QMessageBox::Ok);
			return;
		}
	}
}



