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
#ifndef clientmanager_h
#define clientmanager_h

#include "clientslist.h"
#include "osdwidget.h"

#include <QObject>
#include <QHash>

class ClientManagerPriv;
class CClient;
class QDate;
class QListWidgetItem;

typedef QHash<QByteArray, CClient *> PClientMap;
typedef QHash<QString, OSDWidget *> PClientOsdMap;
typedef QHash<QString, QListWidgetItem *> PClientItemMap;
typedef QHash<QString, QTreeWidgetItem *> PClientTreeItemMap;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class ClientManager : public QObject
{
Q_OBJECT
public:
	ClientManager();
	~ClientManager();
	
	static ClientManager *instance();
	static QByteArray toBase64(QString string);
	ClientsList *getGraphicList();
	void setGraphicList(QWidget *parent);
	
	CClient *findClient(QByteArray id);
	CClient *findClient(QString name);
	CClient *findClientByIP(QString clientIP);

	CClient *createClient(const QString &name, const QString &id, const QDate &date, const QString &mac, QString &error, bool insert = false);
	
	void setCurrentClient(CClient *client);
	
	CClient *currentClient();
	
	bool renameClient(CClient *client, const QString &newName, QString &errMsg);
	bool deleteClient(CClient *client, QString &errMSg);
	
	PClientMap::iterator getClientIterator(CClient *client);	
	
	void insertClient(CClient *client);
	void loadClient(QString &error, QString clientId);
	
	void setCurrent(QList<QListWidgetItem *> clients);
	QList<QListWidgetItem *> getCurrent() const;
	
	PClientMap getClientsMap();
	void rebootClient(CClient *client);
	
	void pingClient(CClient *client);
	
	// OSD related stuff
	OSDWidget *getClientOsd( const CClient *client );
	void addOsdToMap( const CClient *client, OSDWidget *osd );
	void removeOsdFromMap( const CClient *client );
	bool osdContains( const QString &name );	
	bool osdContains( const CClient *client );
	
	// QTreeItemWidget and QListWidgetItem related stuff	
	QTreeWidgetItem *getClientTreeItem( const CClient *client );
	QListWidgetItem *getClientItem( const CClient *client );
	void addItemToMap( bool tree, const CClient *client, QTreeWidgetItem *treeitem = 0, QListWidgetItem *listitem = 0);
	void removeItemFromMap( bool tree, const CClient *client );
	bool itemContains( bool tree, const CClient *client );
	bool itemContains( bool tree, const QString &name );
	
private:
	static ClientManager *m_instance;
	ClientsList 	*clientsListWidget;
	ClientManagerPriv *d;	
	
	void setupClientName(const QString &key, CClient *client);
	void setupClientTooltip(CClient *client);
	
private slots:
	void drawClient(CClient *client);
	
signals:
	void signalClientAdded(CClient *);
	void signalClientDeleted(CClient *);	
	void signalClientCurrentModified(CClient *);
	void signalAllClientsLoaded();
	void signalClientRenamed(CClient *);
	void signalDrawClient(CClient *);
};

#endif
