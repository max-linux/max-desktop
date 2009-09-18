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
#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include "osdwidget.h"

#include <QObject>
#include <QHash>

class GroupManagerPriv;
class CGroup;
class QDate;
class CClient;


typedef QHash<int , CGroup *> PGroupMap;
typedef QMap< unsigned int, OSDWidget * > PGroupOsdMap;
typedef QMap< unsigned int, QListWidgetItem *> PGroupItemMap;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class GroupManager : public QObject
{
Q_OBJECT
public:
    GroupManager(QObject *parent = 0);
    ~GroupManager();

    static GroupManager *instance();
    static QByteArray toBase64(QString string);

    CGroup *findGroup(int id);
    CGroup *findGroup(QString name);

    CGroup *createGroup(const QString &name, const QDate &date, unsigned int idtask, QString &errMsg, int groupId=-1, bool configured=false);

    void setCurrentGroup(CGroup *group);

    CGroup *currentGroup();

    bool renameGroup(CGroup *group, const QString &newName, QString &errMsg);
    bool deleteGroup(CGroup *group, QString &errMSg);

    PGroupMap::iterator getGroupIterator(CGroup *group);

    void insertGroup(CGroup *group);	
    void loadGroup(int groupId, QString &error);
    void loadGroupClients(int groupId, QString &error);
    void loadGroupConfig(int groupId, QString &error);
    void saveGroupConfig(CGroup *group, QString &error);
    void saveGroupClientsRelation(CGroup *group);
    
    bool throwGroup(CGroup *group, QString &error);
    void rebootGroup(CGroup *group);
    
    // OSD Related stuff
    OSDWidget *getGroupOsd( const CGroup *group );
    void addOsdToMap( const CGroup *group, OSDWidget *osd );
    void removeOsdFromMap( const CGroup *group );
    bool osdContains( const CGroup *group );
    bool osdContains( const unsigned int groupId );
    
    // QListWidgetItem related Stuff
    QListWidgetItem *getGroupItem( const CGroup *group );
    void addItemToMap( const CGroup *group, QListWidgetItem *item);
    void removeItemFromMap( const CGroup *group );
    bool itemContains( const CGroup *group );
    bool itemContains( const unsigned int groupId );

    inline void emitConfigure(CGroup *group) {
	    emit signalConfigureGroup(group);
    }

    void completeGroupClient(CClient *client);
    CGroup *findGroupByClientName(const QString &clientName);

private:
	static GroupManager *m_instance;
	GroupManagerPriv *d;	
		
public slots:
	void setConfigured(CGroup *group);
	void completeGroup(CGroup *group);
	void osdUpdateProgress( const CGroup *group );

private slots:
	void drawGroup(CGroup *group);
	bool deleteGroup(CGroup *group);
	void updateGroupRelation(CGroup *group, QString clientName);

signals:
	void signalGroupAdded(CGroup *);
	void signalGroupDeleted(CGroup *);	
	void signalGroupCurrentModified(CGroup *);
	void signalGroupRenamed(CGroup *);
	void signalDrawGroup(CGroup *);
	void signalConfigureGroup(CGroup *);
};

#endif
