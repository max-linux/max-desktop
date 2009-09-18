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
#include "preferencesdialog.h"
#include "Loader.h"
#include "Database.h"

#include <QtGui>
#include <QMessageBox>

PreferencesSQLDialog::PreferencesSQLDialog(QWidget *parent) : QDialog(parent)
{
     setupUi(this);
     CreateConnections();
     checkCurrentConfig();
	username->setText("backharddi");
}

void PreferencesSQLDialog::onComboBoxChanged(QString index)
{
     if(index.contains("mysql", Qt::CaseInsensitive)) {
          stackedWidget->setCurrentIndex(1);
          driver = "QMYSQL";
     }
     else {
          stackedWidget->setCurrentIndex(0);
          driver = "QSQLITE";
     }
}

void PreferencesSQLDialog::CreateConnections()
{
     connect(okButton, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));  
     connect(cancelButton, SIGNAL(clicked()), this, SLOT(Close()));
     connect(comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onComboBoxChanged(QString)));    
}

void PreferencesSQLDialog::onOkButtonClicked()
{
     processPreferences();
}

void PreferencesSQLDialog::checkCurrentConfig()
{
     extern Loader *settings;
     if(settings->getKey("SQL/driver") == "QSQLITE") {
          stackedWidget->setCurrentIndex(0); 
          comboBox->setCurrentIndex(0);         
     }
     else {
          stackedWidget->setCurrentIndex(1);
          comboBox->setCurrentIndex(1);         
     }          
     
     // MySQL Properties
     hostname->setText(settings->getKey("SQL/hostName").toString());
     username->setText(settings->getKey("SQL/userName").toString());
     password->setText(settings->getKey("SQL/password").toString());
     strictModeCheck->setCheckState((settings->getKey("SQL/MySQLStrict").toBool()) ? Qt::Checked : Qt::Unchecked);
     utf8Check->setCheckState((settings->getKey("SQL/MySQLUtf8").toBool()) ? Qt::Checked : Qt::Unchecked);
}

void PreferencesSQLDialog::processPreferences()
{
     extern Loader *settings;
     if(stackedWidget->currentIndex() == 0) 
          settings->setKey("SQL/driver", "QSQLITE");          
     else
          settings->setKey("SQL/driver", "QMYSQL");     
     
     // MySQL Properties
     settings->setKey("SQL/hostName", hostname->text());
     settings->setKey("SQL/userName", username->text());
     settings->setKey("SQL/password", password->text());
     settings->setKey("SQL/MySQLStrict", (strictModeCheck->checkState() == Qt::Checked) ? true : false);
     settings->setKey("SQL/MySQLUtf8", (utf8Check->checkState() == Qt::Checked) ? true : false);
     
     qDebug() << "Settings save successful...";
     
     // Reload the Database Configuration, clean session and start a new one
     extern CDatabase *BDatabase;
     if(BDatabase->getDatabase().driverName().contains("MYSQL") && stackedWidget->currentIndex() == 1) {
          BDatabase->getDatabase().close();
          BDatabase->update();
          BDatabase->getDatabase().open();          
     } 
     else if(BDatabase->getDatabase().driverName().contains("SQLITE") && stackedWidget->currentIndex() == 0) {               
          BDatabase->update();
     }
     else {
          BDatabase->StopDatabase();
          BDatabase->StartDatabase();
     }
     
     if(BDatabase->hasErrors()) {
       if(!BDatabase->lastError().text().contains("Unknown database"))
               QMessageBox::critical(0, QObject::tr("Database Error"), BDatabase->lastError().text());
       else
               QMessageBox::critical(0, QObject::tr("Database Error"), BDatabase->lastError().text().append("\n\nYour backharddi installation seems to be corrupted, your backharddi MySQL Database is gone, please reconfigure the Backharddi-Net package in order to fix this problem.\n\nExecute: sudo dpkg-reconfigure backharddi-net"));              
       
       comboBox->setCurrentIndex(0);         
       return;
     }
     
     Close();
   
     // Create the database tables if there is no registers at task table
     if(BDatabase->rowsCount("tasks", "idtask") == 0) 
          BDatabase->CreateTables();     
     
     if(settings->getKey("SQL/MySQLStrict").toBool()) {
          qDebug() << "Forcing Strict transition tables if possible.";
          BDatabase->forceTransStrict();
     }
}

void PreferencesSQLDialog::Close()
{     
     close();
}
