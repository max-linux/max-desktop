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
#include "Loader.h"

#include <QSettings>
#include <QDir>

Loader::Loader() 
{
     m_Settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "Open Phoenix IT", "Backharddi NG Monitor");     
  
     if(!( m_Settings->value("managing/firstBoot") ).toBool()) {
          // First Application run           
          m_Settings->setValue("MainWindow/fullScreen", false);
          m_Settings->setValue("SQL/driver", "QSQLITE");
          m_Settings->setValue("SQL/hostName", "localhost");
          m_Settings->setValue("SQL/userName", "");
          m_Settings->setValue("SQL/password", "");
          m_Settings->setValue("SQL/database", "backharddi-ng"); 
          m_Settings->setValue("SQL/port", 0);
          m_Settings->setValue("SQL/SQLiteCache", false);
          m_Settings->setValue("SQL/SQLiteMemory", false);
          
          QString homePath = QDir::homePath().append("/.backharddi-ng");
          m_Settings->setValue("SQL/SQLiteDbPath", homePath);
          
          m_Settings->setValue("SQL/SQLiteCryptDb", false);
          m_Settings->setValue("SQL/MySQLStrict", false);
          m_Settings->setValue("SQL/MySQLUtf8", true);
              
          m_Settings->setValue("LastTask/taskId", 0);
          
          m_Settings->setValue("managing/firstBoot", true);
          
          firstBoot = true;
     }
     else {
          firstBoot = false;
     }
}

Loader::~Loader()
{
     delete m_Settings;
}

void Loader::restore() 
{
     // Restore default values
     m_Settings->setValue("MainWindow/fullScreen", false);
     m_Settings->setValue("SQL/driver", "QSQLITE");
     m_Settings->setValue("SQL/hostName", "localhost");
     m_Settings->setValue("SQL/userName", "");
     m_Settings->setValue("SQL/password", "");
     m_Settings->setValue("SQL/database", "backharddi-ng");  
     m_Settings->setValue("SQL/port", 0);
     m_Settings->setValue("LastTask/taskId", 0);
}

void Loader::setKey(QString key, QVariant value)
{
     if(key.isEmpty())
          return;
     
     m_Settings->setValue(key, value);
}

QVariant Loader::getKey(QString key)
{         
     if(!m_Settings->contains(key)) {
          qDebug() << key << " is not at Application Settings.";
          return 0;
     }    
     
     return m_Settings->value(key);
}

bool Loader::isFirstBoot() const 
{
     return firstBoot;
}



