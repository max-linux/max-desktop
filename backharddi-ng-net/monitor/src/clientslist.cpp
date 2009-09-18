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
#include "clientslist.h"
#include "cclient.h"
#include "clientstree.h"

#include <QtGui>
#include <QListView>

ClientsList::ClientsList(QWidget *parent) : QWidget(parent)
{
	listWidget = new ClientsListWidget(this);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	listWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);	
	listWidget->setDragEnabled(true);
	listWidget->setAcceptDrops(true);	
	mainLayout->addWidget(listWidget);
	setLayout(mainLayout);
	setAcceptDrops(true);	
}

ClientsList::~ClientsList()
{
	delete listWidget;
}

ClientsListWidget *ClientsList::getList()
{
	return listWidget;
}

