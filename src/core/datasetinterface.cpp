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
#include <QSortFilterProxyModel>
#include "datasetinterface.h"


namespace CuteReport
{


DatasetInterface::DatasetInterface(QObject *parent)
    : ReportPluginInterface(parent), m_filterColumn(-1), m_datasetFlags(0)
{
}


DatasetInterface *DatasetInterface::clone() const
{
    DatasetInterface * newDataset = this->objectClone();
    newDataset->setReportCore(this->reportCore());
    return newDataset;
}


QString	DatasetInterface::getParentDataset()
{
    return m_parentDataset;
}


void DatasetInterface::setParentDataset(QString pDataset)
{
    m_parentDataset = pDataset;
}


void DatasetInterface::setFilterCondition(QString list)
{
    m_filterCondition = list;
}


QString DatasetInterface::getFilterCondition()
{
    return m_filterCondition;
}


int DatasetInterface::getFilterColumn()
{
    return m_filterColumn;
}


void DatasetInterface::setFilterColumn(int col)
{
    m_filterColumn = col;
}


int DatasetInterface::datasetFlags() const
{
    return m_datasetFlags;
}


void DatasetInterface::setDatasetFlags(int flags)
{
    if (m_datasetFlags == flags)
        return;
    m_datasetFlags = flags;
    emit datasetFlagsChanged(m_datasetFlags);
}


void DatasetInterface::setDatasetFlag(int flag, bool value)
{
    int newFlags = m_datasetFlags;
    if (value)
        newFlags |= flag;
    else
        newFlags &= ~flag;
    if (newFlags != m_datasetFlags) {
        m_datasetFlags = newFlags;
        emit datasetFlagsChanged(m_datasetFlags);
    }
}


bool DatasetInterface::isDatasetFlagSet(int flag)
{
    return m_datasetFlags & flag;
}


//QVariant DatasetInterface::getValue(int /*index*/) {return QVariant();}
//QVariant DatasetInterface::getValue(const QString & /*field*/) {return QVariant();}

QVariant DatasetInterface::getNextRowValue(int /*index*/) {return QVariant();}
QVariant DatasetInterface::getNextRowValue(const QString & /*field*/) {return QVariant();}
QVariant DatasetInterface::getPreviousRowValue(int /*index*/) {return QVariant();}
QVariant DatasetInterface::getPreviousRowValue(const QString & /*field*/) {return QVariant();}
QAbstractItemModel * DatasetInterface::model() {return 0;}
QString DatasetInterface::getLastError() {return QString();}
QString DatasetInterface::getFieldName(int /*column*/ ) {return tr("Unknown");}


void DatasetInterface::setFilter ( const int col, const QString & str, Qt::CaseSensitivity cs)
{
    emit beforeSetFilter (col, str);
    QSortFilterProxyModel* _model = dynamic_cast<QSortFilterProxyModel*>( model() );
    if (_model ) {
        //	_model->setFilterRegExp (regExp);
        _model->setFilterFixedString(str);
        _model->setFilterCaseSensitivity(cs);
        _model->setFilterKeyColumn(col);
    } else
        qWarning("WARNING: please reimplement setFilter () in plugin \'%s\'", this->metaObject()->className());

    emit afterSetFilter (col, str);

}


} // namespace



