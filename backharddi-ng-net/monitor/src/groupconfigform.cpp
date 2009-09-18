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
#include "TaskManager.h"
#include "groupconfigform.h"
#include "groupmanager.h"
#include "clientmanager.h"
#include "cgroup.h"
#include "cclient.h"

#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QHash>

GroupConfigForm::GroupConfigForm(CGroup *group, bool editing, QWidget *parent) : QDialog(parent)
{
	m_group = group;
	this->editing = editing;
	setupUi(this);
	initForm();
	createConnections();
	
	if(editing)
		checkCurrentConfig();
	else
		group->setConfigured(false);
}

void GroupConfigForm::onComboBoxChanged(int index)
{
	if(index == 1) {
		directoryLabel->setText(tr("Specify an image to restore :"));	
		imageNameEdit->hide();
		label_2->hide();
		if(stackedWidget->currentIndex() == 1)
			stackedWidget->setCurrentIndex(0);
		
		nextButton->setText(tr("&Accept"));
	}
	else {
		directoryLabel->setText(tr("Specify a directory :"));
		imageNameEdit->show();
		label_2->show();
		nextButton->setText(tr("&Next"));
	}
}

void GroupConfigForm::onStackedChanged(int index)
{
	if(index == 1) {
		prevButton->show();
		prevButton->setEnabled(true);
		nextButton->setText(tr("&Accept"));
	}
	else {
		prevButton->hide();
		prevButton->setEnabled(false);
		nextButton->setText(tr("&Next"));
	}
}

void GroupConfigForm::createConnections()
{
	connect(nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));
	connect(prevButton, SIGNAL(clicked()), this, SLOT(onPrevButtonClicked()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(onClose()));
	connect(modeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxChanged(int)));
	connect(stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(onStackedChanged(int)));
	connect(toolButton, SIGNAL(clicked()), this, SLOT(onToolButtonClicked()));
}

void GroupConfigForm::initForm()
{
	// Settings the Next/Accept button
	prevButton->hide();
	nextButton->setText(tr("&Next"));
	nextButton->setEnabled(true);
}

void GroupConfigForm::onNextButtonClicked()
{
	if(stackedWidget->currentIndex() == 0) {
		if(modeComboBox->currentIndex() == 0) {
			if(m_group->getAllClientsFromMap().count() > 1) {
				for(int i = 0; i < m_group->getAllClientsFromMap().count(); ++i) {
					QListWidgetItem *copy = ClientManager::instance()->getClientItem(m_group->getClientFromMap(m_group->getAllClientsFromMap().keys().at(i)));
					QListWidgetItem *newItem = new QListWidgetItem(*copy);
					newItem->setHidden(false);
					listWidget->addItem(newItem);
				}
				
				stackedWidget->setCurrentIndex(1);
			}
			else
				processConfig();
		}
		else
			processConfig();
	}
	else	
		processConfig();
}

void GroupConfigForm::onPrevButtonClicked()
{	
	stackedWidget->setCurrentIndex(0);
}

void GroupConfigForm::checkCurrentConfig()
{
	if(m_group && m_group->isConfigured()) {
		if(m_group->getSettings().isEmpty()) {
			QString error;
			GroupManager::instance()->loadGroupConfig(static_cast<int>(m_group->getId()), error);
			
			if(!error.isEmpty()) {
#ifdef __MONITOR_DEBUG
				qDebug() << error;
#endif
				return;
			}
		}
		
		QStringList groupSettings = m_group->getSettings().split((","), QString::SkipEmptyParts);
		modeComboBox->setCurrentIndex(groupSettings.at(0).toInt());
		
		if(modeComboBox->currentIndex() == 0) { // Generate
			modeComboBox->setEnabled(false);
			
			QString directory, imageName;
			directory = groupSettings.at(1).left(groupSettings.at(1).lastIndexOf("+"));
			imageName = groupSettings.at(1).right((groupSettings.at(1).size() - groupSettings.at(1).lastIndexOf("+")));
 			directoryNameEdit->setText(directory.left(directory.size() - 1));
			imageNameEdit->setText(imageName.replace("+", ""));
		}
		else {
			directoryNameEdit->setText(groupSettings.at(1));
		}
		
		for(int i = 3; i < groupSettings.count(); ++i)
			listWidget->addItem(GroupManager::instance()->getGroupItem(GroupManager::instance()->findGroup(groupSettings.at(i).toInt())));
		
		if(modeComboBox->currentIndex() == 0) {
			nextButton->setText(tr("&Next"));
			nextButton->setEnabled(true);
			nextButton->show();
		}
		else {
			nextButton->setText(tr("&Accept"));
			nextButton->setEnabled(true);
			nextButton->show();
		}
	}
}

void GroupConfigForm::processConfig()
{
	if(imageNameEdit->text().isEmpty() && modeComboBox->currentIndex() == 0) {
		QMessageBox::critical(this, tr("Backharddi-Net Monitor"), tr("The new Image name can not be empty."), QMessageBox::Ok);
		return;
	}
		
	if(directoryNameEdit->text().isEmpty()) {
		QMessageBox::critical(this, tr("Backharddi-Net Monitor"), tr("The image directory can not be empty.."), QMessageBox::Ok);
		return;
	}
	
	QString image, error;
	if(modeComboBox->currentIndex() == 0) {
		image = directoryNameEdit->text() + "/+" + imageNameEdit->text();
		GroupManager::instance()->renameGroup(m_group, image.split("+").last().replace("/", ""), error);
		image = directoryNameEdit->text() + "/" + imageNameEdit->text();
	}
	else {
		image = directoryNameEdit->text();
		QString tmpStr = image.split("+").last().split("/").last();
		GroupManager::instance()->renameGroup(m_group, tmpStr, error);
	}
	
	if(listWidget->count() != 0) {
		m_group->m_generateIP = ClientManager::instance()->findClient(listWidget->currentItem()->text())->getInformation("ip_address");
		for(int i = 0; i < listWidget->count(); ++i) 
			delete listWidget->takeItem(i);
		
		QString clientsRemoved;
		for(int i = 0; i < m_group->getAllClientsFromMap().count(); ++i) {
			CClient *tmpClient = m_group->getClientFromMap(m_group->getAllClientsFromMap().keys().at(i));
			if(tmpClient) {
				if(tmpClient->getInformation("ip_address") != m_group->m_generateIP) {
					clientsRemoved += tmpClient->getFakeName() + " ";
					ClientManager::instance()->getClientItem(tmpClient)->setHidden(false);
					m_group->removeClientFromMap(tmpClient->getName());
				}
			}
		}
		
		if(!clientsRemoved.isEmpty()) {
			QString message = "Removing " + clientsRemoved + "from the group, they are not needed at all.\n\nIs strongly recomended delete this generation group before upload the master.";
			QMessageBox::information(this, "Backharddi-Net Monitor", tr(message), QMessageBox::Ok);
		}
	}	
	
	QString finalStr = QString::number(modeComboBox->currentIndex())+","+image+","+QString::number(m_group->getId());
	m_group->setSettings(finalStr);
	
	GroupManager::instance()->saveGroupConfig(m_group, error);
	
#ifdef __MONITOR_DEBUG
	if(!error.isEmpty()) 
		qDebug() << error;
#endif
		
	if(!m_group->isConfigured()) {
		TaskManager::instance()->currentTask()->addGroupToMap(m_group);
		m_group->setConfigured(true);
	}
	
	TaskManager::instance()->redrawTask();
	GroupManager::instance()->saveGroupClientsRelation(m_group);
	
	Close();
}

void GroupConfigForm::onToolButtonClicked()
{
	if(modeComboBox->currentIndex() == 1)
		fileDialog = new BFileDialog(this, "Backharddi-Net Monitor Image Browser", "/tmp/backharddi-ng");
	else
		fileDialog = new BFileDialog(this, "Backharddi-Net Monitor Image Browser", "/tmp/backharddi-ng", false);
	
	QString imagedir;
	
	if(fileDialog->exec()) {
		imagedir = fileDialog->selectedFiles().at(0);
	}
	
	if(!imagedir.isEmpty() && fileDialog->isOk()) {
		directoryNameEdit->setText(imagedir);
	}
	
	delete fileDialog;
	fileDialog = NULL;
}

void GroupConfigForm::Close()
{
	for(int i = 0; i < listWidget->count(); ++i) {
		QListWidgetItem *tmpItem = listWidget->item(i);
		listWidget->removeItemWidget(tmpItem);
		delete tmpItem;
	}
	close();
}

void GroupConfigForm::onClose()
{
	if(!editing) {
		for(int i = 0; i < m_group->getAllClientsFromMap().count(); ++i) 
			ClientManager::instance()->getClientItem(m_group->getClientFromMap(m_group->getAllClientsFromMap().keys().at(i)))->setHidden(false);

		QString error;
		if(!GroupManager::instance()->deleteGroup(m_group, error))
			QMessageBox::warning(this, "Backharddi-Net Monitor", error, QMessageBox::Ok);
	}
	Close();
}

void GroupConfigForm::onCurrentChanged(const QString &str )
{
	str.size();	
}

BFileDialog::BFileDialog(QWidget *parent, QString caption, QString dir, bool readOnly) : QFileDialog(parent, caption, dir, "")
{
	setReadOnly(readOnly);
	setViewMode(QFileDialog::List);
	setResolveSymlinks(false);
	setAcceptMode(QFileDialog::AcceptOpen);
	setFileMode(QFileDialog::DirectoryOnly);
	setFilter("Backharddi Image Containers");
	
	// Hook the QFileDialog layout
	QGridLayout *layout = (QGridLayout*)this->layout();
	
	// Remove QFileDialog Stuff
	QSplitter *split = static_cast<QSplitter*>(layout->itemAt(2)->widget());
	split->widget(0)->hide();
	QHBoxLayout *box = static_cast<QHBoxLayout*>(layout->itemAt(1)->layout());
	box->itemAt(4)->widget()->hide();
	box->itemAt(3)->widget()->hide();
	QComboBox *fileCombo = static_cast<QComboBox*>(box->itemAt(0)->widget());
	fileCombo->setEnabled(false);
	
	// Add our stuff
	toolButton = new QToolButton(0);
	toolButton->setAutoRaise(true);
	(readOnly) ? toolButton->setEnabled(false) : toolButton->setEnabled(true);
	toolButton->setIcon(QIcon(QString::fromUtf8(":/icons/load.png")));
	toolButton->setToolTip(tr("Create a new Image directory Container"));
	box->insertWidget(3, toolButton, 0, Qt::AlignHCenter);	
	
	// Hook QFileSystemModel (as possible T.T)
	QFrame *tmpFrame = static_cast<QFrame*>(split->widget(1));
	QStackedWidget *stackWidget =  static_cast<QStackedWidget*>(tmpFrame->layout()->itemAt(0)->widget());
	dirsList = static_cast<QListView *>(stackWidget->widget(0)->layout()->itemAt(0)->widget());
	
	connect(this, SIGNAL(directoryEntered( const QString& )), this, SLOT(onDirectoryEntered( const QString & )));
	connect(this, SIGNAL(currentChanged( const QString& )), this, SLOT(onCurrentChanged( const QString & )));
	connect(this, SIGNAL(filesSelected( const QStringList& )), this, SLOT(onFilesSelected( const QStringList & )));
	connect(toolButton, SIGNAL(clicked()), this, SLOT(onNewFolderTriggered()));
	connect(dirsList, SIGNAL(clicked( const QModelIndex& )), this, SLOT(onModelClicked(const QModelIndex& )));
}

BFileDialog::~BFileDialog()
{
}

void BFileDialog::checkDirectory(QString directory)
{
	// NOTE: Needed on near future?
	directory.size();
}

void BFileDialog::onDirectoryEntered( const QString &directory )
{	
	// NOTE: Needed on near future?
	directory.size();
}

void BFileDialog::onCurrentChanged( const QString &changed )
{	
	if(!changed.contains("/tmp/backharddi-ng") && !changed.isEmpty()) {
		QMessageBox::critical(this, "Backharddi-Net Monitor", tr("You can only explore folders under /tmp/backharddi-ng"), QMessageBox::Ok);
		setDirectory("/tmp/backharddi-ng");
		return;
	}
}

void BFileDialog::onFilesSelected( const QStringList &files )
{
	if(files.size() > 1) {
		QMessageBox::critical(this, "Backharddi-Net Monitor", tr("You can only select one image directory"), QMessageBox::Ok);
		setDirectory("/tmp/backharddi-ng");
		return;
	}
	
	bool found = false;
	found = (isDirImage(files.at(0), m_devicePath)) ? true : false;	
	
	if(isReadOnly()) {
		if(!found) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("The directory you selected does not seems to be a valid Backharddi image container."), QMessageBox::Ok);
			m_isOk = false;
			return;
		}
	}
	else {
		if(found) {
			QMessageBox::critical(this, "Backharddi-Net Monitor", tr("The directory you selected already contains a Backharddi image!!!."), QMessageBox::Ok);
			m_isOk = false;
			return;
		}
	}
	
	m_isOk = true;
}

void BFileDialog::onNewFolderTriggered()
{	
	// Create folder name related stuff
	bool ok;
	QString newFolderString = QFileDialog::tr("New Image Container");
	QString folderName = QInputDialog::getText(this, "Backharddi-Net Monitor", tr("Input the name of new folder that will contains the Image"), QLineEdit::Normal, newFolderString, &ok);
	if(!ok || folderName.isEmpty())
		return;
	
	QString prefix = directory().absolutePath() + QDir::separator() + "+";
	
	// Existance checks
	if(QFile::exists(prefix + folderName)) {
		QMessageBox::critical(this, "Backharddi-Net Monitor", tr("A directory with the same name exists already.\nAborting."), QMessageBox::Ok);
		return;
	}
	
	// Parse the folder Name
	QRegExp reg("[^a-zA-Z0-9ñÑçÇáéíóúàèìòù\\+\\.,:;-]");
	folderName.replace(reg, "_");
		
	QDir dir;
	dir.mkdir(QString("%1%2").arg(prefix, folderName));
	
	setDirectory(folderName);
}

void BFileDialog::onModelClicked(const QModelIndex &model )
{
	bool valid = false;
	if(!model.data(Qt::DisplayRole).toString().contains("+")) {
		QString tmpDir = directory().absolutePath() + "/" + model.data(Qt::DisplayRole).toString();
		QDir *dir = new QDir(tmpDir);
		
		foreach(QString tmpDir, dir->entryList()) {
			
			if(tmpDir.contains("=dev=")) {
				valid = true;
				break;
			}
		}
		if(!valid) {
			QMessageBox::warning(this, "Backharddi-Net Monitor", tr("Valid Backharddi image containers are prefixed with '+' character."), QMessageBox::Ok);
			delete dir;
			return;
		}

		delete dir;
	}	
}

bool BFileDialog::isOk() const
{
	return m_isOk;
}

bool BFileDialog::isDirImage(const QString &path, QString &devicePath) const
{
	QDir dir(path);
	foreach(QString entry, dir.entryList()) {
		if(entry.contains("=dev=")) {
			devicePath = entry;
			return true;
		}
	}
	
	return false;
}

