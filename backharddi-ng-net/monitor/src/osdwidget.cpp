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

#include "osdwidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QImage>
#include <QIcon>
#include <QCursor>
#include <QLocale>
#include <QPixmap>
#include <QPainter>
#include <QBitmap>

#include <X11/Xlib.h>

using namespace Qt;

OSDWidget::OSDWidget( QWidget* parent, const char* name, Type type )
	: QWidget( parent, name, Qt::WType_TopLevel | Qt::WNoAutoErase | Qt::WStyle_Customize | Qt::WX11BypassWM | Qt::WStyle_StaysOnTop ),
	  m_dirty(true),
	  m_progress(0),
	  m_dragging(false),
	  m_screen(0),
	  m_location(s_outerMargin, s_outerMargin),
	  m_type(type)
{
	setFocusPolicy( Qt::NoFocus );
	setBackgroundMode( Qt::NoBackground );

	hideAction = new QAction( tr( "Close OSD" ), this );
	contextMenu = new QMenu(this);

	contextMenu->addAction(hideAction);
	
	connect(hideAction, SIGNAL(triggered()), this, SLOT(hide()));
}


OSDWidget::~OSDWidget()
{
	delete hideAction;
	delete contextMenu;
}

void OSDWidget::show()
{
  	// start with 0 progress
	setProgress(0);

	if( m_dirty )
		renderOSD();
  
	QWidget::show();
}


void OSDWidget::setText( const QString& text )
{
	if( m_text != text ) {
		m_text = text;
		refresh();
	}
}


void OSDWidget::setProgress( int p )
{	
	if( m_progress != p ) {
		m_progress = p;
		refresh();
	}
}


void OSDWidget::setlocation( const QPoint& p )
{
	m_location = p;
	relocation();
}

void OSDWidget::setType( const Type type )
{
	switch( type ) {
		case OSDWidget::Notification:
			break;
		case OSDWidget::Information:
			break;
		case OSDWidget::Interact:
			break;
	}
}

void OSDWidget::setOsdSize( QSize size )
{
	m_size = size;
}

void OSDWidget::setOsdProgressbar( QRect bar )
{
	m_progressbar = bar;
}

void OSDWidget::setOsdFrame( QRect frame )
{
	m_frame = frame;
}

void OSDWidget::setIcon( QPixmap icon )
{
	m_icon = icon;
}

void OSDWidget::setMargin( int margin )
{
	m_margin = margin;
}

void OSDWidget::refresh()
{
	if( isVisible() )
		renderOSD();
	else
		m_dirty = true;
}


void OSDWidget::renderOSD()
{
	QPixmap icon("/local/Development/backharddi_monitor/kdevelop/src/icons/MainWindow/pango32x32.png");
	
	int margin = 10;
	int textWidth = fontMetrics().width( m_text );
	
	// Do not change the frame size while is not required by text.	
	QSize newSize( QMAX( QMAX( 2*margin + icon.width() + margin + textWidth, 100 ), width() ), QMAX( 2*margin + icon.height(), 2*margin + fontMetrics().height()*2 ) );		
	
	// Resize the OSD buffer to allocate the OSD Size
	m_osdBuffer.resize( newSize );	
	
	// Create the QPainter Object and sets his pen to white	
	QPainter p( &m_osdBuffer );
	p.setPen( Qt::white );	

	// Draw the OSD frame	
	QRect thisRect( 0, 0, newSize.width(), newSize.height() );
	p.fillRect( thisRect, Qt::black );
	p.drawRect( thisRect );

	// Draw the application icon
	p.drawPixmap( margin, (newSize.height()-icon.height())/2, icon );
    
	// Draw the On-Display text
	QSize textSize = fontMetrics().size( 0, m_text );
	int textX = 2*margin + icon.width();
	int textY = margin + fontMetrics().ascent();
	p.drawText( textX, textY, m_text );	
    
	// Draw the OSD progressbar
	textY += fontMetrics().descent() + 4;
	QRect osdProgressbar( textX, textY, newSize.width()-textX-margin, newSize.height()-textY-margin );
	p.drawRect( osdProgressbar );
	osdProgressbar.setWidth( m_progress > 0 ? m_progress*osdProgressbar.width()/100 : 0 );
	p.fillRect( osdProgressbar, Qt::darkGreen );
	
	// relocation the osd
	relocation( newSize );		
	
	update();	
}


void OSDWidget::setScreen( int screen )
{
	const int n = QApplication::desktop()->numScreens();	
	m_screen = (screen >= n) ? n-1 : screen;
	relocation();
}


void OSDWidget::relocation( QSize newSize )
{
	if( !newSize.isValid() )
		newSize = size();

	QPoint newPos = m_location;
	const QRect& screen = QApplication::desktop()->screenGeometry( m_screen );

  	// now to properly resize if put into one of the corners we interpret the location
  	// depending on the quadrant
	int midH = screen.width()/2;
	int midV = screen.height()/2;
	if( newPos.x() > midH )
		newPos.rx() -= newSize.width();
	if( newPos.y() > midV )
		newPos.ry() -= newSize.height();

	newPos = fixuplocation( newPos );
 
  	// correct for screen location
	newPos += screen.topLeft();
  
  	// ensure we are painted before we move
	if( isVisible() )
		update();

  	// fancy X11 move+resize, reduces visual artifacts
	XMoveResizeWindow( x11Display(), winId(), newPos.x(), newPos.y(), newSize.width(), newSize.height() );
}


void OSDWidget::paintEvent( QPaintEvent * )
{	
	QPainter p( this );	
	p.drawPixmap(this->rect(), m_osdBuffer);	
}


void OSDWidget::mousePressEvent( QMouseEvent* e )
{
	m_dragOffset = e->pos();

	if( e->button() == LeftButton && !m_dragging ) {
		grabMouse( QApplication::desktop()->cursor() );
		m_dragging = true;
	}
}


void OSDWidget::mouseReleaseEvent( QMouseEvent* )
{
	if( m_dragging ) {
		m_dragging = false;
		releaseMouse();
	}
}


void OSDWidget::mouseMoveEvent( QMouseEvent* e )
{
	if( m_dragging && this == mouseGrabber() ) {
    		// check if the osd has been dragged out of the current screen
		int currentScreen = QApplication::desktop()->screenNumber( e->globalPos() );
		if( currentScreen != -1 )
			m_screen = currentScreen;

		const QRect& screen = QApplication::desktop()->screenGeometry( m_screen );
    
    		// make sure the location is valid
		m_location = fixuplocation( e->globalPos() - m_dragOffset - screen.topLeft() );

    		// move us to the new location
		move( m_location );

    		// fix the location
		int midH = screen.width()/2;
		int midV = screen.height()/2;
		if( m_location.x() + width() > midH )
			m_location.rx() += width();
		if( m_location.y() + height() > midV )
			m_location.ry() += height();
	}
}

void OSDWidget::contextMenuEvent( QContextMenuEvent *event )
{
	contextMenu->exec(event->globalPos());
}

QPoint OSDWidget::fixuplocation( const QPoint& pp )
{
	QPoint p(pp);
	const QRect& screen = QApplication::desktop()->screenGeometry( m_screen );
	int maxY = screen.height() - height() - s_outerMargin;
	int maxX = screen.width() - width() - s_outerMargin;

	if( p.y() < s_outerMargin )
		p.ry() = s_outerMargin;
	else if( p.y() > maxY )
		p.ry() = maxY;

	if( p.x() < s_outerMargin )
		p.rx() = s_outerMargin;
	else if( p.x() > maxX )
		p.rx() = screen.width() - s_outerMargin - width();

	p += screen.topLeft();

	return p;
}


