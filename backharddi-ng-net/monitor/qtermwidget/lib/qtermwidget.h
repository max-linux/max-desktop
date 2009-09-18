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
						    

#ifndef _Q_TERM_WIDGET
#define _Q_TERM_WIDGET

#include <QtGui>

class QTermWidget : public QWidget
{
    Q_OBJECT
public:

    //Creation of widget
    QTermWidget(int startnow = 1, //start shell programm immediatelly
		QWidget *parent = 0);
    ~QTermWidget();

    //start shell program if it was not started in constructor
    void startShellProgram();
    
    //look-n-feel, if you don`t like defaults

    //	Terminal font
    // Default is application font with family Monospace, size 10
    void setTerminalFont(QFont &font); 
    
    //	Shell program, default is /bin/bash
    void setShellProgram(QString &progname);
    
    // Shell program args, default is none
    void setArgs(QStringList &args);
    
    //Text codec, default is UTF-8
    void setTextCodec(QTextCodec *codec);
    
    //set size
    void setSize(int h, int v);
    
    // History size for scrolling 
    void setHistorySize(int lines); //infinite if lines < 0
    
    // Send some text to terminal
    void sendText(QString &text);
            
signals:
    void finished();
        
protected: 
    void *createSession();
    void *createTerminalDisplay(void *session);
    virtual void resizeEvent(QResizeEvent *);
    
protected slots:
    void sessionFinished();        
    
private:
    void init();
    
    void *_session;
    void *_terminalDisplay;
};


//Maybe useful, maybe not

#ifdef __cplusplus
extern "C"
#endif
void *createTermWidget(int startnow, void *parent); 

#endif

