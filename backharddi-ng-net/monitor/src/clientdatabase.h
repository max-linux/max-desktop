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
#ifndef clientdatabase_h
#define clientdatabase_h

class QStringList;
class QString;
class QDate;
class ClientDBPriv;
class CGroup;
class CClient;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class ClientDatabase 
{
public:
	ClientDatabase();
	~ClientDatabase();
	
	int addClient(const QString &name, const QDate &date, const QString id, const QString mac, QString &error);
	void setClientName(const QString &clientId, const QString &name, QString &error);
	void setClientDate(const QString &clientId, const QDate &date, QString &error);
	
	QString getClientName(QString clietId, QString &error);
	QString getClientId(const QString &clientName, QString &error);
	QDate getDate(QString clientId, QString &error);
	QString getMac(QString clientId, QString &rror);
	
	void loadClientFromDB(QString &error, QString clientId);
	void deleteClient(QString clientId, QString &error);
	
	bool exec(const QString &sql, QString &error, const bool debug = false);
	bool select(const QString &sql,  QString &error, QStringList *const values = 0, const bool debug = false);
     
	bool isRunning();
     
	
	void loadClientGroup(QString &error, QString clientId);

	private:
		QString escapeString(QString str) const;
     
		ClientDBPriv* d;
};

#endif
