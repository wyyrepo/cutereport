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

#include "renamedialog.h"
#include "reportinterface.h"
#include "reportcore.h"
#include "ui_renamedialog.h"

#include <QDialogButtonBox>
#include <QPushButton>


using namespace CuteReport;

RenameDialog::RenameDialog(QObject * object, CuteReport::ReportInterface * report, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameDialog),
    m_object(object),
    m_report(report)
{
    ui->setupUi(this);
    ui->validationText->clear();
    ui->name->setText(object->objectName());
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));
}


RenameDialog::~RenameDialog()
{
    delete ui;
}

QString RenameDialog::newName()
{
    return ui->name->text();
}


void RenameDialog::textEdited(QString text)
{
    if (text.isEmpty()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->validationText->setText("");
        return;
    }

    bool unique = ReportCore::isNameUnique(m_object, text, m_report);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(unique);
    ui->validationText->setText(unique ? "Ok" : "Object with this name already exists!");
}
