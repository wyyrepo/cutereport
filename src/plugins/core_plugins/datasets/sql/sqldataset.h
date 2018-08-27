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
#ifndef SQLDATASET_H
#define SQLDATASET_H

#include <QtSql>
#include <QSortFilterProxyModel>

#include "datasetinterface.h"
#include "reportinterface.h"
#include "cutereport_globals.h"
#include "scriptengineinterface.h"

#define DATASET_NAME "SQL"

class SqlDataset : public CuteReport::DatasetInterface
{
	Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "CuteReport.DatasetInterface/1.0")
#endif
    Q_INTERFACES(CuteReport::DatasetInterface)

    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QString dbhost READ dbhost WRITE setdbhost NOTIFY dbhostChanged)
    Q_PROPERTY(QString dbname READ dbname WRITE setdbname NOTIFY dbnameChanged)
    Q_PROPERTY(QString dbuser READ dbuser WRITE setdbuser NOTIFY dbuserChanged)
    Q_PROPERTY(QString dbpasswd READ dbpasswd WRITE setdbpasswd NOTIFY dbpasswdChanged)
    Q_PROPERTY(QString driver READ driver WRITE setDriver NOTIFY driverChanged)
    Q_PROPERTY(QString options READ options WRITE setOptions NOTIFY optionsChanged)

public:
    explicit SqlDataset(QObject *parent = 0);
    virtual ~SqlDataset();

    virtual bool moduleInit();

    virtual QIcon icon();

    virtual DatasetInterface * createInstance(QObject* parent = 0) const;
    virtual CuteReport::DatasetHelperInterface * createHelper(CuteDesigner::Core *designer);
    virtual QAbstractItemModel * model();

    virtual QString getLastError();

    virtual bool populate();
    virtual bool isPopulated();
    virtual void setPopulated(bool b);
    virtual void reset();
    virtual void resetCursor();
    virtual bool setFirstRow();
    virtual bool setLastRow();
    virtual bool setNextRow();
    virtual bool setPreviousRow();
    virtual int  getCurrentRowNumber();
    virtual bool setCurrentRowNumber(int index);
    virtual int getRowCount();
    virtual int getColumnCount();
    virtual QVariant getValue(int column, int row = -1);
    virtual QVariant getValue(const QString & fieldName, int row = -1);
    virtual QVariant getNextRowValue(int index);
    virtual QVariant getNextRowValue(const QString & field);
    virtual QVariant getPreviousRowValue(int index);
    virtual QVariant getPreviousRowValue(const QString & field);
    virtual QString getFieldName(int column );
    virtual QVariant::Type getFieldType(int column);

    QString	    query() const;
    void	    setQuery(const QString &str);

    QStringList drivers() const;
    QString driver() const;
    void setDriver(QString driver);
    QString dbhost() const;
    void setdbhost(QString host);
    QString dbuser() const;
    void setdbuser(QString user);
    QString dbname() const;
    void setdbname(QString name);
    QString dbpasswd() const;
    void setdbpasswd(QString passwd);
    QString options() const;
    void setOptions(const QString &options);

    virtual QList<CuteReport::InternalStringData> renderingStrings();

    virtual QString moduleShortName() const {return QString("SQL");}
    virtual QString suitName() const { return "Standard"; }

    virtual void renderInit(CuteReport::ScriptEngineInterface *scriptEngine);
    virtual void renderReset();


signals:
    void queryChanged(QString);
    void dbhostChanged(QString);
    void dbnameChanged(QString);
    void dbuserChanged(QString);
    void dbpasswdChanged(QString);
    void driverChanged(QString);
    void optionsChanged(QString);

private:
    explicit SqlDataset(const SqlDataset &dd, QObject * parent);
    virtual DatasetInterface * objectClone() const;

    inline void populateIfNeeded();

	int m_currentRow;
    Stage m_state;
	QString m_queryText;
	QSqlQueryModel * m_model;
	QSortFilterProxyModel * m_fmodel;
//    QPointer<CuteReport::DatasetHelperInterface> m_helper;
    QString m_dbhost;
    QString m_dbname;
    QString m_dbuser;
    QString m_dbpasswd;
    QString m_driver;
    QSqlDatabase db;
    QString m_connectionName;
    QString m_lastError;
    QString m_options;

    CuteReport::RendererPublicInterface * m_renderer;
};

#endif
