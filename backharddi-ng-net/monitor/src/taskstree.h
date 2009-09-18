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
#ifndef taskstree_h
#define taskstree_h

#include <QIcon>
#include <QTimer>
#include <QTreeWidget>

class CTask;
class CGroup;
class CClient;
class QStringList;
class QMimeData;
class QPoint;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class TasksTree : public QTreeWidget
{
	Q_OBJECT
public:
    	TasksTree(QWidget *parent = 0);
	
	void setTaskObject(CTask *task);
	QSize sizeHint() const;
	
public slots:
	void setAutoRefresh(bool autoRefresh);	
	void maybeRefresh();
	void refresh();
	
protected:
	void contextMenuEvent(QContextMenuEvent *event);
	bool event(QEvent *event);	
	QStringList mimeTypes() const;
	void mouseDoubleClickEvent(QMouseEvent *event);	
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);	
	
	QMimeData *mimeData(const QList<QTreeWidgetItem*> items) const;
	bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);
	Qt::DropActions supportedDropActions() const;
	
private slots:
	void updateGroup(QTreeWidgetItem *item);
	void redrawTask();
	void paintStatusColors(QTreeWidgetItem *item, CClient *client);	
	void startShell();
	
private:
	void createContextMenu();
	void createActions();
	void updateChildItems(QTreeWidgetItem *parent);
	QTreeWidgetItem *createItem(const QString &text, QTreeWidgetItem *parent, int index);
	QTreeWidgetItem *childAt(QTreeWidgetItem *parent, int index);
	int childCount(QTreeWidgetItem *parent);
	int findChild(QTreeWidgetItem *parent, const QString &text, int startIndex);
	void moveItemForward(QTreeWidgetItem *parent, int oldIndex, int newIndex);
	QTreeWidgetItem *findItem(unsigned int itemId);	
	QTreeWidgetItem *findItem(QByteArray itemId);

	CTask *task;
	QTimer refreshTimer;
	bool autoRefresh;
	QIcon groupIcon;
	QIcon clientIcon;
	QMenu *contextMenu;
	QAction *removeGroup;
	QAction *removeClient;
	QAction *throwGroup;
	QAction *reboot;
	QAction *throwAll;
	QAction *openShell;
};

#endif
