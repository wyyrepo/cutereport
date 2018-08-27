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

#ifndef OPTIONSDIALOGPRINTERPAGE_H
#define OPTIONSDIALOGPRINTERPAGE_H

#include <optionsdialog.h>

#include <QPointer>

namespace Ui {
class OptionsDialogPrinterPage;
}

namespace CuteDesigner {
class Core;
}

namespace CuteReport {
class PrinterHelperInterface;
class PrinterInterface;
}

class QTreeWidgetItem;

class OptionsDialogPrinterPage :  public OptionsDialogPageInterface
{
    Q_OBJECT

public:
    explicit OptionsDialogPrinterPage(CuteDesigner::Core *core);
    ~OptionsDialogPrinterPage();

    virtual void activate();
    virtual void deactivate();

    virtual QListWidgetItem * createButton(QListWidget * listWidget);

private slots:
    void listIndexChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void setNewObject();
    void deleteCurrent();
    void setDefault();
    void clearDefault();
    void updateObjectList();
    void slotObjectNameChanged(const QString & name);

private:
    Ui::OptionsDialogPrinterPage *ui;
    CuteDesigner::Core * m_core;
    QPointer<QWidget> m_currentHelper;
};

#endif // OPTIONSDIALOGPRINTERPAGE_H
