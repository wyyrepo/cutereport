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
 ***************************************************************************/
#include "sqldataset.h"
#include "sqldatasethelper.h"
#include "reportcore.h"
#include "cutereport_functions.h"
#include "rendererpublicinterface.h"

#include <QSqlDatabase>

using namespace CuteReport;

inline void initMyResource() { Q_INIT_RESOURCE(sqldataset); }

SqlDataset::SqlDataset(QObject *parent)
    : CuteReport::DatasetInterface(parent)
    ,m_renderer(0)
{
    m_currentRow = 0;
    m_state = Unpopulated;
    m_model = new QSqlQueryModel  (this);
    m_fmodel = new QSortFilterProxyModel(this);
    m_fmodel->setSourceModel(m_model);
    m_connectionName = QString("_sqldataset_%1").arg(QTime::currentTime().msec());
}


SqlDataset::SqlDataset(const SqlDataset &dd, QObject * parent)
    : DatasetInterface(parent)
    ,m_renderer(0)
{
    m_currentRow = 0;
    m_state = Unpopulated;
    m_queryText = dd.m_queryText;
    m_dbhost = dd.m_dbhost;
    m_dbname = dd.m_dbname;
    m_dbuser = dd.m_dbuser;
    m_dbpasswd = dd.m_dbpasswd;
    m_driver = dd.m_driver;
    m_connectionName = dd.m_connectionName;
    m_lastError = dd.m_lastError;
    m_model = new QSqlQueryModel(this);
    m_fmodel = new QSortFilterProxyModel(this);
    m_fmodel->setSourceModel(m_model);
    if (dd.m_state == Populated) {
        populate();
        setCurrentRowNumber(dd.m_currentRow);
    }
}

SqlDataset::~SqlDataset()
{
    //    if (m_helper)
    //        delete m_helper;
}


bool SqlDataset::moduleInit()
{
    initMyResource();
    return true;
}


QIcon SqlDataset::icon()
{
    QIcon icon(":/images/sql.png");
    return icon;
}


CuteReport::DatasetInterface * SqlDataset::createInstance(QObject* parent) const
{
    return new SqlDataset(parent);
}


DatasetInterface *SqlDataset::objectClone() const
{
    return new SqlDataset(*this, parent());
}


void SqlDataset::populateIfNeeded()
{
    if (m_state == Unpopulated)
        populate();
}


DatasetHelperInterface * SqlDataset::createHelper(CuteDesigner::Core * designer)
{
    //    if (!m_helper)
    //        m_helper = new SqlDatasetHelper(this);

    //    return m_helper;
    return new SqlDatasetHelper(this, designer);
}


QString SqlDataset::getLastError()
{
    if (!db.isValid() || db.isOpenError() || !db.isOpen())
        return m_lastError;
    return m_model->lastError().text();
}


QString SqlDataset::getFieldName(int column )
{
    populateIfNeeded();
    return m_model->record().fieldName(column);
    //    } else
    //        return m_model->headerData ( column, Qt::Horizontal).toString();
}


QVariant::Type SqlDataset::getFieldType(int column)
{
    Q_UNUSED(column)
    return QVariant::String;
}



QAbstractItemModel * SqlDataset::model()
{
    return m_fmodel;
}


QString SqlDataset::query() const
{
    return m_queryText;
}


void SqlDataset::setQuery(const QString &str)
{
    if (m_queryText == str)
        return;

    m_queryText = str;

    emit queryChanged(m_queryText);
    emit renderingStringsChanged();
    emit changed();
}


bool SqlDataset::populate()
{
    ReportCore::log(CuteReport::LogDebug, "SqlDataset", QString("\'%1\' populate").arg(objectName()));

    if (m_state == Populating)
        return false;

    emit beforePopulate();

    reset();

    m_state = Populating;

    QString script;
    QString dbHost;
    QString dbName;
    QString dbUser;
    QString dbPassword;
    QString dbDriver;
    QString dbOptions;

    CuteReport::ReportInterface* report = dynamic_cast<CuteReport::ReportInterface*> (parent());
    if (!report) {
        m_state = Unpopulated;
        return false;
    }

    if (m_renderer) {
        ReportCore::log(CuteReport::LogDebug, "SqlDataset", QString("populate via renderer"));
        script = m_renderer->processString(CuteReport::ScriptString(report, this, m_queryText));
        dbHost = m_renderer->processString(CuteReport::ScriptString(report, this, m_dbhost));
        dbName = m_renderer->processString(CuteReport::ScriptString(report, this, m_dbname));
        dbUser = m_renderer->processString(CuteReport::ScriptString(report, this, m_dbuser));
        dbPassword = m_renderer->processString(CuteReport::ScriptString(report, this, m_dbpasswd));
        dbDriver = m_renderer->processString(CuteReport::ScriptString(report, this,  m_driver));
        dbOptions = m_renderer->processString(CuteReport::ScriptString(report, this,  m_options));
    } else {
        ReportCore::log(CuteReport::LogDebug, "SqlDataset", QString("populate via ReportCore"));
        script = reportCore()->processString(CuteReport::ScriptString(report, this, m_queryText)).resultStr;
        dbHost = reportCore()->processString(CuteReport::ScriptString(report, this, m_dbhost)).resultStr;
        dbName = reportCore()->processString(CuteReport::ScriptString(report, this, m_dbname)).resultStr;
        dbUser = reportCore()->processString(CuteReport::ScriptString(report, this, m_dbuser)).resultStr;
        dbPassword = reportCore()->processString(CuteReport::ScriptString(report, this, m_dbpasswd)).resultStr;
        dbDriver = reportCore()->processString(CuteReport::ScriptString(report, this,  m_driver)).resultStr;
        dbOptions = reportCore()->processString(CuteReport::ScriptString(report, this,  m_options)).resultStr;
    }

    ReportCore::log(CuteReport::LogDebug, "SqlDataset", QString("query = %1").arg(script));

    dbName = dbHost.isEmpty() ? reportCore()->localCachedFileName(dbName, report) : dbName ;
    if (dbName.isEmpty()) {
        ReportCore::log(CuteReport::LogError, "SqlDataset", "Database name is empty", QString("dataset:\'%1\'").arg(objectName()));
        m_state = Unpopulated;
        return false;
    }

    if (dbDriver.isEmpty()) {
        ReportCore::log(CuteReport::LogError, "SqlDataset", "Database driver not defined", QString("dataset:\'%1\'").arg(objectName()));
        m_state = Unpopulated;
        return false;
    }

    if (script.isEmpty()) {
        ReportCore::log(CuteReport::LogError, "SqlDataset", "Query text is empty", QString("dataset:\'%1\'").arg(objectName()));
        m_state = Unpopulated;
        return false;
    }

    db = QSqlDatabase::database(m_connectionName, false);
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase(dbDriver, m_connectionName);
        if (!db.isValid()) {
            ReportCore::log(CuteReport::LogError, "SqlDataset", "Database error", QString("dataset:\'%1\' error:\'%2\'").arg(objectName(), db.lastError().text()));
            m_state = Unpopulated;
            return false;
        }
    }

    db.setHostName(dbHost);
    db.setDatabaseName(dbName);
    db.setUserName(dbUser);
    db.setPassword(dbPassword);
    db.setConnectOptions(dbOptions);

    if (!db.open()) {
        ReportCore::log(CuteReport::LogError, "SqlDataset", "Cannot open database", QString("dataset:\'%1\' error:\'%2\'").arg(objectName(), db.lastError().text()));
        m_state = Unpopulated;
        return false;
    }

    m_model->setQuery(script, db);
    bool ret = m_model->lastError().type() == QSqlError::NoError;

    if (ret) {
        if (!QSqlDatabase::database().driver()->hasFeature(QSqlDriver::QuerySize)) {
            while (m_model->canFetchMore())
                m_model->fetchMore();
        }
    } else {
        ReportCore::log(CuteReport::LogError, "SqlDataset", "SQL Query error", QString("dataset:\'%1\' error:\'%2\'").arg(objectName(), m_model->lastError().text()));
        m_state = Unpopulated;
        return false;
    }

    m_fmodel->setSourceModel(m_model);
    m_currentRow = m_fmodel->rowCount() > 0 ? 0 : -1;

    m_state = ret ? Populated : Unpopulated;

    emit afterPopulate();

    return ret;
}


bool SqlDataset::isPopulated()
{
    return (m_state == Populated);
}


void SqlDataset::setPopulated(bool b)
{
    m_state = b ? Populated : Unpopulated;
}


void SqlDataset::reset()
{
    m_state = Unpopulated;
    m_model->clear();
    m_currentRow = -1;
    m_lastError = "";
    db.close();
}


void SqlDataset::resetCursor()
{
    m_currentRow = -1;
}


bool SqlDataset::setFirstRow()
{
    populateIfNeeded();
    emit(beforeFirst());

    m_currentRow = 0;
    bool ret = getRowCount();

    emit(afterFirst());

    return ret;
}


bool SqlDataset::setLastRow()
{
    populateIfNeeded();
    emit(beforeLast());

    m_currentRow = m_fmodel->rowCount();
    bool ret = m_fmodel->index(m_currentRow, 0).isValid();

    emit(afterLast());

    return ret;
}


bool SqlDataset::setNextRow()
{
    populateIfNeeded();
    emit(beforeNext());

    m_currentRow++;
    bool ret = m_currentRow < getRowCount();

    emit(afterNext());

    return ret;
}


bool SqlDataset::setPreviousRow()
{
    populateIfNeeded();
    emit(beforePrevious());

    m_currentRow--;
    bool ret = m_currentRow >= 0;

    emit(afterPrevious());

    return ret;
}


int SqlDataset::getCurrentRowNumber()
{
    return m_currentRow;
}


bool SqlDataset::setCurrentRowNumber(int index)
{
    populateIfNeeded();
    emit(beforeSeek(index));

    m_currentRow = index;
    bool ret = m_fmodel->index(m_currentRow, 0).isValid();

    emit(afterSeek(index));

    return ret;
}


int SqlDataset::getRowCount()
{
    populateIfNeeded();
    return m_fmodel->rowCount();
}


int SqlDataset::getColumnCount()
{
    populateIfNeeded();
    return m_fmodel->columnCount();
}


QVariant SqlDataset::getValue(int index, int row)
{
    populateIfNeeded();
    return m_fmodel->data( m_fmodel->index(row == -1 ? m_currentRow : row,index) );
}


QVariant SqlDataset::getValue(const QString & field, int row)
{
    populateIfNeeded();
    //    qDebug() << m_currentRow << m_fmodel->data( m_fmodel->index(m_currentRow, m_model->record().indexOf(field) ) ).toString();
    return m_fmodel->data( m_fmodel->index(row == -1 ? m_currentRow : row, m_model->record().indexOf(field) ) );
}


QVariant SqlDataset::getNextRowValue(int index)
{
    populateIfNeeded();
    return m_currentRow+1 < m_fmodel->rowCount() && index < m_fmodel->columnCount() ?  m_fmodel->data( m_fmodel->index(m_currentRow + 1,index) ) : QVariant::Invalid;
}


QVariant SqlDataset::getNextRowValue(const QString & field)
{
    populateIfNeeded();
    return m_currentRow+1 < m_fmodel->rowCount() ?  m_fmodel->data( m_fmodel->index(m_currentRow + 1, m_model->record().indexOf(field) ) ) : QVariant::Invalid;
}


QVariant SqlDataset::getPreviousRowValue(int index)
{
    populateIfNeeded();
    return m_currentRow-1 < 0 && index < m_fmodel->columnCount() ?  m_fmodel->data( m_fmodel->index(m_currentRow - 1,index) ) : QVariant::Invalid;
}


QVariant SqlDataset::getPreviousRowValue(const QString & field)
{
    populateIfNeeded();
    return m_currentRow-1 < 0  ?  m_fmodel->data( m_fmodel->index(m_currentRow - 1, m_model->record().indexOf(field) ) )  : QVariant::Invalid;
}


QStringList SqlDataset::drivers() const
{
    return QSqlDatabase::drivers();
}


QString SqlDataset::driver() const
{
    return m_driver;
}


void SqlDataset::setDriver(QString driver)
{
    if (m_driver == driver)
        return;

    m_driver = driver;

    emit driverChanged(m_driver);
    emit renderingStringsChanged();
    emit changed();
}


QString SqlDataset::dbhost() const
{
    return m_dbhost;
}


void SqlDataset::setdbhost(QString host)
{
    if (m_dbhost == host)
        return;

    m_dbhost = host;

    emit dbhostChanged(m_dbhost);
    emit renderingStringsChanged();
    emit changed();
}


QString SqlDataset::dbuser() const
{
    return m_dbuser;
}


void SqlDataset::setdbuser(QString user)
{
    if (m_dbuser == user)
        return;

    m_dbuser = user;

    emit dbuserChanged(m_dbuser);
    emit renderingStringsChanged();
    emit changed();
}


QString SqlDataset::dbname() const
{
    return m_dbname;
}


void SqlDataset::setdbname(QString name)
{
    if (m_dbname == name)
        return;

    m_dbname = name;

    emit dbnameChanged(m_dbname);
    emit renderingStringsChanged();
    emit changed();
}


QString SqlDataset::dbpasswd() const
{
    return m_dbpasswd;
}


void SqlDataset::setdbpasswd(QString passwd)
{
    if (m_dbpasswd == passwd)
        return;

    m_dbpasswd = passwd;

    emit dbpasswdChanged(m_dbpasswd);
    emit renderingStringsChanged();
    emit changed();
}


QString SqlDataset::options() const
{
    return m_options;
}


void SqlDataset::setOptions(const QString &options)
{
    if (m_options == options)
        return;

    m_options = options;

    emit optionsChanged(m_options);
    emit renderingStringsChanged();
    emit changed();
}



QList<InternalStringData> SqlDataset::renderingStrings()
{
    QList<InternalStringData> list;
    list << InternalStringData(0, m_queryText, false, StringHasExpression); // can have only parameter variable for now ${myVar}
    list << InternalStringData(1, m_dbhost, false, StringHasExpression);
    list << InternalStringData(2, m_dbname, false, StringHasExpression);
    list << InternalStringData(3, m_dbuser, false, StringHasExpression);
    list << InternalStringData(4, m_dbpasswd, false, StringHasExpression);
    list << InternalStringData(5, m_driver, false, StringHasExpression);
    list << InternalStringData(6, m_options, false, StringHasExpression);
    return list;
}


void SqlDataset::renderInit(ScriptEngineInterface *scriptEngine)
{
    m_renderer = scriptEngine->rendererItemInterface();
}


void SqlDataset::renderReset()
{
    m_renderer = 0;
}




#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(SqlDataset, SqlDataset)
#endif
