/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
		
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
				
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
						

#include "qtermwidget.h"

#include "Session.h"
#include "TerminalDisplay.h"

using namespace Konsole;

void *createTermWidget(int startnow, void *parent)
{ 
    return (void*) new QTermWidget(startnow, (QWidget*)parent); 
}


QTermWidget::QTermWidget(int startnow, QWidget *parent)
:QWidget(parent)
{
    init();

    if (startnow && _session) {
	((Session*)_session)->run();
    }
    
    this->setFocus( Qt::OtherFocusReason );
    ((TerminalDisplay*)_terminalDisplay)->resize(this->size());
}

void QTermWidget::startShellProgram()
{
    if ( ((Session*)_session)->isRunning() )
	return;
	
    ((Session*)_session)->run();
}

void QTermWidget::init()
{
    _session = createSession();
    _terminalDisplay = createTerminalDisplay(_session);
    
    ((TerminalDisplay*)_terminalDisplay)->setSize(80, 40);
    
    QFont font = QApplication::font(); 
    font.setFamily("Monospace");
    font.setPointSize(10);
    font.setStyleHint(QFont::TypeWriter);
    setTerminalFont(font);
    
    ((Session*)_session)->addView((TerminalDisplay*)_terminalDisplay);
    
    connect(((Session*)_session), SIGNAL(finished()), this, SLOT(sessionFinished()));
}


QTermWidget::~QTermWidget()
{
    emit destroyed();
}


void QTermWidget::setTerminalFont(QFont &font)
{
    if (!_terminalDisplay)
	return;
    ((TerminalDisplay*)_terminalDisplay)->setVTFont(font);
}

void QTermWidget::setShellProgram(QString &progname)
{
    if (!_session)
	return;
    ((Session*)_session)->setProgram(progname);	
}

void QTermWidget::setArgs(QStringList &args)
{
    if (!_session)
	return;
    ((Session*)_session)->setArguments(args);	
}

void QTermWidget::setTextCodec(QTextCodec *codec)
{
    if (!_session)
	return;
    ((Session*)_session)->setCodec(codec);	
}

void QTermWidget::setSize(int h, int v)
{
    if (!_terminalDisplay)
	return;
    ((TerminalDisplay*)_terminalDisplay)->setSize(h, v);
}

void QTermWidget::setHistorySize(int lines)
{
    if (lines < 0)
        ((Session*)_session)->setHistoryType(HistoryTypeFile());
    else
	((Session*)_session)->setHistoryType(HistoryTypeBuffer(lines));
}

void QTermWidget::sendText(QString &text)
{
    ((Session*)_session)->sendText(text); 
}

void *QTermWidget::createSession()
{
    Session *session = new Session();

    session->setTitle(Session::NameRole, "QTermWidget");
    session->setProgram("/bin/bash");
    QStringList args("");
    session->setArguments(args);
    session->setAutoClose(true);
		    
    session->setCodec(QTextCodec::codecForName("UTF-8"));
			
    session->setFlowControlEnabled(true);
    session->setHistoryType(HistoryTypeBuffer(1000));
    
    session->setDarkBackground(true);
	    
    session->setKeyBindings("");	    
    return (void*)session;
}

//TerminalDisplay *QTermWidget::createTerminalDisplay(Session *session)
void *QTermWidget::createTerminalDisplay(void *session)
{
//    TerminalDisplay* display = new TerminalDisplay(this);
    TerminalDisplay* display = new TerminalDisplay(this);
    
    display->setBellMode(TerminalDisplay::NotifyBell);
    display->setTerminalSizeHint(true);
    display->setTripleClickMode(TerminalDisplay::SelectWholeLine);
    display->setTerminalSizeStartup(true);

//    display->setScrollBarPosition(TerminalDisplay::ScrollBarRight);
    display->setScrollBarPosition(TerminalDisplay::NoScrollBar);

    display->setRandomSeed(((Session*)session)->sessionId() * 31);
    
    
    return (void*)display;
}
	
void QTermWidget::resizeEvent(QResizeEvent*)
{
//qDebug("global window resizing...with %d %d", this->size().width(), this->size().height());
    ((TerminalDisplay*)_terminalDisplay)->resize(this->size());

}


void QTermWidget::sessionFinished()
{
    emit finished();
}

	
//#include "moc_consoleq.cpp"

