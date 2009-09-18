/***************************************************************************
 *   Copyright (C) 2008 by Oscar Campos Ruiz                               *
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
#ifndef shellwidget_h
#define shellwidget_h

#include <QtGui>
#include <QByteArray>
#include "ui_ShellWidgetBase.h"

#include <qtermwidget.h>


/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class ShellWidget : public QTermWidget, private Ui::ShellWidgetBase
{
	Q_OBJECT
public:
	ShellWidget(QWidget *parent = 0, const char *name = 0, const QByteArray &clientId = QByteArray());
    	virtual ~ShellWidget();
	
	bool isRunning();	
	QString getClient() const;
	void setClient(const QString &name);
	
private:
	Ui::ShellWidgetBase ui;
	
	QString m_shellName;
	QString m_clientName;
	bool m_isRunning;
	QByteArray m_clientId;
};

#endif
