/***************************************************************************
 *   This file is part of the CuteReport project                           *
 *   Copyright (C) 2012-2017 by Alexander Mikhalov                         *
 *   alexander.mikhalov@gmail.com                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/

#include "optionsdialogstoragepage.h"
#include "ui_optionsdialogstoragepage.h"
#include "designercore.h"
#include "reportcore.h"
#include "storageinterface.h"

#include <QMenu>

OptionsDialogStoragePage::OptionsDialogStoragePage(CuteDesigner::Core *core) :
    OptionsDialogPageInterface(core),
    ui(new Ui::OptionsDialogStoragePage),
    m_core(core)
{
    ui->setupUi(this);

    QMenu * menu = new QMenu(ui->bAddStorage);
    foreach (CuteReport::ReportPluginInterface* module, m_core->reportCore()->modules(CuteReport::StorageModule)) {
        QAction * action = new QAction(module->moduleFullName(), menu);
        action->setData(module->moduleFullName());
        connect(action, SIGNAL(triggered()), this, SLOT(setNewStorageModule()));
        menu->addAction(action);
    }
    ui->bAddStorage->setMenu(menu);

    updateStorageList();

    connect(ui->bDeleteStorage, SIGNAL(clicked()), this, SLOT(deleteCurrentStorage()));
    connect(ui->bDefaultStorage, SIGNAL(clicked()), this, SLOT(setDefaultStorage()));
    connect(ui->bResetDefaultStorage, SIGNAL(clicked()), this, SLOT(clearDefaultStorage()));
    connect(ui->storageList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(storagesListIndexChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}


OptionsDialogStoragePage::~OptionsDialogStoragePage()
{
    deactivate();
    delete ui;
}


void OptionsDialogStoragePage::activate()
{
}


void OptionsDialogStoragePage::deactivate()
{
//    if (m_currentStorage) {
//        m_currentStorage->helper()->save();
//        m_core->settings()->setValue(QString("CuteReport/Storage_%1_options").arg(m_currentStorage->moduleFullName().replace("::","_")),
//                                     m_core->reportCore()->moduleOptionsStr(m_currentStorage));
//    }
}


QListWidgetItem * OptionsDialogStoragePage::createButton(QListWidget * listWidget)
{
    QListWidgetItem *configButton = new QListWidgetItem(listWidget);
    configButton->setIcon(QIcon(":/images/storage_96x96.jpeg"));
    configButton->setText(tr("Storage"));
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    return configButton;
}


void OptionsDialogStoragePage::storagesListIndexChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    Q_UNUSED(previous)
    if (!current) {
        delete m_currentStorageHelper;
        return;
    }

    QString storageName = current->text(0);
    CuteReport::StorageInterface * storage = m_core->reportCore()->storage(storageName);

    if (!storage)
        return;

    if (m_currentStorageHelper)
        m_currentStorageHelper->save();
    delete m_currentStorageHelper;

    m_currentStorageHelper = storage->createHelper(m_core);
    if (m_currentStorageHelper)
        ui->storageHelperLayout->addWidget(m_currentStorageHelper);
    ui->storageType->setText(storage->moduleFullName());
}


void OptionsDialogStoragePage::setNewStorageModule()
{
    QAction * action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    QString moduleName = action->data().toString();
    CuteReport::StorageInterface * storage = m_core->reportCore()->createStorageObject(moduleName);
    if (!storage)
        return;
    storage->setObjectName(CuteReport::ReportCore::uniqueName(storage, storage->objectNameHint(), m_core->reportCore()));
    m_core->reportCore()->setStorage(storage);

    updateStorageList();
}


void OptionsDialogStoragePage::deleteCurrentStorage()
{
    if (!ui->storageList->currentItem())
        return;
    QString name = ui->storageList->currentItem()->text(0);
    m_core->reportCore()->deleteStorage(name);
    updateStorageList();
}


void OptionsDialogStoragePage::setDefaultStorage()
{
    if (!ui->storageList->currentItem())
        return;
    QString name = ui->storageList->currentItem()->text(0);
    m_core->reportCore()->setDefaultStorage(name);
    ui->leDefaultStorage->setText(m_core->reportCore()->defaultStorageName());
}


void OptionsDialogStoragePage::clearDefaultStorage()
{
    m_core->reportCore()->setDefaultStorage(QString());
    ui->leDefaultStorage->setText(m_core->reportCore()->defaultStorageName());
}


void OptionsDialogStoragePage::updateStorageList()
{
    ui->storageList->clear();
    QStringList nameList = m_core->reportCore()->storageNameList();
    qSort(nameList);
    foreach (const QString &name, nameList)
        ui->storageList->addTopLevelItem(new QTreeWidgetItem ( ui->storageList, QStringList() << name));

    ui->leDefaultStorage->setText(m_core->reportCore()->defaultStorageName());
}
