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

#ifndef _Common_h_
#define _Common_h_

#include <QHash>
#include <QVector>
#include <QList>
#include <QDebug>

//==========================================================================
// Constants
//==========================================================================
const int  MIN_CLIENT_NAME_LENGTH   = 5;
const int  MAX_CLIENT_NAME_LENGTH   = 48;
const int  MONITOR_VERSION          = 0x0001;

//==========================================================================
// Error Codes to return from methods
//==========================================================================
enum ERROR_CODE {
     ERR_NO_ERROR,
     ERR_NAME_EMPTY,
     ERR_NAME_TOO_SHORT,
     ERR_NAME_TOO_LONG,
     ERR_CLIENT_NULL,
     ERR_CLIENT_UNKNOWN,
     ERR_CLIENT_ALREADY_ON_MAP,
     ERR_PROTOCOL_MISMATCH,
     MAX_ERROR_CODE
};

#endif    // _Common_h_
