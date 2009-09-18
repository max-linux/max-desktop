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
#include "shellwidget.h"

ShellWidget::ShellWidget(QWidget *parent, const char *name, const QByteArray &clientId) : QTermWidget(3, parent)
{
	ui.setupUi(this);
	m_shellName = name;
	m_clientId = clientId;
	m_isRunning = true;
}


ShellWidget::~ShellWidget()
{
}

bool ShellWidget::isRunning()
{
	return m_isRunning;
}

void ShellWidget::setClient(const QString &name)
{
	m_clientName = name;
}

QString ShellWidget::getClient() const
{
	return m_clientName;
}



