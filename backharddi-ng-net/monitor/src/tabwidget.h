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
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>

class QToolButton;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class TabWidget : public QTabWidget
{
	Q_OBJECT
public:
	TabWidget(QWidget* parent);
     virtual ~TabWidget();
	void setCloseButton(bool b);
	
private slots:
	void closeTab();
	
protected:
	bool eventFilter(QObject *obj, QEvent *event);
	
private:
	bool swapTabs(int tab1, int tab2);
	
	QToolButton	*m_button;
	QPoint		m_mousePos;
	bool			m_crossButtons;
	qint32		m_selectedItem;

};

#endif
