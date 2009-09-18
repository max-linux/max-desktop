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
#ifndef GROUPDATABASE_H
#define GROUPDATABASE_H

class GroupDBPriv;
class QStringList;
class QString;
class QDate;
class CGroup;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class GroupDatabase
{
public:
	GroupDatabase();
	~GroupDatabase();

	int addGroup(const QString &name, const QDate &date, int taskId, QString &error);
	void addClientsRelation(const int groupId, const QString &clientId);
	void deleteClientsRelation(const int groupId, const QString &clientId);
	void setGroupName(int id, const QString &newName, QString &error);
	void setGroupDate(int id, const QDate &newDate, QString &error);
	
	bool setConfigured(bool configured, QString &error);
	
	void deleteGroup(int groupId, QString &error);
	
	bool exec(const QString &sql, QString &error, const bool debug = false);
	bool select(const QString &sql,  QString &error, QStringList *const values = 0, const bool debug = false);	
	
	void loadClientsFromGroup(const int idgroup, QString &error);	
	void loadGroupFromDB(const int idgroup, QString &error);
	void loadGroupConfigFromDB(const int idgroup, QString &error);
	void saveGroupConfig(QString settings, QString &error);
	void updateGroupConfig(QString groupSettings, QString &error);
	
	bool isRunning();
	bool isRegistered(QString table, QString target, QString value);
	
private:
	QString escapeString(QString str) const;
	
	GroupDBPriv *d;
};

#endif
