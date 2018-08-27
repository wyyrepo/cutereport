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

#include "reporteditor.h"
#include "reportcontainer.h"
#include "reportproperties.h"
#include "reportcore.h"
#include "reportinterface.h"
#include "propertyeditor.h"
#include "rendererinterface.h"
#include "printerinterface.h"
#include "storageinterface.h"
#include "mainwindow.h"
#include "renamedialog.h"
#include "mainwindow.h"

#include <QMenu>
#include <QWidget>
#include <QMessageBox>
#include <QInputDialog>

using namespace CuteReport;

const int documentId = 98765;
const QString MODULENAME = "ReportEditor";

inline void initMyResource() { Q_INIT_RESOURCE(reporteditor); }

ReportEditor::ReportEditor(QObject *parent) :
    ModuleInterface(parent)
  ,m_prevReportNumber(20)
  ,m_newReportAction(0)
  ,m_openReportAction(0)
  ,m_saveReportAction(0)
  ,m_saveReportAsAction(0)
  ,m_closeReportAction(0)
  ,m_previousReportsAction(0)
{
}


ReportEditor::~ReportEditor()
{
    foreach (ReportStruct st, m_reports) {
        disconnect(st.report, 0, 0, 0);
        delete st.report;
    }
    delete ui;
}


void ReportEditor::init(CuteDesigner::Core *core)
{
    initMyResource();
    ModuleInterface::init(core);

    ui = new ReportContainer(this);

    if (core->getSettingValue("CuteReport_ReportEditor/tabMode").isNull())
        core->setSettingValue("CuteReport_ReportEditor/tabMode", 2);

    connect(core, SIGNAL(currentReportChanged(CuteReport::ReportInterface*)),
            this, SLOT(slotCurrentReportChangedByCore(CuteReport::ReportInterface*)));
    connect(core, SIGNAL(requestForReport(QString)), this, SLOT(loadReport(QString)));
    connect(core, SIGNAL(appIsAboutToClose(bool*)), this, SLOT(slotAppIsAboutToClose(bool*)));
    connect(core, SIGNAL(initDone()), this, SLOT(slotDesignerInitDone()));

    connect(core->reportCore(), SIGNAL(rendererStarted(CuteReport::ReportInterface*)), this, SLOT(slotRendererStarted(CuteReport::ReportInterface*)));
    connect(core->reportCore(), SIGNAL(rendererDone(CuteReport::ReportInterface*,bool)), this, SLOT(slotRendererStopped(CuteReport::ReportInterface*, bool)));

    // queued because need to have lowest priority for preventing emmitting currentReportChanged before reportCreated
    // cancelled since after mass undoredo delete/create pointer is not valid
    connect(core, SIGNAL(newReport_after(CuteReport::ReportInterface*)), this, SLOT(slotCoreReportCreated(CuteReport::ReportInterface*)));
    connect(core, SIGNAL(loadReport_after(CuteReport::ReportInterface*)), this, SLOT(slotCoreReportCreated(CuteReport::ReportInterface*)));

    connect(ui.data(), SIGNAL(requestForNewReport()), this, SLOT(slotRequestForNewReport()));
    connect(ui.data(), SIGNAL(requestForTemplate()), this, SLOT(slotRequestForTemplate()));
    connect(ui.data(), SIGNAL(requestForOpenReport()), this, SLOT(slotRequestForOpenReport()));
    connect(ui.data(), SIGNAL(requestForCloseReport()), this, SLOT(slotRequestForCloseReport()));
    connect(ui.data(), SIGNAL(requestForSaveReport()), this, SLOT(slotRequestForSaveReport()));
    connect(ui.data(), SIGNAL(currentTabChanged(int)), this, SLOT(slotCurrentTabChanged(int)));
    //    connect(ui.data(), SIGNAL(requestForRenameReport(int)), this, SLOT(slotRequestForRenameReport(int)));

    ui->saveEnabled(false);
    ui->deleteEnabled(false);
}


void ReportEditor::reloadSettings()
{
    m_prevReportNumber = qMin(30, m_core->getSettingValue("CuteReport_ReportEditor/prevReportsLimit", 7).toInt());
    m_prevReports = m_core->getSettingValue("CuteReport_ReportEditor/prevReports").toStringList();
    if (ui)
        ui->reloadSettings();
}


void ReportEditor::saveSettings()
{
    m_core->setSettingValue("CuteReport_ReportEditor/prevReportsLimit", m_prevReportNumber);
    m_core->setSettingValue("CuteReport_ReportEditor/prevReports", m_prevReports);
    if (ui)
        ui->saveSettings();
}


void ReportEditor::sync()
{
    if (core()->currentReport()) {
        ReportIterator i;
        for (i = m_reports.begin(); i != m_reports.end(); ++i) {
            if (i->report == core()->currentReport()) {
                i->reportProperties->saveAll();
                break;
            }
        }
    }
}


QWidget * ReportEditor::view()
{
    return ui;
}


void ReportEditor::activate()
{
}


void ReportEditor::deactivate()
{
    if (core()->currentReport()) {
        ReportIterator i;
        for (i = m_reports.begin(); i != m_reports.end(); ++i) {
            if (i->report == core()->currentReport()) {
                i->reportProperties->saveAll();
                break;
            }
        }
    }
}


QIcon ReportEditor::icon()
{
    return QIcon(":images/report.png");
}


QString ReportEditor::name()
{
    return QString("Reports");
}


void ReportEditor::slotCurrentReportChangedByCore(CuteReport::ReportInterface* report)
{
    if (m_saveReportAction)
        m_saveReportAction->setEnabled(report);
    bool isRendered = core()->reportCore()->isRendered(report);
    ui->saveEnabled(!isRendered);
    ui->deleteEnabled(!isRendered);

#ifndef DEMO_BUILD
    if (m_saveReportAsAction)
        m_saveReportAsAction->setEnabled(report);
    if (m_closeReportAction)
        m_closeReportAction->setEnabled(report);
#endif

    if (!report) {
        core()->setDocumentTitles(documentId, QString());
        return;
    }

    ui->setCurrentTab(report->objectName());
    QString reportName = (report->name().isEmpty() ? "NoName" : report->name()) +  (report->isValid() ? "" : "(invalid)") + (report->isDirty() ? "*" : "");
    core()->setDocumentTitles(documentId, reportName);
}


void ReportEditor::slotCurrentTabChanged(int index)
{
    if (index >= m_reports.size())
        return;

    core()->sync();
    core()->setCurrentReport(m_reports[index].report);
}


//void ReportEditor::slotPrinterChanged(CuteReport::PrinterInterface* printer)
//{
//    CuteReport::ReportInterface * report = dynamic_cast<CuteReport::ReportInterface *>(sender());
//    if (!report)
//        return;

//    ReportIterator i;
//    for (i = m_reports.begin(); i != m_reports.end(); ++i) {
//        if (i->report == report) {
//            i->printerPropertyEditor->setObject(printer);
//            break;
//        }
//    }
//}


//void ReportEditor::slotRendererChanged(CuteReport::RendererInterface* renderer)
//{
//    CuteReport::ReportInterface * report = dynamic_cast<CuteReport::ReportInterface*>(sender());
//    if (!report)
//        return;

//    ReportIterator i;
//    for (i = m_reports.begin(); i != m_reports.end(); ++i) {
//        if (i->report == report) {
//            i->rendererPropertyEditor->setObject(renderer);
//            break;
//        }
//    }
//}


void ReportEditor::slotRequestForNewRenderer(const CuteReport::RendererInterface *renderer)
{
    if (!core()->currentReport())
        return;
    core()->currentReport()->addRenderer( core()->reportCore()->createRendererObject(renderer->moduleFullName(), core()->currentReport()) );
}


void ReportEditor::slotRequestForDeleteRenderer(const QString &name)
{
    if (!core()->currentReport())
        return;

    int ret = QMessageBox::warning(core()->mainWindow(), tr("CuteReport"),
                                   tr("Current selected renderer will be removed.\n"
                                      "Do you want to proceed?"),
                                   QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;

    core()->currentReport()->deleteRenderer(name);
}


void ReportEditor::slotRequestForChangeDefaultRenderer(const QString &name)
{
    if (!core()->currentReport())
        return;
    core()->currentReport()->setDefaultRendererName(name);
}


void ReportEditor::slotRequestForNewRendererName(const QString &name)
{
    bool ok = true;
    QString text = QInputDialog::getText(0, tr("Renderer renaming"),
                                         tr("Renderer name:"), QLineEdit::Normal,
                                         name, &ok);
    if (ok && !text.isEmpty()) {
        RendererInterface * renderer = core()->currentReport()->renderer(name);
        if (renderer)
            renderer->setObjectName(text);
    }
}


void ReportEditor::slotRequestForNewPrinter(const CuteReport::PrinterInterface* printer)
{
    if (!core()->currentReport())
        return;
    core()->currentReport()->addPrinter( core()->reportCore()->createPrinterObject(printer->moduleFullName(), core()->currentReport()) );
}


void ReportEditor::slotRequestForDeletePrinter(const QString &name)
{
    if (!core()->currentReport())
        return;

    int ret = QMessageBox::warning(core()->mainWindow(), tr("CuteReport"),
                                   tr("Current selected printer will be removed.\n"
                                      "Do you want to proceed?"),
                                   QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;

    core()->currentReport()->deletePrinter(name);
}


void ReportEditor::slotRequestForChangeDefaultPrinter(const QString &name)
{
    if (!core()->currentReport())
        return;
    core()->currentReport()->setDefaultPrinterName(name);
}


void ReportEditor::slotRequestForNewPrinterName(const QString &name)
{
    bool ok = true;
    QString text = QInputDialog::getText(0, tr("Printer renaming"),
                                         tr("Printer name:"), QLineEdit::Normal,
                                         name, &ok);
    if (ok && !text.isEmpty()) {
        PrinterInterface * printer = core()->currentReport()->printer(name);
        if (printer)
            printer->setObjectName(text);
    }
}


void ReportEditor::slotRequestToInheritReport()
{
    if (!core()->currentReport())
        return;
    core()->reportCore()->log(CuteReport::LogDebug, "ReportEditor", "slotRequestToInheritReport", "");

    QString baseReportUrl = core()->loadReportDialog();
    core()->reportCore()->inheritReport(core()->currentReport(), baseReportUrl);
}


void ReportEditor::slotRequestForDetachReport()
{
    if (!core()->currentReport())
        return;
    core()->reportCore()->log(CuteReport::LogDebug, "ReportEditor", "slotRequestForDetachReport", "");
    core()->reportCore()->detachReport(core()->currentReport());
}


void ReportEditor::slotRequestForNewStorage(const CuteReport::StorageInterface* storage)
{
    if (!core()->currentReport())
        return;
    core()->currentReport()->addStorage( core()->reportCore()->createStorageObject( storage->moduleFullName(), core()->currentReport()) );
}


void ReportEditor::slotRequestForDeleteStorage(const QString & storageName)
{
    if (!core()->currentReport())
        return;

    int ret = QMessageBox::warning(core()->mainWindow(), tr("CuteReport"),
                                   tr("Current selected storage will be removed.\n"
                                      "Do you want to proceed?"),
                                   QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;

    core()->currentReport()->deleteStorage(storageName);
}


void ReportEditor::slotRequestForChangeDefaultStorage(const QString & storageName)
{
    if (!core()->currentReport())
        return;
    core()->currentReport()->setDefaultStorageName(storageName);
}


void ReportEditor::slotRequestForNewStorageName(const QString &storageName)
{
    bool ok = true;
    QString text = QInputDialog::getText(0, tr("Storage renaming"),
                                         tr("Storage name:"), QLineEdit::Normal,
                                         storageName, &ok);
    if (ok && !text.isEmpty()) {
        StorageInterface * storage = core()->currentReport()->storage(storageName);
        if (storage)
            storage->setObjectName(text);
    }
}


void ReportEditor::slotRequestForTemplate()
{
    QString selectedObjectUrl = core()->loadReportDialog();

    if (selectedObjectUrl.isEmpty())
        return;

    CuteReport::ReportInterface * new_report = core()->reportCore()->loadReport(selectedObjectUrl);

    if (new_report) {
        newReportPreprocess(new_report);
        ui->setCurrentTab(new_report->objectName());
        core()->setCurrentReport(new_report);
    }
}


void ReportEditor::slotRequestForNewReport()
{
    core()->emitNewReportBefore();

    ReportIterator i;
    for (i = m_reports.begin(); i != m_reports.end(); ++i) {
        if (i->report->name().isEmpty()) {
            QMessageBox::warning(core()->reportCore()->rootWidget(), tr("CuteReport"),
                                 tr("There is unnamed report open.\nName this report before opening new one."),
                                 QMessageBox::Ok);
            return;
        }
    }

    CuteReport::ReportInterface * new_report = core()->reportCore()->createReport();

    core()->emitNewReportAfter(new_report);
}


void ReportEditor::loadReport(const QString & objectUrl)
{
    core()->reportCore()->log(CuteReport::LogDebug, "ReportEditor", "loadReport", QString("file: %1").arg(objectUrl));

    core()->emitLoadReportBefore(objectUrl);

    QString selectedObjectUrl = objectUrl;

    if (selectedObjectUrl.isEmpty()) {
        selectedObjectUrl = core()->loadReportDialog();
    }

    CuteReport::ReportInterface * new_report = core()->reportCore()->loadReport(selectedObjectUrl);
    if (!new_report) {
        core()->emitLoadReportAfter(new_report);
        return;
    }

    if (CuteReport::ReportInterface * existsReport = sameReportExists(new_report)) {
        delete new_report;
        m_reportToSwitch = existsReport;
        QTimer::singleShot(0, this, SLOT(slotSetCurrentReport()));
    } else {
        putReportNameOnTop(new_report->fileUrl());
        core()->emitLoadReportAfter(new_report);
    }
}


ReportInterface *ReportEditor::sameReportExists(CuteReport::ReportInterface* newReport)
{
    CuteReport::ReportInterface* existsReport = 0;

    ReportIterator i;
    for (i = m_reports.begin(); i != m_reports.end(); ++i) {
        if (i->report->name() == newReport->name()) {
            existsReport = i->report;
            break;
        }
    }

    if (existsReport) {
        if (newReport->name().isEmpty())
            QMessageBox::warning(core()->reportCore()->rootWidget(), tr("CuteReport"),
                                 tr("Report with an empty name is already open.\nName your current report before opening new one."),
                                 QMessageBox::Ok);
        else
            QMessageBox::warning(core()->reportCore()->rootWidget(), tr("CuteReport"),
                                 tr("Report with name \'%1\' is already open.").arg(newReport->name()),
                                 QMessageBox::Ok);
        return existsReport;
    }

    return 0;
}


void ReportEditor::slotRequestForOpenReport()
{
    loadReport("");
}


void ReportEditor::slotRequestForSaveReport()
{
    saveReport(core()->currentReport(), false);
}


void ReportEditor::slotRequestForSaveReportAs()
{
    saveReport(core()->currentReport(), true);
}


bool ReportEditor::saveReport(CuteReport::ReportInterface * report, bool askFileName)
{
    if (!report)
        return false;

    core()->sync();

    //qDebug() << report->description();

    if (!report->isValid()) {
        QMessageBox::warning(core()->mainWindow(), tr("Cute Report"),
                             tr("The document is invalid and can not be saved."),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    QString objectUrl;

    if (askFileName || report->fileUrl().isEmpty()) {
        objectUrl = core()->saveCurrentReportDialog();
    } else
        objectUrl = report->fileUrl();

    if (objectUrl.isEmpty())
        return false;

    QFileInfo file(objectUrl);
    if(file.suffix().isEmpty())
        objectUrl += ".qtrp";
    QString error;
    bool result = core()->reportCore()->saveReport(objectUrl, report, &error);

    if (!result && !error.isEmpty()) {
        core()->reportCore()->log(CuteReport::LogError, "ReportEditor", error);
        QMessageBox::critical(core()->mainWindow(), tr("Cute Report"),
                              tr("File cannot be saved there."),
                              QMessageBox::Ok, QMessageBox::Ok);
    }

    putReportNameOnTop(objectUrl);

    return result;
}


void ReportEditor::slotRequestForCloseReport()
{
    CuteReport::ReportInterface * report = core()->currentReport();
    if (!report)
        return;

    if (report->isDirty() && report->isValid()) {
        int ret = QMessageBox::warning(core()->mainWindow(), tr("Cute Report"),
                                       tr("The document has been modified.\n"
                                          "Do you want to save your changes?"),
                                       QMessageBox::Save | QMessageBox::Discard
                                       | QMessageBox::Cancel,
                                       QMessageBox::Save);
        switch (ret) {
            case QMessageBox::Save:
                if (!saveReport(core()->currentReport()))
                    return;
                break;
            case QMessageBox::Discard:    break;
            case QMessageBox::Cancel:     return;
            default:                      return;
        }
    }

    core()->emitDeleteReportBefore(report);
    delete report;
//    core()->reportCore()->deleteReport(report);
    report = 0;
    core()->emitDeleteReportAfter(report);

    saveOpenReportList();
}


void ReportEditor::slotReportObjectDestroyed(QObject *report)
{
    int index = -1;
    for (int i = 0; i < m_reports.size(); ++i ) {
        if (m_reports.at(i).report == report) {
            index = i;
            break;
        }
    }

    if (index == -1)
        return;

    ReportStruct s = m_reports.takeAt(index);
    ui->removeTab(index);
//    delete s.printerPropertyEditor;
//    delete s.rendererPropertyEditor;
    delete s.reportProperties;

    index = qMin(index, m_reports.size()-1);

    core()->setCurrentReport( index<0 ? 0 : m_reports.at(index).report );
}


//void ReportEditor::slotRequestForRenameReport(int index)
//{
//    if (index >= m_reports.size())
//        return;

//    CuteReport::ReportInterface* report = m_reports.at(index).report;

//    RenameDialog d(report, report, core()->mainWindow());
//    d.setWindowTitle("Report renaming");
//    if (d.exec() == QDialog::Accepted) {
//        report->setObjectName(d.newName());

//        ui.data()->changeTabText(name, report->objectName());

//        QList>::iterator it = m_reports.find(report);
//        for (i = list.begin(); i != list.end(); ++i)
//            it.value().tabText = report->objectName();
//    }
//}


void ReportEditor::newReportPreprocess(CuteReport::ReportInterface * report)
{
    report->setFlag(ReportInterface::VariablesAutoUpdate, true);
    report->setFlag(ReportInterface::DirtynessAutoUpdate, true);

    report->setObjectName(makeCorrectObjectName(report));

    ReportStruct reportProp;

    reportProp.report = report;
    reportProp.tabText = report->objectName();
//    reportProp.rendererPropertyEditor = core()->createPropertyEditor(ui);
//    reportProp.printerPropertyEditor = core()->createPropertyEditor(ui);
    reportProp.reportProperties = new PropertyEditor::ReportProperties(core(), ui);

//    reportProp.reportProperties->addRendererPropertyEditor(reportProp.rendererPropertyEditor);
//    reportProp.reportProperties->addPrinterPropertyEditor(reportProp.printerPropertyEditor);

    reportProp.reportProperties->connectReport(report);
//    reportProp.rendererPropertyEditor->setObject(report->renderer());
//    reportProp.printerPropertyEditor->setObject(report->printer());
    m_reports.append(reportProp);

    ui->addTab(reportProp.reportProperties, QIcon(":images/report.png"), report->objectName());

//    connect(report, SIGNAL(printerChanged(CuteReport::PrinterInterface*)),
//            this, SLOT(slotPrinterChanged(CuteReport::PrinterInterface*)));
//    connect(report, SIGNAL(rendererChanged(CuteReport::RendererInterface*)),
//            this, SLOT(slotRendererChanged(CuteReport::RendererInterface*)));
    connect(report, SIGNAL(nameChanged(QString)), this, SLOT(slotReportNameChangedOutside(QString)));
    connect(report, SIGNAL(dirtynessChanged(bool)), this, SLOT(slotDirtynessChanged(bool)));
    connect(report, SIGNAL(destroyed(QObject*)), this, SLOT(slotReportObjectDestroyed(QObject*)));

    connect(reportProp.reportProperties, SIGNAL(requestForNewStorage(const CuteReport::StorageInterface*)), this, SLOT(slotRequestForNewStorage(const CuteReport::StorageInterface*)));
    connect(reportProp.reportProperties, SIGNAL(requestForDeleteStorage(QString)), this, SLOT(slotRequestForDeleteStorage(QString)));
    connect(reportProp.reportProperties, SIGNAL(requestForDefaultStorage(QString)), this, SLOT(slotRequestForChangeDefaultStorage(QString)));
    connect(reportProp.reportProperties, SIGNAL(requestForNewStorageName(QString)), this, SLOT(slotRequestForNewStorageName(QString)) );

    connect(reportProp.reportProperties, SIGNAL(requestForNewRenderer(const CuteReport::RendererInterface*)), this, SLOT(slotRequestForNewRenderer(const CuteReport::RendererInterface*)));
    connect(reportProp.reportProperties, SIGNAL(requestForDeleteRenderer(QString)), this, SLOT(slotRequestForDeleteRenderer(QString)));
    connect(reportProp.reportProperties, SIGNAL(requestForDefaultRenderer(QString)), this, SLOT(slotRequestForChangeDefaultRenderer(QString)));
    connect(reportProp.reportProperties, SIGNAL(requestForNewRendererName(QString)), this, SLOT(slotRequestForNewRendererName(QString)) );

    connect(reportProp.reportProperties, SIGNAL(requestForNewPrinter(const CuteReport::PrinterInterface*)), this, SLOT(slotRequestForNewPrinter(const CuteReport::PrinterInterface*)));
    connect(reportProp.reportProperties, SIGNAL(requestForDeletePrinter(QString)), this, SLOT(slotRequestForDeletePrinter(QString)));
    connect(reportProp.reportProperties, SIGNAL(requestForDefaultPrinter(QString)), this, SLOT(slotRequestForChangeDefaultPrinter(QString)));
    connect(reportProp.reportProperties, SIGNAL(requestForNewPrinterName(QString)), this, SLOT(slotRequestForNewPrinterName(QString)) );
    connect(reportProp.reportProperties, SIGNAL(requestToInheritReport()), this, SLOT(slotRequestToInheritReport()));
    connect(reportProp.reportProperties, SIGNAL(requestToDetachBaseReport()), this, SLOT(slotRequestForDetachReport()));

    saveOpenReportList();

    QTimer::singleShot(10, report, SLOT(updateVariables()));
}


QList<CuteDesigner::DesignerMenu*> ReportEditor::mainMenu()
{
    QWidget * parent = core()->mainWindow();

    QList<CuteDesigner::DesignerMenu*> menus;

    CuteDesigner::DesignerMenu * reportMenu = new CuteDesigner::DesignerMenu(parent, "File", 1000, 1000); // very high priority;  very high priority;
    menus.append(reportMenu);

    m_newReportAction = createAction("actionNewReport", "New Report", ":/images/document-new.png", "Ctrl+N", SLOT(slotRequestForNewReport()));
    reportMenu->menu->addAction(m_newReportAction);
    m_openReportAction = createAction("actionOpenReport", "Open Report", ":/images/document-open.png", "Ctrl+O", SLOT(slotRequestForOpenReport()));
    reportMenu->menu->addAction(m_openReportAction);

    m_previousReportsAction = createAction("actionOpenPrevReport", "Recent Reports", ":/images/document-save.png", "", "");
    QMenu * prevOpenMenu = new QMenu("Previous Reports");
    for (int i = 0; i<m_prevReports.size(); ++i) {
        QString url = m_prevReports.at(i);
        prevOpenMenu->addAction(createAction(QString("actionOpenPrevReport_%1").arg(i), url, "", "", SLOT(slotOpenPrevReport())));
    }
    m_previousReportsAction->setMenu(prevOpenMenu);
    reportMenu->menu->addAction(m_previousReportsAction);

    m_saveReportAction = createAction("actionSaveReport", "Save Report", ":/images/document-save.png", "Ctrl+S", SLOT(slotRequestForSaveReport()));
    reportMenu->menu->addAction(m_saveReportAction);
    m_saveReportAsAction = createAction("actionSaveAsReport", "Save Report As...", ":/images/document-save-as.png", "", SLOT(slotRequestForSaveReportAs()));
    reportMenu->menu->addAction(m_saveReportAsAction);
    m_closeReportAction = createAction("actionCloseReport", "Close Report", ":/images/document-close.png", "Ctrl+W", SLOT(slotRequestForCloseReport()));
    reportMenu->menu->addAction(m_closeReportAction);

    m_saveReportAction->setEnabled(false);
    m_closeReportAction->setEnabled(false);
    m_saveReportAsAction->setEnabled(false);


#ifdef DEMO_BUILD
    m_newReportAction->setEnabled(false);
    m_openReportAction->setEnabled(false);
    m_previousReportsAction->setEnabled(false);
    m_saveReportAsAction->setEnabled(false);
    m_closeReportAction->setEnabled(false);
#endif

    return menus;
}


void ReportEditor::slotReportNameChangedOutside(QString name)
{
    Q_UNUSED(name);
    CuteReport::ReportInterface* report = dynamic_cast<CuteReport::ReportInterface*>(sender());

    int index = -1;
    for (int i = 0; i < m_reports.size(); ++i ) {
        if (m_reports.at(i).report == report) {
            index = i;
            break;
        }
    }

    if (index == -1)
        return;

    report->setObjectName(makeCorrectObjectName(report));
    ui.data()->changeTabText(index, report->objectName());

    m_reports.value(index).tabText = report->objectName();

    QString reportName = (report->name().isEmpty() ? "NoName" : report->name()) +  (report->isValid() ? "" : "(invalid)") + (report->isDirty() ? "*" : "");
    core()->setDocumentTitles(documentId, reportName);
}


void ReportEditor::slotDirtynessChanged(bool isDirty)
{
    Q_UNUSED(isDirty);
    CuteReport::ReportInterface* report = dynamic_cast<CuteReport::ReportInterface*>(sender());
    if (report != core()->currentReport())
        return;

    QString reportName = (report->name().isEmpty() ? "NoName" : report->name()) +  (report->isValid() ? "" : "(invalid)") + (report->isDirty() ? "*" : "");
    core()->setDocumentTitles(documentId, reportName);
}


void ReportEditor::slotCoreReportCreated(ReportInterface *report)
{
    if (!report)
        return;

    foreach (ReportStruct st, m_reports) {
        if (st.report == report)
            return;
    }

    if (CuteReport::ReportInterface* existsReport = sameReportExists(report)) {
        delete report;
        m_reportToSwitch = existsReport;
        QTimer::singleShot(0, this, SLOT(slotSetCurrentReport()));
    } else {
        newReportPreprocess(report);
        m_reportToSwitch = report;
        QTimer::singleShot(0, this, SLOT(slotSetCurrentReport()));
    }
}


void ReportEditor::slotSetCurrentReport()
{
    core()->setCurrentReport(m_reportToSwitch.data());
    /// do not set m_reportToSwitch to 0 because of issues of some sequential reports
}


void ReportEditor::slotDesignerInitDone()
{
    // check if argument with report url passed
    QString fileURL;
    QStringList arguments = QCoreApplication::arguments();
    for (int i = 0; i< arguments.count()-1; ++i) {
        QString arg = arguments.at(i);
        if (arg == "-i" || arg == "--input-file" )
            fileURL = arguments.at(i+1);
    }

    if (!fileURL.isEmpty()) {
        loadReport(fileURL);
    } else {
        QStringList reportURLs = core()->getSettingValue("CuteReport_Designer/LastReportURLs").toString().split(";");
        bool ok = true;

#ifndef DEMO_BUILD
        ok = core()->getSettingValue("CuteReport_Designer/LastExitSuccess", true).toBool();

        if (!ok) {
            int ret = QMessageBox::warning(core()->mainWindow(), tr("CuteReport Designer"),
                                           tr("It seems previous time CuteDesigner was accidentally stoped.\n"
                                              "There some reports were edited when it happened and\n"
                                              "if you try to open them again, the application can crash again.\n\n"
                                              "Do you still want to load last edited reports?"),
                                           QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
            if (ret == QMessageBox::Yes)
                ok = true;
        }
#endif

        if (ok) {
            foreach (const QString & url, reportURLs) {
                if (!url.isEmpty())
                    loadReport(url);
            }
        }
    }
}


void ReportEditor::slotAppIsAboutToClose(bool* cancel)
{
    QStringList list;
    foreach (ReportStruct st, m_reports) {
        CuteReport::ReportInterface * report = st.report;
        if ((report->isDirty() || report->fileUrl().isEmpty()) && report->isValid()) {
            int ret = QMessageBox::warning(core()->mainWindow(), tr("Cute Report"),
                                           tr("The document <b>\"%1\"</b> has been modified.<br>"
                                              "Do you want to save your changes?").arg(report->name().isEmpty() ? tr("NoName") : report->name()),
                                           QMessageBox::Yes | QMessageBox::No| QMessageBox::Cancel,
                                           QMessageBox::Yes);
            if (ret == QMessageBox::Cancel) {
                if (cancel) {
                    (*cancel) = true;
                    return;
                }
            } else if (ret == QMessageBox::Yes) {
                bool res = saveReport(report, false);
                if (!res && cancel) {
                    (*cancel) = true;
                    return;
                }
            }
        }
        list << report->fileUrl();
    }
    core()->setSettingValue("CuteReport_Designer/LastReportURLs", list.join(";"));
}


QString ReportEditor::makeCorrectObjectName(CuteReport::ReportInterface * report)
{
    if (report->name().isEmpty())
        return report->objectName();

    QString newObjectName = report->name();
    newObjectName.replace(QRegExp("\\W"), "_");
    newObjectName.replace(QRegExp("_{2,}"),"_");
    return newObjectName;
}


void ReportEditor::putReportNameOnTop(const QString &reportUrl)
{
    if (reportUrl.isEmpty())
        return;

    m_prevReports.removeAll(reportUrl);
    m_prevReports.prepend(reportUrl);
    while (m_prevReports.size() > qMin(30, m_prevReportNumber))
        m_prevReports.removeLast();

    // TODO: update menu on demand
    m_previousReportsAction->menu()->clear();
    QMenu * prevOpenMenu = m_previousReportsAction->menu();

    for (int i = 0; i<m_prevReports.size(); ++i) {
        QString url = m_prevReports.at(i);
        prevOpenMenu->addAction(createAction(QString("actionOpenPrevReport_%1").arg(i), url, "", "", SLOT(slotOpenPrevReport())));
    }

    m_core->setSettingValue("CuteReport_ReportEditor/prevReports", m_prevReports);
}


void ReportEditor::saveOpenReportList()
{
    QStringList list;
    foreach (ReportStruct st, m_reports) {
        CuteReport::ReportInterface * report = st.report;
        list << report->fileUrl();
    }
    core()->setSettingValue("CuteReport_Designer/LastReportURLs", list.join(";"));
}


void ReportEditor::slotOpenPrevReport()
{
    QAction * action = dynamic_cast<QAction*>(sender());
    if (!action)
        return;

    loadReport(action->text());
}


void ReportEditor::slotRendererStarted(ReportInterface *report)
{
    if (report != core()->currentReport())
        return;

    if (m_saveReportAction)
        m_saveReportAction->setEnabled(false);
    ui->saveEnabled(false);
    ui->deleteEnabled(false);

#ifndef DEMO_BUILD
    if (m_saveReportAsAction)
        m_saveReportAsAction->setEnabled(false);
    if (m_closeReportAction)
        m_closeReportAction->setEnabled(false);
#endif
}


void ReportEditor::slotRendererStopped(ReportInterface *report, bool)
{
    if (report != core()->currentReport())
        return;

    if (m_saveReportAction)
        m_saveReportAction->setEnabled(true);
    ui->saveEnabled(true);
    ui->deleteEnabled(true);

#ifndef DEMO_BUILD
    if (m_saveReportAsAction)
        m_saveReportAsAction->setEnabled(true);
    if (m_closeReportAction)
        m_closeReportAction->setEnabled(true);
#endif
}



//suit_end_namespace

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(ReportEditor, ReportEditor)
#endif
