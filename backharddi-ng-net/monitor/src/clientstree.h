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
#ifndef clientstree_h
#define clientstree_h

#include <QListWidget>
#include <QIcon>

class CClient;
class QStringList;
class QMimeData;
class QPoint;
class QMenu;
class QAction;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class ClientsListWidget : public QListWidget
{
	Q_OBJECT
public:
     ClientsListWidget(QWidget *parent = 0);
	QSize sizeHint() const;
	
protected:
	bool event(QEvent *event);	
	virtual QStringList mimeTypes() const;
	virtual QMimeData *mimeData(const QList<QListWidgetItem*> items) const;	
	virtual void mouseDoubleClickEvent(QMouseEvent *event);	
	virtual void dragEnterEvent(QDragEnterEvent *event);	
	virtual void dragMoveEvent(QDragMoveEvent *event);	
	virtual void dropEvent(QDropEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);
	
private:
	void createContextMenu();
	void createActions();
	
private slots:
	void createItem(CClient *client);
	void selectionChanged();	
	void renameClient();
	void rebootClient();
	
signals:
	void signalDrawClient(CClient*);

private:	
	QIcon clientIcon;	
	QMenu *contextMenu;
	QAction *renameAction;
	QAction *rebootAction;
};

#endif
