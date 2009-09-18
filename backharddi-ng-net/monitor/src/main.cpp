
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

#include "backharddinet_monitor.h"
#include "Database.h"
#include "MonitorServer.h"
#include "Loader.h"
#include "TaskManager.h"
#include "groupmanager.h"
#include "clientmanager.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QTextStream>
#include <QPixmap>
#include <QSplashScreen>
#include <QSettings>
#include <QTest>
#include <QFile>
#include <QDir>

#include <unistd.h>

CDatabase *BDatabase = new CDatabase();
Loader *settings = new Loader();

BackharddiNet_Monitor *PBMonitor = NULL;
MonitorServer *PServer = NULL;

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(application);	
	
	int pID = 0;
	QApplication app(argc, argv);
	
	QFile *tmpFile = new QFile("/tmp/.backharddi-net-monitor.run");
	if(tmpFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(tmpFile);
		QString lastPid = in.readLine();
		
		if(!lastPid.isEmpty()) {
			pID = lastPid.toInt();
		}
		
		QDir procPid;
		if(procPid.exists(QString("/proc/%1/cmdline").arg(pID))) {
			QMessageBox::critical(0, "Backharddi-Net Monitor", QObject::tr("This application has been already started.. aborting initialization."), QMessageBox::Ok);
			tmpFile->close();
			delete tmpFile;
			return 1;
		}
		
		tmpFile->close();
		
		
	}
	
	if(tmpFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
		pID = static_cast<int>(getpid());
		qDebug() << pID;
			
		QTextStream out(tmpFile);
		out.flush();
		out << pID << "\n";
		tmpFile->close();
	}
	
	delete tmpFile;
	
	QString locale = QLocale::system().name();
	
	QTranslator translator;
	translator.load(QString("monitor_") + locale);
	app.installTranslator(&translator);

	QWidget tmp;

	QSplashScreen *splash = 0;
	int screenId = QApplication::desktop()->screenNumber(tmp.geometry().center());
	splash = new QSplashScreen(QPixmap(QLatin1String(":/icons/Splash.jpg")));
	if (QApplication::desktop()->isVirtualDesktop()) {
		QRect srect(0, 0, splash->width(), splash->height());
		splash->move(QApplication::desktop()->availableGeometry(screenId).center() - srect.center() );
	}
	splash->setAttribute(Qt::WA_DeleteOnClose);
	splash->show();

	 // Initialize the database
	splash->showMessage("Preconfiguring the Database Connection");
	BDatabase->StopDatabase();
	BDatabase->StartDatabase();

	 // Initialice the task manager
	TaskManager* man = new TaskManager();
	 
	 // Initialize the group manager
	GroupManager* gman = new GroupManager();

	 // Initialize the client manager
	ClientManager* cman = new ClientManager();

	splash->showMessage("Starting XML-RPC Server");
	MonitorServer *XMLServer = new MonitorServer(7777);
	PServer = XMLServer;

	BackharddiNet_Monitor *BMonitor = new BackharddiNet_Monitor;
	PBMonitor = BMonitor;
	 
	BMonitor->loadLastTask();
	BMonitor->show();  
	QTest::qSleep(2000);

	splash->finish(BMonitor);
		
	return app.exec();
	
	Q_UNUSED(man);
	Q_UNUSED(gman);
	Q_UNUSED(cman);
}

