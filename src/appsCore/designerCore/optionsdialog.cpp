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

#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "designercore.h"
#include "mainwindow.h"
#include "optionsdialogstoragepage.h"
#include "optionsdialogrendererpage.h"
#include "optionsdialogprinterpage.h"
#include "optionsdialogdesigner.h"
#include "reportcore.h"

#include <QtGui>

OptionsDialog::OptionsDialog(CuteDesigner::Core *core) :
    QDialog(core->mainWindow()),
    ui(new Ui::OptionsDialog),
    m_core(core)
{
    ui->setupUi(this);

    setWindowTitle(tr("Options Dialog"));

    connect(ui->contentsWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    OptionsDialogPageInterface * page;

    page = new OptionsDialogDesigner(core);
    page->createButton(ui->contentsWidget);
    ui->pagesWidget->addWidget(page);

    page = new OptionsDialogStoragePage(core);
    page->createButton(ui->contentsWidget);
    ui->pagesWidget->addWidget(page);

    page = new OptionsDialogRendererPage(core);
    page->createButton(ui->contentsWidget);
    ui->pagesWidget->addWidget(page);

    page = new OptionsDialogPrinterPage(core);
    page->createButton(ui->contentsWidget);
    ui->pagesWidget->addWidget(page);

    ui->contentsWidget->setCurrentRow(0);

    QByteArray geometry = m_core->reportCore()->settings()->value("CuteReport_Designer/OptionsDialogGeometry").toByteArray();
    if (!geometry.isNull())
        restoreGeometry(geometry);
}


OptionsDialog::~OptionsDialog()
{
    OptionsDialogPageInterface * curPage = dynamic_cast<OptionsDialogPageInterface*>(ui->pagesWidget->currentWidget());
    if (curPage)
        curPage->deactivate();

    m_core->reportCore()->settings()->setValue("CuteReport_Designer/OptionsDialogGeometry", this->saveGeometry());

    delete ui;
}


void OptionsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    OptionsDialogPageInterface * prevPage = 0;
    if (previous && ( prevPage = dynamic_cast<OptionsDialogPageInterface*>(ui->pagesWidget->widget(ui->contentsWidget->row(previous))))) {
        prevPage->deactivate();
    }

    OptionsDialogPageInterface * curPage = 0;
    if (current && ( curPage = dynamic_cast<OptionsDialogPageInterface*>(ui->pagesWidget->widget(ui->contentsWidget->row(current))))) {
        curPage->activate();
    }

    ui->pagesWidget->setCurrentIndex(ui->contentsWidget->row(current));
}



//==================================================================================


OptionsDialogPageInterface::OptionsDialogPageInterface(CuteDesigner::Core *core)
{
    Q_UNUSED(core);
}

OptionsDialogPageInterface::~OptionsDialogPageInterface()
{

}


void OptionsDialogPageInterface::activate()
{

}


void OptionsDialogPageInterface::deactivate()
{

}
