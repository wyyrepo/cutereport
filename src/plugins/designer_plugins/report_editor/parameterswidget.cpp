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
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/

#include "parameterswidget.h"
#include "ui_parameterswidget.h"
#include "reportinterface.h"
#include "comboboxdelegate.h"
#include "parametersmodel.h"

#include <QStandardItemModel>
#include <QDateTime>
#include <QDebug>
#include <QVariant>

ParametersWidget::ParametersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParametersWidget)
{
    ui->setupUi(this);

    m_variablesModel = new ParametersModel(0, 3, this);
    m_variablesModel->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Type") << tr("Value"));
    ui->variables->setModel(m_variablesModel);
    ui->variables->horizontalHeader()->setStretchLastSection(true);

    ComboBoxDelegate::ListPairIntStr list;
    list << ComboBoxDelegate::PairIntStr(ParametersModel::NotDefined, tr("Not Defined"));
    list << ComboBoxDelegate::PairIntStr(ParametersModel::StringType, tr("String"));
    list << ComboBoxDelegate::PairIntStr(ParametersModel::BoolType, tr("Boolean"));
    list << ComboBoxDelegate::PairIntStr(ParametersModel::IntType, tr("Integer"));
    list << ComboBoxDelegate::PairIntStr(ParametersModel::DoubleType, tr("Double"));
    list << ComboBoxDelegate::PairIntStr(ParametersModel::DateType, tr("Date"));
    list << ComboBoxDelegate::PairIntStr(ParametersModel::TimeType, tr("Time"));
    list << ComboBoxDelegate::PairIntStr(ParametersModel::DatetimeType, tr("DateTime"));
    list << ComboBoxDelegate::PairIntStr(ParametersModel::ObjectType, tr("Object"));
    //    list << ComboBoxDelegate::PairIntStr(PixmapType, tr("String"));


    ui->variables->setItemDelegateForColumn(1, new ComboBoxDelegate(list, ui->variables));

    connect(m_variablesModel, SIGNAL(variableChanged(QString,QVariant)), this, SLOT(slotModelVariableChanged(QString,QVariant)));
}

ParametersWidget::~ParametersWidget()
{
    delete ui;
}


void ParametersWidget::setReport(CuteReport::ReportInterface *report)
{
    m_report = report;
    updateView();
    connect(m_report, SIGNAL(variablesChanged()), this, SLOT(updateView()), Qt::UniqueConnection);
}


void ParametersWidget::slotModelVariableChanged(const QString &variableName, const QVariant &variableValue)
{
    disconnect(m_report, SIGNAL(variablesChanged()), this, 0);
    m_report->setVariableValue(variableName, variableValue);
    connect(m_report, SIGNAL(variablesChanged()), this, SLOT(updateView()), Qt::UniqueConnection);
}



void ParametersWidget::updateView()
{
    m_variablesModel->setVariables(m_report->variables());
}

