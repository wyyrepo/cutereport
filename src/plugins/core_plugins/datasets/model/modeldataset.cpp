/***************************************************************************
 *   This file is part of the CuteReport project                           *
 *   Copyright (C) 2014 by Ivan Volkov                                     *
 *   wulff007@gmail.com                                                    *
 *   Copyright (C) 2014-2017  by Alexander Mikhalov                        *
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
#include "modeldataset.h"
#include "modeldatasethelper.h"
#include "models/testmodel.h"
#include "models/clonemodel.h"
#include "reportcore.h"
#include "scriptengineinterface.h"


#include <QIcon>

using namespace CuteReport;

const QString ModuleName("ModelDataset");

inline void initMyResource() { Q_INIT_RESOURCE(modeldataset); }

ModelDataset::ModelDataset(QObject *parent) :
    CuteReport::DatasetInterface(parent),
    m_sourceModel(0),
    m_testModel(new TestModel(this)),    
    m_currentRow(0),
    m_state(Unpopulated),
    m_externalModelAddress(0),
    m_isInited(false),
    m_renderer(0)
{    
}

ModelDataset::ModelDataset(const ModelDataset &dd, QObject *parent) :
    CuteReport::DatasetInterface(parent),
    m_sourceModel(0),
    m_testModel(new TestModel(this)),
    m_addressVariableName(dd.m_addressVariableName),
    m_testModelData(dd.m_testModelData),
    m_currentRow(dd.m_currentRow),
    m_state(Unpopulated),
    m_externalModelAddress(dd.m_externalModelAddress),
    m_renderer(0)
{
}


ModelDataset::~ModelDataset()
{
}


bool ModelDataset::moduleInit()
{
    initMyResource();
    return true;
}


void ModelDataset::init()
{
    m_isInited = true;
    if (m_addressVariableName.isEmpty())
        setAddressVariable(objectName() + "_model");
    DatasetInterface::init();
}


void ModelDataset::setAddressVariable(QString name)
{
    if (name == m_addressVariableName)
        return;

    QString newName = name.trimmed();

    if (ReportInterface * report = qobject_cast<ReportInterface*>(parent())) {
        if (report->variableExists(newName))
            report->renameVariable(m_addressVariableName, newName);
        else if (newName.isEmpty())
            report->removeVariable(m_addressVariableName);
        m_addressVariableName = newName;
        report->setVariableValue(m_addressVariableName, QVariant());
        /// FIXME: v1.4 replace for string below when report will be able to store/restore Object variables
//        report->setVariableObject(m_addressVariableName, 0);
    } else
        m_addressVariableName = newName;

    emit addressVariableChanged(m_addressVariableName);
    emit renderingStringsChanged();
    emit changed();
}


QString ModelDataset::addressVariable() const
{
    return m_addressVariableName;
}


void ModelDataset::setModelAddress(quint64 address)
{
    if (!m_isInited)    // do not restore address from serializer, it is not actual
        return;
    if (address == m_externalModelAddress)
        return;

    m_externalModelAddress = address;

    emit changed();
    emit modelAddressChanged(m_externalModelAddress);
}


quint64 ModelDataset::modelAddress() const
{
    return m_externalModelAddress;
}


QByteArray ModelDataset::testModelData() const
{
    return m_testModelData;
}


void ModelDataset::setTestModelData(QByteArray data)
{
    if (data == m_testModelData)
        return;

    m_testModelData = data;

    emit changed();
    emit testModelDataChanged(m_testModelData);
}


QAbstractItemModel *ModelDataset::model()
{
    return !m_sourceModel ? m_testModel : m_sourceModel;
}


TestModel *ModelDataset::testModel() const
{
    return m_testModel;
}


void ModelDataset::renderInit(ScriptEngineInterface *scriptEngine)
{
    m_renderer = scriptEngine->rendererItemInterface();
}


void ModelDataset::renderReset()
{
    m_renderer = 0;
}


QList<InternalStringData> ModelDataset::renderingStrings()
{
    QList<InternalStringData> list;
    list << InternalStringData(0, m_addressVariableName, false, StringHasExpression);
    return list;
}


CuteReport::DatasetInterface *ModelDataset::objectClone() const
{
    return new ModelDataset(*this, parent());
}


QString ModelDataset::moduleShortName() const
{
    return QString("Model");
}


CuteReport::DatasetHelperInterface *ModelDataset::createHelper(CuteDesigner::Core *designer)
{
//    if (!m_helper) {
//        m_helper = new ModelDatasetHelper(this);
//    }

//    return m_helper;
    return new ModelDatasetHelper(this, designer);
}


QIcon ModelDataset::icon()
{
    return QIcon(":/images/model.png");
}


bool ModelDataset::setFirstRow()
{
    emit beforeFirst();
    m_currentRow = 0;
    bool ret = getRowCount();
    emit afterFirst();
    return ret;
}


bool ModelDataset::setLastRow()
{
    emit beforeLast();
    m_currentRow = model()->rowCount();
    bool ret = m_currentRow < model()->rowCount() ? true:false;
    emit afterLast();
    return ret;
}


bool ModelDataset::setNextRow()
{
    emit beforeNext();
    m_currentRow++;
    bool ret = m_currentRow < getRowCount();
    emit afterNext();
    return ret;
}


bool ModelDataset::setPreviousRow()
{
    emit beforePrevious();
    m_currentRow--;
    bool ret = m_currentRow >= 0;
    emit afterPrevious();
    return ret;
}


bool ModelDataset::populate()
{
    /// if populate is called, skip initialization - perhaps model is added dynamically by user
    m_isInited = true;

    ReportCore::log(CuteReport::LogDebug, ModuleName, "populate", QString("dataset:\'%1\'").arg(objectName()));

    if (m_state == Populating)
        return false;

    emit beforePopulate();

    CuteReport::ReportInterface * report = dynamic_cast<CuteReport::ReportInterface*>(parent());

    QAbstractItemModel *model = 0;
    delete m_sourceModel;
    m_sourceModel = 0;

    if (!m_addressVariableName.isEmpty()) {
        if (report->variables().contains(m_addressVariableName)) {
            QVariant var = report->variableValue(addressVariable());
            if (var.type() == QVariant::ULongLong) {
                ReportCore::log(CuteReport::LogError, ModuleName, "populate", QString("dataset:\'%1\' passed as QVariant::ULongLong").arg(objectName()));
                model = (QAbstractItemModel*)(var.toULongLong());
            } else if (var.canConvert<QObject*>()) {
                ReportCore::log(CuteReport::LogError, ModuleName, "populate", QString("dataset:\'%1\' passed as QObject*").arg(objectName()));
                model = qobject_cast<QAbstractItemModel*>(var.value<QObject*>());
            } else
                ReportCore::log(CuteReport::LogError, ModuleName, "populate", QString("dataset:\'%1\' passed variable is not of type QVariant::ULongLong nor QObject*").arg(objectName()));
        } else if (report->variableObjectsNames().contains(m_addressVariableName)) {
            QObject * object = report->variableObject(m_addressVariableName);
            model = qobject_cast<QAbstractItemModel*>(object);
            if (model)
                ReportCore::log(CuteReport::LogError, ModuleName, "populate", QString("dataset:\'%1\' passed as QObject*").arg(objectName()));
            else if (object)
                ReportCore::log(CuteReport::LogError, ModuleName, "populate", QString("dataset:\'%1\' passed as QObject*, but cannot be casted to QAbstractItemModel").arg(objectName()));
            else
                ReportCore::log(CuteReport::LogError, ModuleName, "populate", QString("dataset:\'%1\' property \'addressVariable\' is defined, but not in Report's object variable list").arg(objectName()));
        } else
            ReportCore::log(CuteReport::LogError, ModuleName, "populate", QString("dataset:\'%1\' property \'addressVariable\' is defined, but not in Report's list").arg(objectName()));
    } else if (m_externalModelAddress != 0) {
        ReportCore::log(CuteReport::LogDebug, ModuleName, "populate", QString("dataset:\'%1\' property \'addressVariable\' is not set").arg(objectName()));
        model = (QAbstractItemModel*)(m_externalModelAddress);
        if (m_externalModelAddress)
            ReportCore::log(CuteReport::LogDebug, ModuleName, "populate", QString("dataset:\'%1\' property \'modelAddress\' is set and used").arg(objectName()));
        else
            ReportCore::log(CuteReport::LogDebug, ModuleName, "populate", QString("dataset:\'%1\' property \'modelAddress\' is not set").arg(objectName()));
    }

    /// we dont' use threading for now - so temporary commented out
    m_sourceModel = model;
//    if (model) {
//        CloneModel * cloneModel = new CloneModel(this);
//        cloneModel->populate(model);

//        m_sourceModel = cloneModel;
//    }

    if (!m_sourceModel) {
        ReportCore::log(CuteReport::LogDebug, ModuleName, "populate", QString("dataset:\'%1\' source model is not set, so the test one will be used.").arg(objectName()));
        m_testModel->load(testModelData());
        m_currentRow = m_testModel->rowCount() > 0 ? 0 : -1;
    } else
        m_currentRow = m_sourceModel->rowCount() > 0 ? 0 : -1;

    m_state = Populated;

    emit afterPopulate();
    return true;
}


bool ModelDataset::isPopulated()
{
    return (m_state == Populated);
}


void ModelDataset::setPopulated(bool b)
{
    m_state = b ? Populated : Unpopulated;
}


void ModelDataset::reset()
{
    m_state = Unpopulated;
    m_testModel->clear();
    m_currentRow = -1;
}


void ModelDataset::resetCursor()
{
    m_currentRow = -1;
}


int ModelDataset::getCurrentRowNumber()
{
    return m_currentRow;
}


bool ModelDataset::setCurrentRowNumber(int index)
{
    emit beforeSeek(index);
    m_currentRow = index;
    bool ret = (m_currentRow >=0 && m_currentRow < model()->rowCount() ? true:false);
    emit afterSeek(index);
    return ret;
}


int ModelDataset::getRowCount()
{
    populateIfNeeded();
    return model()->rowCount();
}


CuteReport::DatasetInterface *ModelDataset::createInstance(QObject *parent) const
{
    return new ModelDataset(parent);
}


int ModelDataset::columnIndexByName(QString name)
{
    if (name.isEmpty()) {
        qWarning() << "column name is empty";
        return -1;
    }

    for (int i = 0; i < model()->columnCount(); i++) {
//        qDebug() << sourceModel()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
        if (model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == name) {
            return i;
        }
    }

    int col = columnIndexIn(name, "field");

    if (col == -1) {
        col = columnIndexIn(name, "col");
    }

    if (col == -1) {
        qWarning() << name << "Field name is not defined in source or test model";
    }
    return col;
}


int ModelDataset::columnIndexIn(const QString &name, const QString &templateName) const
{
    int col = -1;
    QString tmp = name;
    if (tmp.contains(templateName, Qt::CaseInsensitive)) {
        tmp.remove(templateName, Qt::CaseInsensitive);

        bool ok = false;
        col = tmp.toInt(&ok);

        if (!ok) {
            col = -1;
            qWarning() << name << "incorrect field name";
        } else {
            return col - 1;
        }
    }

    return col;
}


QString ModelDataset::getLastError()
{
    return m_lastError;
}


int ModelDataset::getColumnCount()
{
    populateIfNeeded();
    return model()->columnCount();
}


QVariant ModelDataset::getValue(int column, int row)
{
    populateIfNeeded();
    return model()->index(row == -1 ? m_currentRow : row, column).data(Qt::DisplayRole);
}


QVariant ModelDataset::getValue(const QString &field, int row)
{
    return getValue(columnIndexByName(field), row);
}


QVariant ModelDataset::getNextRowValue(int index)
{
    populateIfNeeded();

    return m_currentRow <= model()->rowCount() && index < model()->columnCount()
            ? model()->index(m_currentRow + 1, index).data()
            : QVariant();
}


QVariant ModelDataset::getNextRowValue(const QString &field)
{
    return getNextRowValue(columnIndexByName(field));
}


QVariant ModelDataset::getPreviousRowValue(int index)
{
    populateIfNeeded();

    return m_currentRow-1 >= 0 && index < model()->columnCount()
            ? model()->index(m_currentRow, index).data(Qt::DisplayRole)
            : QVariant();
}


QVariant ModelDataset::getPreviousRowValue(const QString &field)
{
    return getPreviousRowValue(columnIndexByName(field));
}


QString ModelDataset::getFieldName(int column)
{
    Q_UNUSED(column)
    return QString();
}


QVariant::Type ModelDataset::getFieldType(int column)
{
    Q_UNUSED(column)
    return QVariant::String;
}


void ModelDataset::populateIfNeeded()
{
    if (m_state == Unpopulated)
        populate();
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(ModelDataset, ModelDataset)
#endif



