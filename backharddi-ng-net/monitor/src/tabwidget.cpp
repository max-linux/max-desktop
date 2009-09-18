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
#include "tabwidget.h"
#include "qtermwidget.h"
#include "backharddinet_monitor.h"

#include <QMouseEvent>
#include <QTabBar>
#include <QMenu>
#include <QDebug>

TabWidget::TabWidget(QWidget* parent): QTabWidget(parent)
{
	// Set the close button on tab
	m_button = new QToolButton(this);
	m_button->hide();
	m_button->setIcon(QIcon(":/icons/button_cancel.png"));
	m_button->setGeometry(0,0,15,15);
	
	setCloseButton(false);
	
	// Prepare event handler
	tabBar()->installEventFilter(this);
	
	m_selectedItem = -1;
	m_crossButtons = false;
	
	setMouseTracking(true);
}


TabWidget::~TabWidget()
{}

void TabWidget::setCloseButton(bool b)
{
	m_crossButtons = b;
	if(!m_crossButtons)
		m_button->hide();
}

void TabWidget::closeTab()
{		
	if(m_selectedItem == 0)
		return;
	
	extern BackharddiNet_Monitor *PBMonitor;
	m_button->hide();	
	PBMonitor->closeTab(qobject_cast<QTermWidget*>(widget(m_selectedItem)));
}

bool TabWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (obj==tabBar())
	{
		if (event->type() == QEvent::Leave)
		{
			QPoint point = m_button->mapToGlobal( QPoint(0, 0) );
			QRect rect(point.x(), point.y(), m_button->width(), m_button->height() );
			if ( !rect.contains( QCursor::pos() ) )
				m_button->hide();
		}
		else if (event->type() == QEvent::HoverMove && m_crossButtons )
		{
			QHoverEvent *mouseEvent = static_cast<QHoverEvent *>(event);
			m_mousePos = mouseEvent->pos();
			for (int i=0; i<tabBar()->count(); i++)
			{
				if ( tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
				{
					m_selectedItem = i;
					break;
				}
			}
			
			if(m_selectedItem == 0)
				return QTabWidget::eventFilter( obj, event);
			
			m_button->setGeometry(tabBar()->tabRect(m_selectedItem).x()+tabBar()->tabRect(m_selectedItem).width()-m_button->width()-5, 5, m_button->width(), m_button->height());
			m_button->show();
			connect(m_button, SIGNAL(pressed()), this, SLOT(closeTab()));
		}
		else if (event->type() == QEvent::MouseButtonRelease )
		{
			qApp->restoreOverrideCursor();
		}
		else if (event->type() == QEvent::MouseButtonPress )
		{
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
			for (int i=0; i<tabBar()->count(); i++)
			{
				if ( tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
				{
					m_selectedItem = i;
					break;
				}
			}
			if ( mouseEvent->button() == Qt::LeftButton )
				qApp->setOverrideCursor( Qt::OpenHandCursor );
			
			if ( mouseEvent->button() == Qt::RightButton )
			{
				QMenu *menu = new QMenu(this);
				connect(menu->addAction(QIcon(":/icons/button_cancel.png"), tr("Close Tab")), SIGNAL(triggered()), this, SLOT(closeTab()) );
				menu->exec(mouseEvent->globalPos());
				delete menu;
			}
		}
		else if (event->type() == QEvent::MouseMove )
		{
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
			for (int i=0; i<tabBar()->count(); i++)
			{
				if ( tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
				{
					if ( swapTabs(i, m_selectedItem) )
					{
						setCurrentWidget(widget(i));
						update();
						int x;
						if ( !tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
						{
							if ( tabBar()->tabRect(m_selectedItem).x() < tabBar()->tabRect(i).x() )
								x = tabBar()->tabRect(i).x();
							else
								x = tabBar()->tabRect(i).x()+(tabBar()->tabRect(i).width()-(qAbs(tabBar()->tabRect(i).width()-tabBar()->tabRect(m_selectedItem).width())));
							QPoint point =  QPoint( x, mouseEvent->pos().y() );
							point =  widget(i)->mapToGlobal( point );
							m_selectedItem = i;
							QCursor::setPos ( point.x(), QCursor::pos().y() );
						}
						m_selectedItem = i;
						break;
					}
				}
			}
		}
	}
	return QTabWidget::eventFilter( obj, event);
}

bool TabWidget::swapTabs(int tab1, int tab2)
{
	if (tab1 == tab2)
		return false;
	
	int t1 = qMin(tab1,tab2);
	int t2 = qMax(tab1,tab2);

	tab1=t1;
	tab2=t2;

	QString name1 = tabBar()->tabText(tab1);
	QString name2 = tabBar()->tabText(tab2);

	QWidget *tabWin1 = widget(tab1);
	QWidget *tabWin2 = widget(tab2);

	removeTab(tab2);
	removeTab(tab1);

	insertTab(tab1,tabWin2,name2);
	insertTab(tab2,tabWin1,name1);
	return true;
}

