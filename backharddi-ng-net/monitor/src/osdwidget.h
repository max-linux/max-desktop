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
#ifndef OSDWIDGET_H
#define OSDWIDGET_H

#include <QWidget>
#include <QPixmap>

class QPaintEvent;
class QMouseEvent;

/**
 * 	This OSD System is inspired and based on k3b one.
 * 	Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
 *	Copyright (C) Oscar Campos Ruiz <oscar.campos@edmanufacturer.es> 
 * 
 *	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
 */
class OSDWidget : public QWidget
{
	Q_OBJECT	
public:
	enum Type { Information, Notification, Interact };

	OSDWidget(QWidget *parent = 0, const char *name = "osd", Type type = Notification);
	~OSDWidget();
    
	void setIcon( QPixmap icon );
	QPixmap icon() { return m_icon; }
	
	void setType( const Type type );
	Type type() const { return m_type; }

	void setMargin( int margin );
	int margin() const { return m_margin; }

	int screen() const {  return m_screen; }
	const QPoint& location() const { return m_location; }
	int progress() const { return m_progress; }

	void setOsdSize( QSize size );
	QSize osdSize() const { return m_size; }
	
	void setOsdFrame( QRect frame );
	QRect osdFrame() const { return m_frame; }

	void setOsdProgressbar( QRect bar );
	QRect osdProgressbar() const { return m_progressbar; }	

public slots:
	void setScreen( int );
	void setText( const QString& );
	void setProgress( int );

	void setlocation( const QPoint& );

	void show();

protected:
	void renderOSD();
	void paintEvent( QPaintEvent* );
	void mousePressEvent( QMouseEvent* );
	void mouseReleaseEvent( QMouseEvent* );
	void mouseMoveEvent( QMouseEvent* );
	void contextMenuEvent( QContextMenuEvent *event );
	void relocation( QSize size = QSize() );
	

protected slots:
	void refresh();

private:

	QPoint fixuplocation( const QPoint& p );
	static const int s_outerMargin = 15;
		
	QPixmap 	m_osdBuffer;	
	bool 	m_dirty;
	QString 	m_text;
	int 		m_progress;
	bool 	m_dragging;
	int 		m_margin;
	QPoint 	m_dragOffset;
	int 		m_screen;
	QPoint 	m_location;
	
	QPixmap 	m_icon;
	QSize 	m_size;
	QRect 	m_frame;
	QRect	m_progressbar;
	Type		m_type;

	QMenu	*contextMenu;
	QAction *hideAction;
};

#endif
