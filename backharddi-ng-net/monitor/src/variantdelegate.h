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

#ifndef variantdelegate_h
#define variantdelegate_h

#include <QItemDelegate>
#include <QRegExp>

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
 */
class VariantDelegate : public QItemDelegate
{
Q_OBJECT

public:
	VariantDelegate(QObject *parent = 0);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

	static bool isSupportedType(QVariant::Type type);
	static QString displayText(const QVariant &value);

private:
	mutable QRegExp boolExp;
	mutable QRegExp byteArrayExp;
	mutable QRegExp charExp;
	mutable QRegExp colorExp;
	mutable QRegExp dateExp;
	mutable QRegExp dateTimeExp;
	mutable QRegExp doubleExp;
	mutable QRegExp pointExp;
	mutable QRegExp rectExp;
	mutable QRegExp signedIntegerExp;
	mutable QRegExp sizeExp;
	mutable QRegExp timeExp;
	mutable QRegExp unsignedIntegerExp;
};

#endif
