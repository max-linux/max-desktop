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
#ifndef generaltab_h
#define generaltab_h

#include <QWidget>

class TasksTree;
class CTask;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class GeneralTab : public QWidget
{
Q_OBJECT
public:
     GeneralTab(CTask *task, QWidget *parent = 0);
     ~GeneralTab();
     TasksTree *getTree();
	
private:
	TasksTree *treeWidget;	
};

#endif
