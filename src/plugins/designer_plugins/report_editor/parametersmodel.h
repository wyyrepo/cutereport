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

#ifndef PARAMETERSMODEL_H
#define PARAMETERSMODEL_H

#include <QStandardItemModel>

class ParametersModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum VarType {NotDefined, StringType, BoolType, IntType, DoubleType, DateType, TimeType, DatetimeType, ObjectType, PixmapType};
    ParametersModel(QObject * parent = 0);
    ParametersModel(int rows, int columns, QObject *parent = 0);

    void setVariables(const QVariantHash &variables);

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

signals:
    void variableChanged(QString name, QVariant value);

private:
    static QVariant convertToType(const QVariant & value, VarType vType);
    static VarType variableType(const QVariant & value);
};

#endif // PARAMETERSMODEL_H
