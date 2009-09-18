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
#ifndef groupconfigform_h
#define groupconfigform_h

#include <QDialog>
#include <QHash>
#include <ui_configureGroup.h>

class CGroup;
class QString;
class stackedWidget;
class BFileDialog;

/**
	@author Oscar Campos Ruiz-Adame <oscar.campos@edmanufacturer.es>
*/
class GroupConfigForm : public QDialog, public Ui::configureGroupWidget
{
	Q_OBJECT
public:
	enum configChoices {
		config_mode, config_images
	};
	
	GroupConfigForm(CGroup *group, bool editing = false, QWidget *parent = 0);
	
private slots:
	void onComboBoxChanged(int index);
	void onStackedChanged(int index);
	void onNextButtonClicked();
	void onPrevButtonClicked();
	void onToolButtonClicked();
	void Close();
	void onClose();
	void onCurrentChanged(const QString &str );
	
private:
	void createConnections();
	void checkCurrentConfig();
	void processConfig();
	void initForm();
	
private:
	CGroup *m_group;
	BFileDialog *fileDialog;
	bool editing;
};

class BFileDialog : public QFileDialog
{
Q_OBJECT
public:
	BFileDialog(QWidget *parent = 0, QString caption="Backharddi File Dialog", QString dir="/tmp/backharddi-ng", bool readOnly = true);
	virtual ~BFileDialog();
	
	bool isOk() const;
	
private:
	bool isDirImage(const QString &path, QString &devicePath) const;
	
private slots:
	void checkDirectory(QString directory);
	void onDirectoryEntered( const QString &directory );
	void onCurrentChanged( const QString &changed );
	void onFilesSelected( const QStringList &files );
	void onNewFolderTriggered();
	void onModelClicked(const QModelIndex &model );
	
signals:
	void signalDirUp(const QString &directory);
	void signalCurrentChanged(const QString &path);
	void signalDirectoryEntered(const QString &directory);
	
private:
	QToolButton 	*toolButton;
	QListView 	*dirsList;
	QString 	m_devicePath;
	bool 		m_isOk;
	QHash<QString, QString>		m_information;
};


#endif
