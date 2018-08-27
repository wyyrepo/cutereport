/***************************************************************************
 *   This file is part of the CuteReport project                           *
 *   Copyright (C) 2013 by Alexander Mikhalov                              *
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
 ***************************************************************************/
#include "filesystemdataset.h"
#include "filesystemdatasethelper.h"
#include "filesystemdatasetmodel.h"
#include "reportcore.h"
#include "cutereport_functions.h"
#include "scriptengineinterface.h"
#include "rendererpublicinterface.h"

#include "QtCore"
#include <QIcon>

#define DATASET_NAME "FileSystem"

using namespace CuteReport;

inline void initMyResource() { Q_INIT_RESOURCE(filesystemdataset); }

FileSystemDataset::FileSystemDataset(QObject *parent)
    : DatasetInterface(parent),
      m_filters(Dirs|AllDirs|Files|Readable|Writable|Executable|Hidden|System),
      m_currentRow(0),
      m_recursionLevel(0),
      m_maxNumber(-1),
      m_state(Unpopulated),
      m_pathAppearance(NoPath),
      m_renderer(0)
{
    m_model = new FSModel (this);
    m_fmodel = new QSortFilterProxyModel(this);
    m_fmodel->setSourceModel(m_model);
}


FileSystemDataset::FileSystemDataset(const FileSystemDataset &dd, QObject * parent)
    : DatasetInterface(parent),
      m_filters(dd.m_filters),
      m_currentRow(0),
      m_recursionLevel(dd.m_recursionLevel),
      m_maxNumber(dd.m_maxNumber),
      m_state(Unpopulated),
      m_pathAppearance(dd.m_pathAppearance),
      m_renderer(0)
{
    m_model = new FSModel (this);
    m_fmodel = new QSortFilterProxyModel(this);
    m_fmodel->setSourceModel(m_model);
    if (dd.m_state == Populated) {
        populate();
        setCurrentRowNumber(dd.m_currentRow);
    }
}


FileSystemDataset::~FileSystemDataset()
{
}


bool FileSystemDataset::moduleInit()
{
    initMyResource();
    return true;
}


QIcon FileSystemDataset::icon()
{
    QIcon icon(":/images/fs.jpeg");
    return icon;
}


DatasetInterface * FileSystemDataset::createInstance(QObject* parent) const
{
    return new FileSystemDataset(parent);
}


DatasetInterface *FileSystemDataset::objectClone() const
{
    return new FileSystemDataset(*this, parent());
}


DatasetHelperInterface * FileSystemDataset::createHelper(CuteDesigner::Core *designer)
{
    //    if (!m_helper)
    //        m_helper = new FileSystemDatasetHelper(this);

    //    return m_helper;
    return new FileSystemDatasetHelper(this, designer);
}


QString FileSystemDataset::moduleShortName() const
{
    return QString("FileSystem");
}


QString FileSystemDataset::getLastError()
{
    return m_lastError;
}


QString	FileSystemDataset::dir() const
{
    return m_dir;
}


void FileSystemDataset::setDir(const QString &str)
{
    if (m_dir == str)
        return;

    m_dir = str;
    if (m_dir.right(1) == "/")
        m_dir.remove(m_dir.length()-1, 1);

    emit dirChanged(m_dir);
    emit renderingStringsChanged();
    emit changed();
}


FileSystemDataset::Filters FileSystemDataset::filters() const
{
    return m_filters;
}


void FileSystemDataset::setFilters(FileSystemDataset::Filters filters)
{
    if (m_filters == filters)
        return;

    m_filters = filters;

    emit filtersChanged(m_filters);
    emit changed();
}


QString FileSystemDataset::getFieldName(int column )
{
    return m_model->headerData ( column, Qt::Horizontal).toString();
}


QVariant::Type FileSystemDataset::getFieldType(int column)
{
    Q_UNUSED(column)
    return QVariant::String;
}


int FileSystemDataset::recursionLevel() const
{
    return m_recursionLevel;
}


void FileSystemDataset::setRecursionLevel(int recursionLevel)
{
    if (m_recursionLevel == recursionLevel)
        return;

    m_recursionLevel = recursionLevel;

    emit recursionLevelChanged(m_recursionLevel);
    emit changed();
}


int FileSystemDataset::maxNumber() const
{
    return m_maxNumber;
}


void FileSystemDataset::setMaxNumber(int value)
{
    if (m_maxNumber == value)
        return;

    m_maxNumber = value;

    emit maxNumberChanged(m_maxNumber);
    emit changed();
}


FileSystemDataset::PathAppearance FileSystemDataset::pathAppearance() const
{
    return m_pathAppearance;
}


void FileSystemDataset::setPathAppearance(const PathAppearance &value)
{
    if (m_pathAppearance == value)
        return;

    m_pathAppearance = value;

    emit pathAppearanceChanged(m_pathAppearance);
    emit changed();
}


QStringList FileSystemDataset::nameFilters() const
{
    return m_nameFilters;
}


void FileSystemDataset::setNameFilters(const QStringList &filters)
{
    if (m_nameFilters == filters)
        return;

    m_nameFilters = filters;

    emit nameFiltersChanged(m_nameFilters);
    emit renderingStringsChanged();
    emit changed();
}


QList<InternalStringData> FileSystemDataset::renderingStrings()
{
    QList<InternalStringData> list;
    list << InternalStringData(0, m_dir, false, StringHasExpression);
    int id = 1;
    foreach (const QString & nameFilter, m_nameFilters) {
        list << InternalStringData(1, nameFilter, false, StringHasExpression);
        id++;
    }
    return list;
}


void FileSystemDataset::renderInit(ScriptEngineInterface *scriptEngine)
{
    m_renderer = scriptEngine->rendererItemInterface();
}


void FileSystemDataset::renderReset()
{
    m_renderer = 0;
}


QAbstractItemModel * FileSystemDataset::model()
{
    return m_fmodel;
}


bool FileSystemDataset::populate()
{
    ReportCore::log(CuteReport::LogDebug, "FileSystemDataset", QString("\'%1\' populate").arg(objectName()));

    if (m_state == Populating)
        return false;

    emit beforePopulate();

    CuteReport::ReportInterface * report = dynamic_cast<CuteReport::ReportInterface*>(parent());

    QString dir;
    QStringList nameFilters;
    int recursionLevel = m_recursionLevel;
    int maxNumber = m_maxNumber;

    if (m_renderer) {
        ReportCore::log(CuteReport::LogDebug, "FileSystemDataset", QString("populate via renderer"));
        dir = m_renderer->processString(CuteReport::ScriptString(report, this,  m_dir));
//        recursionLevel = m_renderer->processString(CuteReport::ScriptString(report, this,  m_recursionLevel)).toInt();
//        maxNumber = m_renderer->processString(CuteReport::ScriptString(report, this,  m_maxNumber)).toInt();
        foreach (const QString & filter, m_nameFilters)
            nameFilters << m_renderer->processString(CuteReport::ScriptString(report, this, filter));
    } else {
        ReportCore::log(CuteReport::LogDebug, "FileSystemDataset", QString("populate via ReportCore"));
        dir = reportCore()->processString(CuteReport::ScriptString(report, this,  m_dir)).resultStr;
//        recursionLevel = reportCore()->processString(CuteReport::ScriptString(report, this,  m_recursionLevel)).resultStr;
//        maxNumber = reportCore()->processString(CuteReport::ScriptString(report, this,  m_maxNumber)).resultStr;
        foreach (const QString & filter, m_nameFilters)
            nameFilters << reportCore()->processString(CuteReport::ScriptString(report, this, filter)).resultStr;
    }

    m_model->setDir(dir);
    m_model->setNameFilters(nameFilters);
    m_model->setFilters(m_filters);
    m_model->setPathAppearance(m_pathAppearance);
    m_model->setRecursionLevel(recursionLevel);
    m_model->setMaxNumber(maxNumber);

    m_model->populate();

    m_currentRow = m_fmodel->rowCount() > 0 ? 0 : -1;

    m_state = Populated;

    emit afterPopulate();
    return true;
}


bool FileSystemDataset::isPopulated()
{
    return (m_state == Populated);
}


void FileSystemDataset::setPopulated(bool b)
{
    m_state = b ? Populated : Unpopulated;
}


void FileSystemDataset::reset()
{
    m_state = Unpopulated;
    m_model->clear();
    m_currentRow = -1;
    m_lastError = "";
}


void FileSystemDataset::resetCursor()
{
    m_currentRow = -1;
}


bool FileSystemDataset::setFirstRow()
{
    emit(beforeFirst());
    m_currentRow = 0;
    bool ret = getRowCount();
    emit(afterFirst());
    return ret;
}


bool FileSystemDataset::setLastRow()
{
    emit(beforeLast());
    m_currentRow = m_fmodel->rowCount();
    bool ret = m_currentRow < m_fmodel->rowCount() ? true:false;
    emit(afterLast());
    return ret;
}


bool FileSystemDataset::setNextRow()
{
    emit(beforeNext());
    m_currentRow++;
    bool ret = m_currentRow < getRowCount();
    emit(afterNext());
    return ret;
}


bool FileSystemDataset::setPreviousRow()
{
    emit(beforePrevious());
    m_currentRow--;
    bool ret = m_currentRow >= 0;
    emit(afterPrevious());
    return ret;
}


int FileSystemDataset::getCurrentRowNumber()
{
    return m_currentRow;
}


bool FileSystemDataset::setCurrentRowNumber(int index)
{
    emit(beforeSeek(index));
    m_currentRow = index;
    bool ret = (m_currentRow >=0 && m_currentRow < m_fmodel->rowCount() ? true:false);
    emit(afterSeek(index));
    return ret;
}


int FileSystemDataset::getRowCount()
{
    populateIfNeeded();
    return m_fmodel->rowCount();
}


int FileSystemDataset::getColumnCount()
{
    populateIfNeeded();
    return m_fmodel->columnCount();
}


QVariant FileSystemDataset::getValue(int column, int row)
{
    populateIfNeeded();
    return m_fmodel->data( m_fmodel->index(row == -1 ? m_currentRow : row, column) );
}


QVariant FileSystemDataset::getValue(const QString & field, int row)
{
    populateIfNeeded();
    return m_fmodel->data( m_fmodel->index(row == -1 ? m_currentRow : row, m_model->fieldIndex(field) ) );
}


QVariant FileSystemDataset::getNextRowValue(int index)
{
    populateIfNeeded();
    return m_currentRow+1 < m_fmodel->rowCount() && index < m_fmodel->columnCount()
            ? m_fmodel->data( m_fmodel->index(m_currentRow + 1, index ) )
            : QVariant(QVariant::Invalid);
}


QVariant FileSystemDataset::getNextRowValue(const QString & field)
{
    populateIfNeeded();
    return getNextRowValue(m_model->fieldIndex(field));
}


QVariant FileSystemDataset::getPreviousRowValue(int index)
{
    populateIfNeeded();
    return m_currentRow-1 >= 0 && index < m_fmodel->columnCount()
            ? m_fmodel->data( m_fmodel->index(m_currentRow - 1, index ) )
            : QVariant(QVariant::Invalid);
}


QVariant FileSystemDataset::getPreviousRowValue(const QString & field)
{
    populateIfNeeded();
    return getPreviousRowValue(m_model->fieldIndex(field));
}


void FileSystemDataset::populateIfNeeded()
{
    if (m_state == Unpopulated)
        populate();
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(FileSystemDataset, FileSystemDataset)
#endif
