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
#include "groupslist.h"
#include "groupmanager.h"
#include "cgroup.h"
#include "Loader.h"
#include "backharddinet_monitor.h"

#include <QtCore>
#include <QtGui>
#include <QMimeData>

extern Loader settings;

GroupsList::GroupsList(QWidget *parent) : QListWidget(parent)
{
	groupIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirIcon));
	setMouseTracking(true);
	
	connect(GroupManager::instance(), SIGNAL(signalGroupDeleted(CGroup*)), this, SLOT(deleteGroup(CGroup*)));
	connect(GroupManager::instance(), SIGNAL(signalGroupAdded(CGroup*)), this, SLOT(createItem(CGroup*)));
	connect(GroupManager::instance(), SIGNAL(signalGroupRenamed(CGroup*)), this, SLOT(renameGroup(CGroup*)));
	connect(this, SIGNAL(signalDrawGroup(CGroup*)), GroupManager::instance(), SLOT(drawGroup(CGroup*)));	
}

bool GroupsList::event(QEvent *event)
{
	// Added for future features extensions
	return QListWidget::event(event);
}

QStringList GroupsList::mimeTypes() const
{
	QStringList types;
	types << "application/vnd.group.item";
	return types;
}

QMimeData *GroupsList::mimeData(const QList<QListWidgetItem*> items) const
{
	Q_UNUSED(items);
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;
	
	mimeData->setData("application/vnd.group.item", encodedData);
	return mimeData;
}

void GroupsList::mouseDoubleClickEvent(QMouseEvent *event)
{
	extern BackharddiNet_Monitor *PBMonitor;
	if(!currentItem())
		return;
	
	if(event->button() == Qt::LeftButton) {
		PBMonitor->showMessage(tr("Backharddi Tips"), tr("Sorry, this function is not implemented yet.\nVisit us at: http://backharddi.ideseneca.es"));
	}
}

void GroupsList::createItem(CGroup *group)
{
	QListWidgetItem *tmpItem = new QListWidgetItem(this);
	tmpItem->setText(group->getName());
	tmpItem->setFlags(tmpItem->flags() | Qt::ItemIsEditable);
	tmpItem->setToolTip(QString(""));
	
	GroupManager::instance()->addItemToMap(group, tmpItem);
	
	emit(signalDrawGroup(group));
}

void GroupsList::renameGroup(CGroup *group)
{	
	QListWidgetItem *tmpItem = GroupManager::instance()->getGroupItem(group);
	tmpItem->setText(group->getName());
	tmpItem->setToolTip(tr("Configured Multicast Group"));
	tmpItem->setTextAlignment(Qt::AlignHCenter);
	tmpItem->setFont(QFont("Helvetica [Cronyx]", 10, QFont::Bold));
	tmpItem->setTextColor(QColor::fromRgb(223, 116, 23));
}

void GroupsList::deleteGroup(CGroup *group)
{	
	GroupManager::instance()->removeItemFromMap(group);
}

