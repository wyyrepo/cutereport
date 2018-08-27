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

#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <QDialog>
#include "designer_globals.h"


namespace Ui {
class RenameDialog;
}


namespace CuteReport {
class ReportCore;
class ReportInterface;
}


class DESIGNER_EXPORTS RenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenameDialog(QObject * object, CuteReport::ReportInterface * report, QWidget *parent = 0);
    ~RenameDialog();

    QString newName();

private slots:
    void textEdited(QString text);

private:
    Ui::RenameDialog *ui;
    QObject * m_object;
    CuteReport::ReportInterface * m_report;
};

#endif // RENAMEDIALOG_H
