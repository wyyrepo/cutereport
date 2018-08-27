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

#include "parametersmodel.h"
#include "cutereport_types.h"

#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QDebug>

ParametersModel::ParametersModel(QObject *parent)
    :QStandardItemModel(parent)
{

}

ParametersModel::ParametersModel(int rows, int columns, QObject *parent)
    :QStandardItemModel(rows, columns, parent)
{

}

void ParametersModel::setVariables(const QVariantHash & variables)
{
    removeRows(0, rowCount());

    int row = -1;
    foreach (const QString & key, variables.keys()) {

        QVariant value = variables.value(key);
//        qDebug() << value.type() << (int)value.type();
        VarType varType = variableType(value);

        ++row;
        QStandardItem *itemName = new QStandardItem(key);
        itemName->setFlags(Qt::ItemIsEnabled);
        setItem(row, 0, itemName);

        QStandardItem *itemType = new QStandardItem();
//        qDebug() << variableType(variables.value(key));
        itemType->setData(variableType(variables.value(key)), Qt::EditRole);
        itemType->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        setItem(row, 1, itemType);

        QStandardItem *itemValue = new QStandardItem(/*m_report->variables().value(key).toString()*/);
        itemValue->setData(variables.value(key), Qt::EditRole);
        if (varType != NotDefined)
            itemValue->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        else
            itemValue->setFlags(Qt::NoItemFlags);
        setItem(row, 2, itemValue);
    }
}


bool ParametersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        int row = index.row();
        int column = index.column();
        if (column == 1) {
//            QVariant oldVariantType = data(index, role);
//            VarType oldType = (VarType)oldVariantType.toInt();
            VarType newType = (VarType)value.toInt();

            QModelIndex valueIndex = this->index(row, 2);
            QVariant oldVariantValue = data(valueIndex);
            QVariant newVarValue = convertToType(oldVariantValue, newType);

            bool result = QStandardItemModel::setData(index, value, role);
//            qDebug() << data(index);
            QStandardItemModel::setData(valueIndex, newVarValue, Qt::EditRole);
//            qDebug() << data(index);

            QStandardItem *itemValue = itemFromIndex (valueIndex);
            if (itemValue) {
                if (newType == ObjectType) {
                    itemValue->setText("Not Editable");
                    itemValue->setFlags(Qt::NoItemFlags);
                } else if (newType != NotDefined)
                    itemValue->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
                else
                    itemValue->setFlags(Qt::NoItemFlags);
            }

            QString varName = this->data(this->index(row, 0)).toString();
            emit variableChanged(varName, newVarValue);

            return result;

        } else if (column == 2) {
            VarType vType = (VarType)this->data(this->index(row, 1)).toInt();
            QVariant newValue = convertToType(value, vType);
            bool result = QStandardItemModel::setData(index, newValue, role);
            QString varName = this->data(this->index(row, 0)).toString();
            emit variableChanged(varName, newValue);
            return result;
        }
    }

    return QStandardItemModel::setData(index, value, role);
}


QVariant ParametersModel::convertToType(const QVariant &value, VarType vType)
{
    QVariant newValue;
    switch (vType) {
        case NotDefined: break;
        case StringType: newValue = value.toString(); break;
        case BoolType: newValue = value.toBool(); break;
        case IntType: newValue = value.toInt(); break;
        case DoubleType: newValue = value.toDouble(); break;
        case DateType: newValue = value.toDate(); break;
        case TimeType: newValue = value.toTime(); break;
        case DatetimeType: newValue = value.toDateTime(); break;
        case PixmapType: newValue = value; break;
        case ObjectType: newValue = QVariant::fromValue<QObject*>(0);  break; // construct NULL Qbject since it cannot be converted from other types
    }
    return newValue;
}


ParametersModel::VarType ParametersModel::variableType(const QVariant &value)
{
    switch (value.type()) {
        case QVariant::String:
            return StringType;
        case QVariant::Bool:
            return BoolType;
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            return IntType;
        case QVariant::Double:
            return DoubleType;
        case QVariant::Date:
            return DateType;
        case QVariant::Time:
            return TimeType;
        case QVariant::DateTime:
            return DatetimeType;
        default: {
                if (value.canConvert<QObject*>())
                    return ObjectType;
            return NotDefined;
            }
    }

    return NotDefined;
}
