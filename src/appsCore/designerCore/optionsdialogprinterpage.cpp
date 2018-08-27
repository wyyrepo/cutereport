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

#include "optionsdialogprinterpage.h"
#include "ui_optionsdialogprinterpage.h"
#include "printerinterface.h"
#include "reportcore.h"
#include "designercore.h"
#include "propertyeditor.h"

#include <QMenu>

OptionsDialogPrinterPage::OptionsDialogPrinterPage(CuteDesigner::Core *core) :
    OptionsDialogPageInterface(core),
    ui(new Ui::OptionsDialogPrinterPage)
  , m_core(core)
{
    ui->setupUi(this);

    QMenu * menu = new QMenu(ui->bAdd);
    foreach (CuteReport::ReportPluginInterface* module, m_core->reportCore()->modules(CuteReport::PrinterModule)) {
        QAction * action = new QAction(module->moduleFullName(), menu);
        action->setData(module->moduleFullName());
        connect(action, SIGNAL(triggered()), this, SLOT(setNewObject()));
        menu->addAction(action);
    }
    ui->bAdd->setMenu(menu);

    updateObjectList();

    connect(ui->bDelete, SIGNAL(clicked()), this, SLOT(deleteCurrent()));
    connect(ui->bDefault, SIGNAL(clicked()), this, SLOT(setDefault()));
    connect(ui->bResetDefault, SIGNAL(clicked()), this, SLOT(clearDefault()));
    connect(ui->objectList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(listIndexChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}


OptionsDialogPrinterPage::~OptionsDialogPrinterPage()
{
    delete ui;
}


void OptionsDialogPrinterPage::activate()
{
}


void OptionsDialogPrinterPage::deactivate()
{
//    if (m_currentModule) {
//        m_core->settings()->setValue("CuteReport/PrimaryPrinter", m_currentModule->moduleFullName());
////        m_currentModule->helper()->save();
//        m_core->settings()->setValue(QString("CuteReport/Printer_%1_options").arg(m_currentModule->moduleFullName()), m_core->reportCore()->moduleOptionsStr(m_currentModule)) ;
//    }
}


QListWidgetItem * OptionsDialogPrinterPage::createButton(QListWidget * listWidget)
{
    QListWidgetItem *configButton = new QListWidgetItem(listWidget);
    configButton->setIcon(QIcon(":/images/printer_96x96.png"));
    configButton->setText(tr("Printer"));
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    return configButton;
}


void OptionsDialogPrinterPage::listIndexChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    Q_UNUSED(previous)
    if (!current) {
        delete m_currentHelper;
        return;
    }

    if (previous) {
        QString objectName = current->text(0);
        CuteReport::PrinterInterface * object = m_core->reportCore()->printer(objectName);
        if (object)
            disconnect(object, 0, this, 0);
    }

    QString objectName = current->text(0);
    CuteReport::PrinterInterface * object = m_core->reportCore()->printer(objectName);

    if (!object)
        return;

    if (m_currentHelper) {
        CuteReport::PrinterHelperInterface * helper = qobject_cast<CuteReport::PrinterHelperInterface*>(m_currentHelper.data());
        if (helper)
            helper->save();
    }
    delete m_currentHelper;

    m_currentHelper = object->helper();
    if (!m_currentHelper) {
        PropertyEditor::EditorWidget * editor = m_core->createPropertyEditor(this);
        editor->setObject(object);
        m_currentHelper = editor;
    }
    if (m_currentHelper)
        ui->helperLayout->addWidget(m_currentHelper);
    ui->objectType->setText(object->moduleFullName());

    connect(object, SIGNAL(objectNameChanged(QString)), this, SLOT(slotObjectNameChanged(QString)));
}


void OptionsDialogPrinterPage::setNewObject()
{
    QAction * action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    QString moduleName = action->data().toString();
    CuteReport::PrinterInterface * object = m_core->reportCore()->createPrinterObject(moduleName);
    if (!object)
        return;
    object->setObjectName(CuteReport::ReportCore::uniqueName(object, object->objectNameHint(), m_core->reportCore()));
    m_core->reportCore()->setPrinter(object);

    updateObjectList();
}


void OptionsDialogPrinterPage::deleteCurrent()
{
    if (!ui->objectList->currentItem())
        return;
    QString name = ui->objectList->currentItem()->text(0);
    m_core->reportCore()->deletePrinter(name);
    updateObjectList();
}


void OptionsDialogPrinterPage::setDefault()
{
    if (!ui->objectList->currentItem())
        return;
    QString name = ui->objectList->currentItem()->text(0);
    m_core->reportCore()->setDefaultPrinter(name);
    ui->leDefault->setText(m_core->reportCore()->defaultPrinterName());
}


void OptionsDialogPrinterPage::clearDefault()
{
    m_core->reportCore()->setDefaultPrinter(QString());
    ui->leDefault->setText(m_core->reportCore()->defaultPrinterName());
}


void OptionsDialogPrinterPage::updateObjectList()
{
    ui->objectList->clear();
    QStringList nameList = m_core->reportCore()->printerNameList();
    qSort(nameList);
    foreach (const QString &name, nameList)
        ui->objectList->addTopLevelItem(new QTreeWidgetItem ( ui->objectList, QStringList() << name));

    ui->leDefault->setText(m_core->reportCore()->defaultPrinterName());
}


void OptionsDialogPrinterPage::slotObjectNameChanged(const QString &name)
{
    QTreeWidgetItem* current = ui->objectList->currentItem();
    if (!current)
        return;

    current->setText(0, name);
}
