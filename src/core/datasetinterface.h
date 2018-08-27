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
#ifndef DATASETINTERFACE_H
#define DATASETINTERFACE_H

#include "cutereport_globals.h"
#include "reportplugininterface.h"

#include <QWidget>
#include <QAbstractTableModel>
#include <QVariant>
#include <QSet>

class DatasetEditorInterface;

namespace CuteDesigner
{
    class Core;
}

namespace CuteReport
{

class ReportInterface;
class DatasetHelperInterface;
//class RendererPublicInterface;
class ScriptEngineInterface;

class CUTEREPORT_EXPORTS DatasetInterface : public ReportPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CuteReport::ReportPluginInterface)

    Q_PROPERTY(QString parentDataset READ getParentDataset WRITE setParentDataset)
    Q_PROPERTY(QString filterCondition READ getFilterCondition WRITE setFilterCondition)
    Q_PROPERTY(int filterColumn READ getFilterColumn WRITE setFilterColumn)

    Q_PROPERTY(int datasetFlags READ datasetFlags WRITE setDatasetFlags NOTIFY datasetFlagsChanged DESIGNABLE false)

public:
    enum DatasetFlag {
        BaseReportDataset  = 0x0001,  /** dataset is integrated from base report and cannot be changed */
    };
    Q_DECLARE_FLAGS(DatasetFlags, DatasetFlag)

    enum Stage {Unpopulated, Populating, Populated};

    DatasetInterface(QObject *parent = 0);
    virtual ~DatasetInterface() {}

    virtual void init(){}
    virtual DatasetHelperInterface * createHelper(CuteDesigner::Core * designer) = 0;
    virtual DatasetInterface * clone() const;
    Q_INVOKABLE virtual QAbstractItemModel * model();

    virtual QIcon icon() = 0;
    Q_INVOKABLE virtual QString getLastError();

    Q_INVOKABLE virtual bool populate() = 0;
    Q_INVOKABLE virtual bool isPopulated() = 0;
    Q_INVOKABLE virtual void reset() = 0;
    Q_INVOKABLE virtual void resetCursor() = 0;
    Q_INVOKABLE virtual bool setFirstRow() = 0;
    Q_INVOKABLE virtual bool setLastRow() = 0;
    Q_INVOKABLE virtual bool setNextRow() = 0;
    Q_INVOKABLE virtual bool setPreviousRow() = 0;
    Q_INVOKABLE virtual void setPopulated(bool b) = 0;
    Q_INVOKABLE virtual int getCurrentRowNumber() = 0;
    Q_INVOKABLE virtual bool setCurrentRowNumber(int index) = 0;
    Q_INVOKABLE virtual int getRowCount() = 0;
    Q_INVOKABLE virtual int getColumnCount() = 0;

    Q_INVOKABLE virtual QVariant getValue(int column, int row = -1) = 0;
    Q_INVOKABLE virtual QVariant getValue(const QString & fieldName, int row = -1) = 0;

//    Q_INVOKABLE virtual QVariant getValue(const QString & field, const QString &roleName) = 0;
//    Q_INVOKABLE virtual QVariant getValue(const QString & field, int row, const QString &roleName) = 0;

    Q_INVOKABLE virtual QVariant getNextRowValue(int index);
    Q_INVOKABLE virtual QVariant getNextRowValue(const QString & field);
    Q_INVOKABLE virtual QVariant getPreviousRowValue(int index);
    Q_INVOKABLE virtual QVariant getPreviousRowValue(const QString & field);
    Q_INVOKABLE virtual QString getFieldName(int column);
    Q_INVOKABLE virtual QVariant::Type getFieldType(int column ) = 0;
    Q_INVOKABLE virtual void setFilter ( const int col, const QString & str, Qt::CaseSensitivity cs = Qt::CaseSensitive );

    Q_INVOKABLE QString	getParentDataset();
    Q_INVOKABLE void	setParentDataset(QString pDataset);
    Q_INVOKABLE QString	getFilterCondition();
    Q_INVOKABLE void	setFilterCondition(QString str);
    Q_INVOKABLE int		getFilterColumn();
    Q_INVOKABLE void	setFilterColumn(int col);

    virtual QList<CuteReport::InternalStringData> renderingStrings() = 0;

    /** method called when renderer starts its work, nothing rendered yet */
    virtual void renderInit(CuteReport::ScriptEngineInterface *scriptEngine) = 0;

    /** method called when renderer has completed its work, all is already rendered */
    virtual void renderReset() = 0;

    virtual int datasetFlags() const;
    virtual void setDatasetFlags(int flags);
    virtual void setDatasetFlag(int flag, bool value = true);
    virtual bool isDatasetFlagSet(int flag);

signals:
    void beforeNext();
    void afterNext();
    void beforePrevious();
    void afterPrevious();
    void beforeFirst();
    void afterFirst();
    void beforeLast();
    void afterLast();
    void beforeSeek(int index);
    void afterSeek(int index);
    void beforePopulate();
    void afterPopulate();
    void beforeSetFilter(int col, QString str);
    void afterSetFilter(int col, QString str);
    void renderingStringsChanged();
    void datasetFlagsChanged(int flags);

protected:
    virtual DatasetInterface * createInstance(QObject* parent = 0) const = 0;
    virtual DatasetInterface * objectClone() const = 0;
private:
    QString m_parentDataset;
    QString m_filterCondition;
    int m_filterColumn;
    qint32 m_datasetFlags;

    friend class ReportCore;
};

class DatasetHelperInterface: public QWidget
{
public:
    DatasetHelperInterface (CuteDesigner::Core * designer): m_designer(designer) {}
    virtual void save() {}
    virtual void load() {}

protected:
    CuteDesigner::Core * m_designer;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DatasetInterface::DatasetFlags)

}
Q_DECLARE_METATYPE(CuteReport::DatasetInterface*)
Q_DECLARE_INTERFACE(CuteReport::DatasetInterface, "CuteReport.DatasetInterface/1.0")

#endif
