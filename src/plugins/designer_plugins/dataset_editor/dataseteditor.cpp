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
#include "dataseteditor.h"
#include "datasetcontainer.h"
#include "reportcore.h"
#include "datasetinterface.h"
#include "propertyeditor.h"
#include "reportinterface.h"
#include "designercore.h"
#include "renamedialog.h"
#include "mainwindow.h"

using namespace CuteDesigner;

inline void initMyResource() { Q_INIT_RESOURCE(dataset_editor); }

DatasetEditor::DatasetEditor(QObject *parent) :
    ModuleInterface(parent),
    ui(0),
    m_currentDataset(0)
{
}


DatasetEditor::~DatasetEditor()
{
    if (ui)
        delete ui;
}


void DatasetEditor::init(Core *core)
{
    initMyResource();
    ModuleInterface::init(core);

    if (core->getSettingValue("CuteReport_DatasetEditor/tabMode").isNull())
        core->setSettingValue("CuteReport_DatasetEditor/tabMode", 2);

    ui = new DatasetContainer(this);
    ui->setEnabled(false);
    ui->addDatasetPlugins(core->reportCore()->modules(CuteReport::DatasetModule));
    ui->setEnabled(core->currentReport());

    m_propertyEditor = core->createPropertyEditor(ui);
    ui->addPropertyEditor(m_propertyEditor);

    connect(ui.data(), SIGNAL(requestForCreateDataset(QString)) , this, SLOT(slotRequestForCreateDataset(QString)), Qt::QueuedConnection);
    connect(ui.data(), SIGNAL(requestForDeleteDataset()), this, SLOT(slotRequestForRemoveDataset()), Qt::QueuedConnection);
    connect(ui.data(), SIGNAL(currentTabChanged(QString)), this, SLOT(slotCurrentTabChanged(QString)), Qt::QueuedConnection);
    connect(ui.data(), SIGNAL(requestForPopulateDataset()), this, SLOT(slotRequestForPopulatedataset()), Qt::QueuedConnection);
    connect(ui.data(), SIGNAL(requestForRename(QString)), this, SLOT(slotRequestForRename(QString)));

    connect(core, SIGNAL(currentReportChanged(CuteReport::ReportInterface*)), this, SLOT(slotCurrentReportChanged(CuteReport::ReportInterface*)));
}


void DatasetEditor::reloadSettings()
{
    if (ui)
        ui->reloadSettings();
}


void DatasetEditor::saveSettings()
{
    if (ui)
        ui->saveSettings();
}


QWidget * DatasetEditor::view()
{
    return ui;
}


void DatasetEditor::sync()
{
    if (m_currentDataset) {
        qDebug() << m_helpers.contains(m_currentDataset) << m_currentDataset << m_helpers.begin().key();
        CuteReport::DatasetHelperInterface * helper = m_helpers.value(m_currentDataset);
        helper->save();
//        m_currentDataset->createHelper()->save();
    }
}


void DatasetEditor::activate()
{
    core()->reportCore()->log(CuteReport::LogDebug, "DatasetEditor", "activate");
}


void DatasetEditor::deactivate()
{
    sync();
    core()->reportCore()->log(CuteReport::LogDebug, "DatasetEditor", "deactivate");
}


void DatasetEditor::beforeDelete()
{
    m_propertyEditor->setObject(0);
}


QIcon DatasetEditor::icon()
{
    return QIcon(":/images/database_48.png");
}


QString DatasetEditor::name()
{
    return QString("Datasets");
}


void DatasetEditor::slotRequestForCreateDataset(QString moduleName)
{
    sync();
    core()->reportCore()->log(CuteReport::LogDebug, "DatasetEditor", "slotCreateDataset:" + moduleName);
    CuteReport::DatasetInterface * dataset = core()->reportCore()->createDatasetObject(moduleName, core()->currentReport());
    if (dataset) {
        QString name = core()->reportCore()->uniqueName(dataset, "data", core()->currentReport());
        dataset->setObjectName(name);
        dataset->init();
        m_currentDataset = dataset;
        m_datasets.insert(m_currentDataset, m_currentDataset->objectName());
        preprocessDataset(m_currentDataset);
        CuteReport::DatasetHelperInterface * helper = dataset->createHelper(core());
        m_helpers.insert(dataset, helper);
        ui->addTab(helper ? helper : new QWidget(ui), dataset->icon(), dataset->objectName());
        m_propertyEditor->setObject(dataset);
        ui->setCurrentTab(dataset->objectName());
    }
}


void DatasetEditor::slotRequestForRemoveDataset()
{
    if (!m_currentDataset)
        return;

    QList<CuteReport::DatasetInterface*> datasets = core()->currentReport()->datasets();
    int index = -1;
    for (int i=0; i<datasets.count(); i++)
        if (datasets[i] == m_currentDataset) {
            index = i;
            break;
        }


    postprocessDataset(m_currentDataset);
    m_helpers.remove(m_currentDataset);
    ui->deleteTab(m_currentDataset->objectName());
    m_datasets.remove(m_currentDataset);
    delete m_currentDataset;


    datasets = core()->currentReport()->datasets();

    if (index > -1) {
        if (index > datasets.count() -1)
                index = datasets.count() -1;
        if (index < 0)
            index = 0;

        CuteReport::DatasetInterface* newCurrentObject = datasets.count() ? datasets[index] : 0;
        m_currentDataset = newCurrentObject;
        core()->setCurrentDataset(m_currentDataset);
        m_propertyEditor->setObject(m_currentDataset);
        if (newCurrentObject) {
            ui->setCurrentTab(newCurrentObject->objectName());
        }
    }
}


void DatasetEditor::slotCurrentTabChanged(QString name)
{
    sync();
    CuteReport::DatasetInterface * dataset = core()->reportCore()->datasetByName(name, core()->currentReport());
    if (dataset) {
        m_currentDataset = dataset;
        core()->setCurrentDataset(m_currentDataset);
        m_propertyEditor->setObject(dataset);
    }
}


void DatasetEditor::slotRequestForPopulatedataset()
{
    core()->reportCore()->log(CuteReport::LogDebug, "DatasetEditor", "slotRequestForPopulatedataset");
    if (!m_currentDataset)
        return;

//    m_currentDataset->createHelper()->save();
    m_helpers.value(m_currentDataset)->save();
    m_currentDataset->populate();
    ui->setModel(m_currentDataset->model());
    ui->setError(m_currentDataset->getLastError());
}


void DatasetEditor::slotCurrentReportChanged(CuteReport::ReportInterface * report)
{
    CuteReport::ReportCore::log(CuteReport::LogDebug, "DatasetEditor", "slotCurrentReportChanged");
    ui->setEnabled(core()->currentReport());

    ui->deleteAllTabs();

    if (m_currentReport) {
        if (m_currentReport)
            foreach(CuteReport::DatasetInterface * dataset, m_datasets.keys())
                postprocessDataset(dataset);

    }

    m_datasets.clear();
    m_helpers.clear();

    if (report) {
        QStringList datasetNames = report->datasetNames();
        foreach (const QString & datasetName, datasetNames) {
            CuteReport::DatasetInterface * ds = report->dataset(datasetName);
            CuteReport::DatasetHelperInterface * helper = ds->createHelper(m_core);
            m_datasets.insert(ds, ds->objectName());
            m_helpers.insert(ds, helper);
            ui->addTab(helper, ds->icon(), ds->objectName());
            preprocessDataset(ds);
        }

        m_currentDataset = !datasetNames.isEmpty() ? report->dataset(datasetNames.first()) : 0;

        if (m_currentDataset)
            ui->setCurrentTab(m_currentDataset->objectName());

    } else {
        m_currentDataset = 0;
    }

    m_propertyEditor->setObject(m_currentDataset);
    m_currentReport = report;
}


void DatasetEditor::preprocessDataset(CuteReport::DatasetInterface *dataset)
{
    connect(dataset, SIGNAL(objectNameChanged(QString)), this, SLOT(datasetNameChanged(QString)));
}


void DatasetEditor::postprocessDataset(CuteReport::DatasetInterface * dataset)
{
    disconnect(dataset, 0, this, 0);
}


void DatasetEditor::datasetNameChanged(QString name)
{
    CuteReport::DatasetInterface * dataset = static_cast<CuteReport::DatasetInterface*>(sender());

    if (dataset) {
        ui->changeTabText(m_datasets.value(dataset), name);
        m_datasets.insert(dataset, dataset->objectName());
    }
}


void DatasetEditor::slotRequestForRename(QString currentName)
{
    CuteReport::DatasetInterface * dataset = core()->currentReport()->dataset(currentName);
    if (!dataset)
        return;
    RenameDialog d(dataset, core()->currentReport(), core()->mainWindow());
    d.setWindowTitle("Dataset renaming");
    if (d.exec() == QDialog::Accepted) {
        dataset->setObjectName(d.newName());
    }
}

//suit_end_namespace

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(DatasetEditor, DatasetEditor)
#endif
