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
#ifndef GROUPSLIST_H
#define GROUPSLIST_H

#include <QListWidget>
#include <QIcon>

class CGroup;
class QStringList;
class QMimeData;
class QPoint;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class GroupsList : public QListWidget
{
	Q_OBJECT
public:
     GroupsList(QWidget *parent = 0);

protected:
	bool event(QEvent *event);
	virtual QStringList mimeTypes() const;
	virtual QMimeData *mimeData(const QList<QListWidgetItem*> items) const;	
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	
private slots:
	void createItem(CGroup *group);
	void renameGroup(CGroup *group);
	void deleteGroup(CGroup *group);
	
signals:
	void signalDrawGroup(CGroup*);

private:	
	QIcon 	groupIcon;
};

#endif
