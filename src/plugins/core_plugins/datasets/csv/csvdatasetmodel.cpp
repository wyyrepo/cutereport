/***************************************************************************
 *   This file is part of the CuteReport project                           *
 *   Copyright (C) 2012-2016 by Mikhalov Alexander                         *
 *   alexander.mikhalov@gmail.com                                          *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#include "csvdatasetmodel.h"
#include <QStringList>

Model::Model(QObject *parent)
    : QAbstractTableModel(parent), m_rows(0), m_columns(0)
{
}


Model::~Model()
{
}


void Model::setArray(Array array)
{
    m_array = array;
    m_rows = m_array.count();
    foreach (QStringList record, m_array)
        if (record.count() > m_columns)
            m_columns = record.count();

    beginResetModel();
    endResetModel();
}


void Model::setHeader(QStringList header)
{
    m_fields.clear();
    for (int i = 0; i< header.count(); ++i) {
        m_fields.insert(header.at(i), i);
        m_header.insert(i, header.at(i));
    }
}


void Model::clear()
{
    beginResetModel();

    m_array.clear();
    m_rows = 0;
    m_columns = 0;

    endResetModel();
}


int Model::fieldIndex(const QString &fieldName)
{
    int index = m_fields.value(fieldName, -1);
    if (index == -1) {
        bool ok = false;
        int i = fieldName.toInt(&ok);
        if (ok && !headerData(i, Qt::Horizontal).isNull())
            return i;
    }
    return m_fields.value(fieldName, -1);
}


int Model::columnCount(const QModelIndex &/*parent*/) const
{
    return m_columns;
}


int Model::rowCount(const QModelIndex &/*parent*/) const
{
    return m_rows;
}


QVariant Model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() < m_columns) {
        QStringList line = m_array[index.row()];
        if (index.column() < line.size()) {
            QString value = m_array[index.row()][index.column()];
            // try to detect numbers for further correct sorting
            bool ok;
            qreal numValue = value.toDouble(&ok);
            if (ok)
                return numValue;
            else
                return value;
            //return m_array[index.row()][index.column()];
        }
        return QVariant();
    }
    else
        return QVariant();
}


Qt::ItemFlags Model::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (m_header.contains(section))
            return m_header.value(section);
        else
            return QString::number(section);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

