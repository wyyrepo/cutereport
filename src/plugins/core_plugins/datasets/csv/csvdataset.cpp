/***************************************************************************
 *   This file is part of the CuteReport project                           *
 *   Copyright (C) 2013-2014 by Alexander Mikhalov                         *
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
#include "csvdataset.h"
#include "csvdatasethelper.h"
#include "csvdatasetmodel.h"
#include "reportcore.h"
#include "cutereport_functions.h"
#include "sortfilterproxymodel.h"
#include "scriptengineinterface.h"
#include "rendererpublicinterface.h"

#include <QtCore>
#include <QIcon>

#define DATASET_NAME "CSV"

using namespace CuteReport;

inline void initMyResource() { Q_INIT_RESOURCE(csvdataset); }

CsvDataset::CsvDataset(QObject *parent)
    : DatasetInterface(parent),
      m_firstRowIsHeader(false),
      m_currentRow(0),
      m_state(Unpopulated),
      m_delimeter(","),
      m_fixFileIssues(false),
      m_maxRecords(0),
      m_renderer(0)
{
    m_model = new Model (this);
    m_fmodel = new SortFilterProxyModel(this);
    m_fmodel->setSourceModel(m_model);
}


CsvDataset::CsvDataset(const CsvDataset &dd, QObject * parent)
    : DatasetInterface(parent),
      m_firstRowIsHeader(dd.m_firstRowIsHeader),
      m_currentRow(0),
      m_state(Unpopulated),
      m_delimeter(dd.m_delimeter),
      m_fixFileIssues(dd.m_fixFileIssues),
      m_sortFields(dd.m_sortFields),
      m_maxRecords(dd.m_maxRecords),
      m_renderer(0)
{
    m_model = new Model (this);
    m_fmodel = new SortFilterProxyModel(this);
    m_fmodel->setSourceModel(m_model);
    if (dd.m_state == Populated) {
        populate();
        setCurrentRowNumber(dd.m_currentRow);
    }
}


CsvDataset::~CsvDataset()
{
}


bool CsvDataset::moduleInit()
{
    initMyResource();
    return true;
}


QIcon CsvDataset::icon()
{
    QIcon icon(":/images/csv.jpeg");
    return icon;
}


DatasetInterface * CsvDataset::createInstance(QObject* parent) const
{
    return new CsvDataset(parent);
}


DatasetHelperInterface * CsvDataset::createHelper(CuteDesigner::Core * designer)
{
//    if (!m_helper)
//        m_helper = new CsvDatasetHelper(this);

//    return m_helper;
    return new CsvDatasetHelper(this, designer);
}


DatasetInterface *CsvDataset::objectClone() const
{
    return new CsvDataset(*this, parent());
}

void CsvDataset::populateIfNotPopulated()
{
    if (m_state == Unpopulated)
        populate();
}


QString CsvDataset::moduleShortName() const
{
    return QString("CSV");
}


QList<InternalStringData> CsvDataset::renderingStrings()
{
    QList<InternalStringData> list;
    list << InternalStringData(0, m_fileName, false, StringHasExpression);
    return list;
}


void CsvDataset::renderInit(ScriptEngineInterface *scriptEngine)
{
    m_renderer = scriptEngine->rendererItemInterface();
}


void CsvDataset::renderReset()
{
    m_renderer = 0;
}


QString CsvDataset::getLastError()
{
    return m_lastError;
}


QString	CsvDataset::getFileName() const
{
    return m_fileName;
}


void CsvDataset::setFileName(const QString &str)
{
    if (m_fileName == str)
        return;

    m_fileName = str;

    emit fileNameChanged(m_fileName);
    emit renderingStringsChanged();
    emit changed();
}


QString CsvDataset::getDelimeter () const
{
    return m_delimeter;
}


void CsvDataset::setDelimeter (const QString &str)
{
    if (m_delimeter == str)
        return;

    m_delimeter = str;

    emit delimiterChanged(m_delimeter);
    emit changed();
}


bool CsvDataset::getFirstRowIsHeader()
{
    return m_firstRowIsHeader;
}


void CsvDataset::setFirstRowIsHeader(bool value)
{
    if (m_firstRowIsHeader == value)
        return;

    m_firstRowIsHeader = value;

    emit firstRowIsHeaderChanged(m_firstRowIsHeader);
    emit changed();
}


bool CsvDataset::getFixFileIssues()
{
    return m_fixFileIssues;
}


void CsvDataset::setFixFileIssues(bool value)
{
    if (m_fixFileIssues == value)
        return;
    m_fixFileIssues = value;

    emit fixFileIssuesChanged(m_fixFileIssues);
    emit changed();
}


QByteArray CsvDataset::fixIssues(const QByteArray &ba)
{
    QString str(ba);
    str.replace(QRegExp("\r\n|\n|\r"),"\n");
    return str.toUtf8();
}


QString CsvDataset::getTextData() const
{
    return m_textData;
}


void CsvDataset::setTextData(const QString &textData)
{
    if (m_textData == textData)
        return;

    m_textData = textData;

    emit textDataChanged(m_textData);
    emit changed();
}




int CsvDataset::getMaxRecords() const
{
    return m_maxRecords;
}


void CsvDataset::setMaxRecords(int maxRecords)
{
    if (m_maxRecords == maxRecords)
        return;
    m_maxRecords = maxRecords;

    emit maxRecordsChanged(m_maxRecords);
    emit changed();
}


QString CsvDataset::getSortFields() const
{
    return m_sortFields;
}


void CsvDataset::setSortFields(const QString &sortFields)
{
    if (m_sortFields == sortFields)
        return;
    m_sortFields = sortFields.trimmed();

    emit sortFieldsChanged(m_sortFields);
    emit changed();
}


QString CsvDataset::getFieldName(int column )
{
    return m_model->headerData ( column, Qt::Horizontal).toString();
}


QVariant::Type CsvDataset::getFieldType(int column)
{
    Q_UNUSED(column);
    return QVariant::String;
}


QAbstractItemModel * CsvDataset::model()
{
    return m_fmodel;
}


bool CsvDataset::populate()
{
    ReportCore::log(CuteReport::LogDebug, "CsvDataset", "populate", QString("dataset:\'%1\'").arg(objectName()));

    if (m_state == Populating)
        return false;

    emit beforePopulate();

    QStringList list;

    if (m_fileName.isEmpty() && m_textData.isEmpty()) {
        ReportCore::log(CuteReport::LogError, "CsvDataset", "Filename is empty", QString("dataset:\'%1\'").arg(objectName()));
        return false;
    }

    if (!m_fileName.isEmpty()) {

        CuteReport::ReportInterface * report = static_cast<CuteReport::ReportInterface*> (parent());

        QString fileURL;
        if (m_renderer) {
            fileURL = m_renderer->processString(CuteReport::ScriptString(report, this, m_fileName));
        } else {
            fileURL = reportCore()->processString(CuteReport::ScriptString(report, this, m_fileName)).resultStr;
        }

        //    QStringList missedVariables;
        //    if (!isStringValued(m_fileName, report->variables(), &missedVariables)) {
        //        m_lastError = QString("Variable is not defined in the \'File\' field: %1").arg(missedVariables.join(", "));
        //        return false;
        //    }

        //QString fileURL = report ? setVariablesValue(m_fileName, report->variables()) : m_fileName;

        QByteArray fileData = reportCore()->loadObject(fileURL, report);
        if (m_fixFileIssues)
            fileData = fixIssues(fileData);

        QTextStream stream(fileData, QIODevice::Text| QIODevice::ReadOnly | QIODevice::Unbuffered);
        stream.setAutoDetectUnicode(true);

        QString line;
        do {
            line = stream.readLine();
            if (!line.isNull())
                list << line;
        } while (!line.isNull());

    } else if (!m_textData.isEmpty()){
        list = m_textData.split("\n");
    }

    Array array;
    m_model->setHeader((m_firstRowIsHeader && !list.isEmpty()) ? list[0].split(m_delimeter) : QStringList());

    for (int i = (m_firstRowIsHeader ? 1:0); i<list.count(); i++) {
        array.append(list.at(i).split(m_delimeter));
    }
    m_model->setArray(array);

    m_fmodel->clearSort();
    if (!m_sortFields.isEmpty()) {
        QStringList list = m_sortFields.split(",");
        foreach(const QString & field, list) {
            QStringList fieldOpts = field.split(":");
            QString fieldName = fieldOpts.size() > 0 ? fieldOpts.at(0) : QString();
            QString orderStr = fieldOpts.size() > 1 ? fieldOpts.at(1) : QString();
            Qt::SortOrder order = orderStr.toLower().trimmed() == "desc" ? Qt::DescendingOrder : Qt::AscendingOrder;
            int index = m_model->fieldIndex(fieldName);
            m_fmodel->addSortedColumn(index, order);
        }
        m_fmodel->update();
    }

    m_currentRow = array.size() > 0 ? 0 : -1;

    m_state = Populated;

    emit afterPopulate();
    return true;
}


bool CsvDataset::isPopulated()
{
    return (m_state == Populated);
}


void CsvDataset::setPopulated(bool b)
{
    m_state = b ? Populated : Unpopulated;
}


void CsvDataset::reset()
{
    m_state = Unpopulated;
    m_model->clear();
    m_currentRow = -1;
    m_lastError = "";
}


void CsvDataset::resetCursor()
{
    m_currentRow = -1;
}


bool CsvDataset::setFirstRow()
{
    populateIfNotPopulated();
    emit(beforeFirst());
    m_currentRow = 0;
    bool ret = getRowCount();
    emit(afterFirst());
    return ret;
}


bool CsvDataset::setLastRow()
{
    populateIfNotPopulated();
    emit(beforeLast());
    m_currentRow = m_fmodel->rowCount();
    bool ret = m_currentRow < m_fmodel->rowCount() ? true:false;
    emit(afterLast());
    return ret;
}


bool CsvDataset::setNextRow()
{
    populateIfNotPopulated();
    emit(beforeNext());
    m_currentRow++;
    bool ret = m_currentRow < getRowCount();
    emit(afterNext());
    return ret;
}


bool CsvDataset::setPreviousRow()
{
    populateIfNotPopulated();
    emit(beforePrevious());
    m_currentRow--;
    bool ret = m_currentRow >= 0;
    emit(afterPrevious());
    return ret;
}


int CsvDataset::getCurrentRowNumber()
{
    return m_currentRow;
}


bool CsvDataset::setCurrentRowNumber(int index)
{
    populateIfNotPopulated();
    emit(beforeSeek(index));
    m_currentRow = index;
    bool ret = (m_currentRow >=0 && m_currentRow < m_fmodel->rowCount() ? true:false);
    emit(afterSeek(index));
    return ret;
}


int CsvDataset::getRowCount()
{
    populateIfNotPopulated();
    return m_fmodel->rowCount();
}


int CsvDataset::getColumnCount()
{
    populateIfNotPopulated();
    return m_fmodel->columnCount();
}


QVariant CsvDataset::getValue(int column, int row)
{
    populateIfNotPopulated();
    return m_fmodel->data( m_fmodel->index(row == -1 ? m_currentRow : row, column) );
}


QVariant CsvDataset::getValue(const QString & fieldName, int row)
{
    return getValue(m_model->fieldIndex(fieldName), row);
}


QVariant CsvDataset::getNextRowValue(int column)
{
    populateIfNotPopulated();
    return m_model->data(m_model->index(m_currentRow+1, column));
}


QVariant CsvDataset::getNextRowValue(const QString & fieldName)
{
    populateIfNotPopulated();
    return getNextRowValue(m_model->fieldIndex(fieldName));
}


QVariant CsvDataset::getPreviousRowValue(int column)
{
    populateIfNotPopulated();
    return m_model->data(m_model->index(m_currentRow+1, column));
}


QVariant CsvDataset::getPreviousRowValue(const QString & fieldName)
{
    populateIfNotPopulated();
    return getPreviousRowValue(m_model->fieldIndex(fieldName));
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(CsvDataset, CsvDataset)
#endif
