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
#include "generaltab.h"
#include "Task.h"
#include "taskstree.h"

#include <QtGui>
#include <QTreeView>

GeneralTab::GeneralTab(CTask *task, QWidget *parent) : QWidget(parent)
{
	treeWidget = new TasksTree(this);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	treeWidget->setTaskObject(task);
	treeWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
	treeWidget->setDragEnabled(true);
	treeWidget->setAcceptDrops(true);	
	mainLayout->addWidget(treeWidget);
	setLayout(mainLayout);
	setAcceptDrops(true);
}

GeneralTab::~GeneralTab()
{
     delete treeWidget;
}

TasksTree *GeneralTab::getTree()
{
	return treeWidget;
}



