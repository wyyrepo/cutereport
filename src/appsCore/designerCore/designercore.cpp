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
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/
#include "designercore.h"
#include "moduleinterface.h"
#include "reportcore.h"
#include "mainwindow.h"
#include "reportinterface.h"
#include "pageinterface.h"
#include "iteminterface.h"
#include "datasetinterface.h"
#include "stylehelper.h"
#include "storagesettingsdialog.h"
#include "storageinterface.h"
#include "rendererinterface.h"
#include "propertyeditorcore.h"
#include "designeriteminterfaceobject.h"
#include "propertyeditor.h"
#include "stdstoragedialog.h"

#include <QSettings>
#include <QApplication>
#include <QtGui>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>

using namespace CuteReport;

inline void initMyResource() { Q_INIT_RESOURCE(designerCore); }

namespace CuteDesigner {

static const QString MODULENAME = "Designer::Core";

bool modulePriorityLessThan(ModuleInterface * i1, ModuleInterface * i2)
{
    return i1->priority() < i2->priority();
}

Core::Core(QObject *parent) :
    QObject(parent),
    m_mainWindow(0),
    m_currentReport(0),
    m_currentPage(0),
    m_currentDataset(0),
    m_currentForm(0),
    m_settings(0)
{
    init();
}

Core::Core(QSettings *settings, QObject *parent) :
    QObject(parent),
    m_mainWindow(0),
    m_currentReport(0),
    m_currentPage(0),
    m_currentDataset(0),
    m_currentForm(0),
    m_settings(settings)
{
    init();
}


Core::~Core()
{
    qDebug() << "Designer Core DTOR";
    PropertyEditor::Core::refDec();

//    qDeleteAll(m_modules);
//    m_modules.clear();
//    delete m_mainWindow;
//    delete m_designerItemObject;
//    PropertyEditor::Core::refDec();
//    delete m_reportCore;
//    m_settings->setValue("CuteReport_Designer/LastExitSuccess", true);
}


void Core::init()
{
    initMyResource();

    if (!m_settings) {
        static QString orgName = "ExaroLogic";
        static QString appName = "CuteReport";
        m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, orgName, appName, this);
        if (m_settings->value("CuteReport_Designer/VersionMajor").toInt() < 1) {
            QString fileName = m_settings->fileName();
            delete m_settings;
            QFile::remove(fileName);
            m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, orgName, appName, this);
        }
    }

    int dLevel = m_settings->value("CuteReport_Designer/ItemDetailLevel", (int)CuteReport::TemplateDetailLevel_Full).toInt();
    CuteReport::BaseItemInterface::setDefaultTemplateDetailLevel(TemplateItemDetailLevel (dLevel));

    m_reportCore = new ReportCore(m_settings, true);
    m_reportCore->setMaxRenderingThreads(1);

    connect(m_reportCore, SIGNAL(metricUpdated(CuteReport::MetricType,QVariant)), this, SLOT(showMetric(CuteReport::MetricType, QVariant)));
    connect(m_reportCore, SIGNAL(rendererDone(CuteReport::ReportInterface*,bool)), this, SLOT(_rendererDone(CuteReport::ReportInterface*,bool)));

    setSettingValue("CuteReport_Designer/VersionMajor", m_reportCore->versionMajor());
    setSettingValue("CuteReport_Designer/VersionMinor", m_reportCore->versionMinor());

    m_designerItemObject = new DesignerItemInterfaceObject(this);

//    m_propertyManager = new PropertyEditor::PluginManager();
    PropertyEditor::Core::createInstance(false);
    connect(PropertyEditor::Core::instance(), SIGNAL(log(int,QString,QString,QString)), this, SLOT(propertyEditorLog(int,QString,QString,QString)));
    PropertyEditor::Core::instance()->init();
    PropertyEditor::Core::refInc();

    //StyleHelper::setBaseColor(QApplication::palette().color(QPalette::Highlight).darker());
    //StyleHelper::setBaseColor(QColor(165,165,165));
    StyleHelper::setBaseColor(QApplication::palette().color(QPalette::AlternateBase).darker());

//    QTimer::singleShot(0, this, SLOT(slotInit()));
    slotInit();
}

void Core::saveSettings()
{
    m_mainWindow->saveSettings();

    foreach (ModuleInterface * module , m_modules)
        module->saveSettings();

    m_reportCore->storeState();

    //setSettingValue("CuteReport_Designer/LastReportURL", m_currentReport ? m_currentReport->filePath() : "");
//    setSettingValue("CuteReport/PrimaryStorage", m_reportCore->defaultStorageName());
//    setSettingValue("CuteReport/PrimaryRenderer", m_reportCore->defaultRendererName());
//    m_settings->setValue( QString("CuteReport/Storage_%1_options").arg(m_reportCore->defaultStorage()->moduleFullName().replace("::","_")),
//                          m_reportCore->moduleOptionsStr(m_reportCore->defaultStorage()));
//    m_settings->setValue( QString("CuteReport/Renderer_%1_options").arg(m_reportCore->defaultRendererName()->moduleFullName().replace("::","_")),
//                          m_reportCore->moduleOptionsStr(m_reportCore->defaultRendererName()));

//    setSettingValue("CuteReport/PrimaryRenderer", m_reportCore->defaultRendererName()->moduleFullName());
}



//void Core::restoreSettings()
//{
//    m_mainWindow->reloadSettings();
//}


void Core::slotInit()
{
    reinitOptions();
    setupMainWindow();
    initModules();

    foreach (ModuleInterface * module , m_guiModules) {
        m_mainWindow->addTab(module->view(), module->icon(), module->name());
    }

    foreach (ModuleInterface * module , m_modules) {
        //qDebug() << "test: " << module;
        module->reloadSettings();
    }

    m_mainWindow->reloadSettings();

    appendMenus();

//    if (m_reportCore->defaultStorageName().isNull()) {
//        StorageSettingsDialog d(this);
//        if (d.exec() == QDialog::Accepted) {
//            m_reportCore->setDefaultStorage( d.currentStorageName());
//        }
//    }

    m_mainWindow->show();

    foreach (ModuleInterface * module , m_modules) {
        if (!m_guiModules.contains(module))
            module->activate();
    }

    /// check application arguments
    QStringList arguments = QCoreApplication::arguments();

    QString moduleName;
    for (int i = 0; i< arguments.count()-1; ++i) {
        QString arg = arguments.at(i);
        if (arg == "-gm" || arg == "--gui-module-switch" )
            moduleName = arguments.at(i+1).toLower();
    }

//    if (!moduleName.isEmpty()) {
//        foreach (CuteDesigner::ModuleInterface* module, m_guiModules) {
//            QString mName = module->name();
//            if (mName.toLower() == moduleName) {
//                switchToModule(module);
//                break;
//            }
//        }
//    }

    if (!moduleName.isEmpty())
        switchToModule(moduleName);

    emit initDone();

    m_settings->setValue("CuteReport_Designer/LastExitSuccess", false); // in case of crash we will have false here
}


void Core::initModules()
{
    QList<CuteDesigner::ModuleInterface*> pluginList;

    foreach (QObject *plugin, QPluginLoader::staticInstances()) {
        if (plugin && qobject_cast<CuteDesigner::ModuleInterface*>(plugin)) {
            m_reportCore->log(LogDebug, MODULENAME, QString("Found static plugin: %1").arg(plugin->metaObject()->className()), "");
            pluginList.push_back(qobject_cast<CuteDesigner::ModuleInterface*>(plugin));
        }
    }

//    foreach (ModuleInterface * module , m_modules) {
//        qDebug() << "test: " << module;
//    }

    QFileInfoList files;
    QStringList dirs;
    dirs << REPORT_DESIGNER_PLUGINS_PATH;

    QDir dir;
    foreach (const QString & dirStr, dirs) {
        dir.setPath(QDir::isRelativePath(dirStr) ? QCoreApplication::applicationDirPath() + "/" + dirStr : dirStr);
        m_reportCore->log(CuteReport::LogDebug, MODULENAME, "Raw Plugin dir: " + dirStr );
        m_reportCore->log(CuteReport::LogDebug, MODULENAME, "Designer plugin dir: " + dir.absolutePath() );
        files += dir.entryInfoList(QStringList() << "*.so" << "*.dll" << "*.dylib", QDir::Files);
    }
    QPluginLoader loader;
    //loader.setLoadHints(QLibrary::ResolveAllSymbolsHint|QLibrary::ExportExternalSymbolsHint);

    foreach(const QFileInfo & fileName, files) {
        m_reportCore->log(CuteReport::LogDebug, MODULENAME, "Plugin loading: " + fileName.fileName());
        loader.setFileName(fileName.absoluteFilePath());
        if (!loader.load()) {
            m_reportCore->log(LogWarning, MODULENAME, QString("Error while loading plugin \'%1\': %2 ").arg(fileName.fileName()).arg(loader.errorString()));
            continue;
        }
        CuteDesigner::ModuleInterface* plugin = qobject_cast<CuteDesigner::ModuleInterface*>(loader.instance());
        if (plugin) {
            pluginList.push_back(plugin);
        } else {
            m_reportCore->log(LogWarning, MODULENAME, QString("Error while creating instance of plugin \'%1\': %2 ").arg(fileName.fileName()).arg(loader.errorString()));
            loader.unload();
        }
    }

    foreach(CuteDesigner::ModuleInterface* plugin, pluginList) {
        plugin->setParent(this);
        plugin->init(this);
        CuteDesigner::ModuleInterface* module = qobject_cast<CuteDesigner::ModuleInterface*>(plugin);
        if (module) {
            if (module->view())
                m_guiModules.append(module);
            m_modules.append(module);
        }
    }

    qSort(m_guiModules.begin(), m_guiModules.end(), modulePriorityLessThan);

    m_currentModuleIndex = -1;
    m_prevModuleIndex = -1;
}


void Core::setupMainWindow()
{
    m_mainWindow = new MainWindow(this);
    m_mainWindow->setWindowTitle("CuteReport");

    // need queued since it will be deleted within MainWindow Method
    connect(m_mainWindow, SIGNAL(closeRequest()), this, SLOT(slotMainWindowCloseRequest()), Qt::QueuedConnection);
    connect(m_mainWindow, SIGNAL(stdActionTriggered(CuteDesigner::Core::StdAction,QAction*)),
            this, SLOT(slotStdActionTriggered(CuteDesigner::Core::StdAction,QAction*)));
    connect(m_mainWindow, SIGNAL(optionsDialogClosed()), this, SLOT(reinitOptions()));
}


MainWindow * Core::mainWindow()
{
    return m_mainWindow;
}


void Core::appendMenus()
{
    if (!m_mainWindow)
        return;

    QMenuBar * menubar = m_mainWindow->menuBar ();

    QHash<int, DesignerMenu*> mainMenu;
    QHash<QString, DesignerMenu*> menuTitles;
    QList<DesignerMenu*> deleteList;

    // add original QMainMenu actions
    int priority = menubar->actions().count() * 100;
    foreach(QAction * action, menubar->actions()) {
        DesignerMenu * menu = new DesignerMenu();
        menu->menu = action->menu();
        menu->mainPriority = priority;
        menu->subPriority = 100;
        //        menubar->removeAction(action);
        menuTitles.insertMulti(action->text(), menu);
        priority -= 100;
    }

    menubar->clear();

    //    // add Modules' actions
    //    foreach (ModuleInterface * module , m_guiModules) {
    //        QList<CuteDesigner::DesignerMenu*> menus = module->mainMenu();
    //        foreach (CuteDesigner::DesignerMenu * menu,  menus) {
    //            menuTitles.insertMulti(menu->menu->title(), menu);
    //        }
    //    }
    foreach (ModuleInterface * module , m_modules) {
        QList<CuteDesigner::DesignerMenu*> menus = module->mainMenu();
        foreach (CuteDesigner::DesignerMenu * menu,  menus) {
            menuTitles.insertMulti(menu->menu->title(), menu);
        }
    }

    // menu concatenating for the same mainMenu title
    foreach(QString title, menuTitles.keys()) {
        QList<DesignerMenu*> list = menuTitles.values(title);
        if (list.count() > 1) {
            QHash<int, DesignerMenu*> subMenu;
            int priority = list[0]->mainPriority;
            for (int i = 0; i<list.count(); ++i) {
                if (list[i]->mainPriority > priority)
                    priority = list[i]->mainPriority;
                subMenu.insert(list[i]->subPriority, list[i]);
            }

            QList<int> subMenuPriorities = subMenu.uniqueKeys();
            qSort(subMenuPriorities.begin(), subMenuPriorities.end(), qGreater<int>());

            DesignerMenu * commonSubMenu = new DesignerMenu();
            commonSubMenu->menu = new QMenu(m_mainWindow);
            commonSubMenu->menu->setTitle(title);

            deleteList.append(commonSubMenu);

            foreach (int priority, subMenuPriorities) {
                QList<DesignerMenu*> subMenus = subMenu.values(priority);
                foreach(DesignerMenu* subMenu, subMenus)
                    commonSubMenu->menu->addActions(subMenu->menu->actions());
                if (priority != subMenuPriorities.last())
                    commonSubMenu->menu->addSeparator();
            }

            mainMenu.insert(priority, commonSubMenu);

        } else {
            DesignerMenu* m = list.takeFirst();
            mainMenu.insert(m->mainPriority, m);
        }
    }


    QList<int> mainMenuPriorities = mainMenu.uniqueKeys();
    qSort(mainMenuPriorities.begin(), mainMenuPriorities.end(), qGreater<int>());


    foreach (int priority, mainMenuPriorities) {
        QList<DesignerMenu*> menus = mainMenu.values(priority);
        foreach (DesignerMenu* menu, menus) {
            menubar->addMenu(menu->menu);
        }
    }

    // cleaning up
    foreach (DesignerMenu * menu, deleteList)
        delete menu;
    foreach (DesignerMenu * menu, menuTitles)
        delete menu;
}



void Core::setSettingValue(QString key, QVariant value)
{
    m_settings->setValue(key, value);
}


QVariant Core::getSettingValue(QString key, QVariant defaultValue)
{
    return m_settings->value(key, defaultValue);
}


QSettings * Core::settings()
{
    return m_settings;
}


void Core::switchToModule(ModuleInterface * module)
{
    int index = m_guiModules.indexOf(module);
    if (index >=0)
        m_mainWindow->switchToTab(index);
}


void Core::switchToModule(const QString &moduleName)
{
    if (!moduleName.isEmpty()) {
        QString name = moduleName.toLower();
        foreach (CuteDesigner::ModuleInterface* module, m_guiModules) {
            QString mName = module->name().toLower();
            if (mName == name) {
                switchToModule(module);
                break;
            }
        }
    }
}



void Core::sync()
{
    if (m_currentReport)
        foreach (ModuleInterface* module, m_guiModules)
            module->sync();
}


void Core::addToStatusBar(QLabel * label)
{
    m_mainWindow->addToStatusBar(label);
}


void Core::setDocumentTitles(int id, const QString & name)
{
    if (name.isEmpty())
        m_documentTitles.remove(id);
    else
        m_documentTitles.insert(id, name);

    QStringList list;
    QMap<int, QString>::iterator i;
    for (i = m_documentTitles.begin(); i != m_documentTitles.end(); ++i)
        list.append(i.value());
    list.append("Cute Report");

    m_mainWindow->setWindowTitle(list.join(" - "));
}


void Core::showMetric(MetricType type, const QVariant &value)
{
    m_mainWindow->showMetric(type, value);
}


CuteReport::ReportInterface* Core::currentReport() const
{
    return m_currentReport;
}


void Core::setCurrentReport(CuteReport::ReportInterface * report)
{
    if (m_currentReport == report)
        return;

    m_reportCore->log(CuteReport::LogDebug, MODULENAME, tr("setCurrentReport: %1").arg(report ? report->objectName() : tr("not defined")));
    m_currentReport = report;

    emit currentReportChanged(m_currentReport);
}


void Core::setCurrentPage(CuteReport::PageInterface * page)
{
    if ( m_currentPage == page)
        return;

    m_currentPage = page;
    emit currentPageChanged(m_currentPage);
}


CuteReport::PageInterface* Core::currentPage()
{
    return m_currentPage;
}


void Core::setCurrentDataset(CuteReport::DatasetInterface* dataset)
{
    if (m_currentDataset == dataset)
        return;

    m_currentDataset = dataset;
    emit currentDatasetChanged(m_currentDataset);
}


CuteReport::DatasetInterface* Core::currentDataset()
{
    return m_currentDataset;
}


void Core::setCurrentForm(CuteReport::FormInterface * form)
{
    if (form == m_currentForm)
        return;

    m_currentForm = form;

}


CuteReport::FormInterface* Core::currentForm()
{
    return m_currentForm;
}


bool Core::newPage()
{
    return true;
}


void Core::render()
{
    if (!m_currentReport)
        return;

    m_reportCore->log(CuteReport::LogDebug, MODULENAME, "render()");

    sync();

    m_reportCore->stopRendering(m_currentReport);
    if (m_reportCore->render(m_currentReport)) {
        emit renderingStarted();
    }

}


void Core::stopRenderer()
{
    m_reportCore->log(CuteReport::LogDebug, MODULENAME, "stopRender()");
    int count = m_reportCore->modules(RendererModule).count();
    if (!count) {
        Q_ASSERT(0);
    } else if (count == 1) {
        m_reportCore->stopRendering(m_currentReport);
    } else {
        // TODO: dialog with choosing renderer if more then 1
    }
}

void Core::moduleSwitched(int index)
{
    //    m_reportCore->log(CuteReport::LogDebug, MODULENAME, QString("moduleSwitched index: %1").arg(index));

    if (m_guiModules.isEmpty())
        return;

    if (m_currentModuleIndex >=0)
        m_guiModules.at(m_currentModuleIndex)->deactivate();
    m_guiModules.at(index)->activate();
    if (m_mainWindow)
        m_mainWindow->setStdActions(m_guiModules.at(index)->stdActions());

    m_currentModuleIndex = index;
}


void Core::_newPeportPage()
{
    // TODO: choose correct module name instead of taking any first one
    QStringList moduleNames = m_reportCore->moduleNames(PageModule);
    if (moduleNames.isEmpty())
        return;
    CuteReport::PageInterface * newPage = m_reportCore->createPageObject(moduleNames.at(0), m_currentReport);
    if (!newPage) {
        m_reportCore->log(LogWarning, MODULENAME, "There is no page plugin in the report core");
        return;
    }

    //    /// adding new manipulator if needed
    //    if (!m_pageManipulators.contains(newPage->pageManupilatorID())) {
    //        CuteReport::PageManipulatorInterface * newManipulator = newPage->createManupulator(this);
    //        Q_ASSERT(newPage->pageManupilatorID() == newManipulator->pageManupilatorId());
    //        m_pageManipulators.insert(newPage->pageManupilatorID(), newManipulator);
    //    }

    m_currentReport->addPage(newPage);

    emit newPage_after(newPage);

    //    changeCurrentPage(newPage);
}


void Core::_afterItemRemoved(BaseItemInterface *item)
{
    Q_UNUSED(item)
    emit activeObjectChanged(m_currentPage);
}


void Core::slotMainWindowCloseRequest()
{
    sync();

    bool cancelled = false;
    emit appIsAboutToClose(&cancelled);
    if (cancelled)
        return;

    foreach (ModuleInterface * module , m_modules)
        module->deactivate();

    saveSettings();

    m_mainWindow->setEnabled(false);

    foreach (ModuleInterface * module , m_modules)
        module->beforeDelete();


    qDeleteAll(m_modules);
    m_modules.clear();
    delete m_mainWindow;
    m_mainWindow = 0;
    delete m_designerItemObject;
    m_designerItemObject = 0;
    delete m_reportCore;
    m_reportCore = 0;
    m_settings->setValue("CuteReport_Designer/LastExitSuccess", true);

    emit closed();
}


void Core::propertyEditorLog(int logLevel, const QString &module, const QString &shortMessage, const QString &fullMessage)
{
    ReportCore::log(CuteReport::LogLevel(logLevel), module, shortMessage, fullMessage);
}


void Core::_rendererDone(ReportInterface * report, bool successful)
{
    Q_UNUSED(report)

    if (!successful) {
        QMessageBox::critical(m_mainWindow, tr("CuteReport Designer"),
                              tr("Errors found while generating report.\n"
                                 "Press log button on the bottom left corner for detailed information."));
    }
}


void Core::slotStdActionTriggered(Core::StdAction actionType, QAction * action)
{
    if (m_currentModuleIndex == -1)
        return;
    m_guiModules.at(m_currentModuleIndex)->stdActionTriggered(actionType, action);
}


void Core::reinitOptions()
{
    m_loadMethod = getSettingValue("CuteReport/ReportLoadingMethod", 1).toInt();
}


DesignerItemInterface *Core::designerItemInterface() const
{
    return m_designerItemObject;
}



QList<CuteDesigner::ModuleInterface *> Core::guiModules() const
{
    return m_guiModules;
}


PropertyEditor::EditorWidget * Core::createPropertyEditor(QWidget * parent)
{
    return new PropertyEditor::EditorWidget(parent);
}


QString Core::loadObjectDialog(const QStringList &fileExtensions, const QString & dialogTitle)
{
    QString title = dialogTitle.isEmpty() ? tr("Load object") : dialogTitle;
    QString lastURL = getSettingValue("CuteReport_ReportEditor/lastURL").toString();

     /// TODO: storage method is not implemented - we need to set file extension automatically
//    if (m_loadMethod == 0) {
//        CuteReport::StdStorageDialog d(reportCore(), mainWindow(), title);
//        d.setUrl(fileURL.isEmpty() ? lastURL : fileURL);
//        if( d.exec() != QDialog::Accepted)
//            return QString();
//        fileURL = d.currentObjectUrl();
//    } else {
        QStringList fileExtensionsFormatted;
        if (fileExtensions.isEmpty())
            fileExtensionsFormatted << "*";
        else {
            foreach (const QString & fileExt, fileExtensions) {
                fileExtensionsFormatted << QString("*.%1").arg(fileExt);
            }
        }
        QString fileFilter = QString("CuteReport object (%1)").arg(fileExtensionsFormatted.join(" "));

        QFileInfo fi(lastURL.section(":",1,1));
        QString fileURL = QFileDialog::getOpenFileName(reportCore()->rootWidget(), title,  fi.absolutePath(), fileFilter, 0,
                                               QFileDialog::Options(m_loadMethod == 3 ? QFileDialog::DontUseNativeDialog : 0));
        if (!fileURL.isEmpty())
            fileURL = "file:" + fileURL;
//    }

    if (!fileURL.isEmpty())
        setSettingValue("CuteReport_ReportEditor/lastURL", fileURL);

    return fileURL;
}


QString Core::saveObjectDialog(const QStringList &fileExtensions, const QString &dialogTitle)
{
    if (!m_currentReport)
        return QString();

    QString title = dialogTitle.isEmpty() ? tr("Save Object") : dialogTitle;
    QString lastURL = getSettingValue("CuteReport_ReportEditor/lastURL").toString();
    QString fileURL;

    /// TODO: storage method is not implemented - we need to set file extension automatically
    //    if (m_loadMethod == 0) {
    //        CuteReport::StdStorageDialog d(reportCore(), mainWindow(), title);
    //        d.setUrl(lastURL);
    //        if (!d.exec())
    //            return QString();
    //        fileURL = d.currentObjectUrl();
    //    } else {
    QFileInfo fi(lastURL.section(":",1,1));
    QStringList fileExtensionsFormatted;
    if (fileExtensions.isEmpty())
        fileExtensionsFormatted << "*.*";
    else {
        foreach (const QString & fileExt, fileExtensions) {
            fileExtensionsFormatted << QString("*.%1").arg(fileExt);
        }
    }
    QString fileFilter = QString("CuteReport object (*.%1)").arg(fileExtensionsFormatted.join(" "));

    fileURL = QFileDialog::getSaveFileName(reportCore()->rootWidget(), title, fi.absolutePath(), QString("CuteReport object (%1)").arg(fileFilter), 0,
                                           QFileDialog::Options(m_loadMethod == 3 ? QFileDialog::DontUseNativeDialog : 0));
    //    }

    if (!fileURL.isEmpty())
        setSettingValue("CuteReport_ReportEditor/lastURL", fileURL);
    return fileURL;
}


QString Core::loadReportDialog()
{
    m_reportCore->log(CuteReport::LogDebug, "Designer::Core", "loadReportDialog");

    QString title = "Load Report";
    QString lastURL = getSettingValue("CuteReport_ReportEditor/lastURL").toString();
    QString fileURL = m_currentReport ? m_currentReport->fileUrl() : "";

    if (m_loadMethod == 0) {
        m_reportCore->log(CuteReport::LogDebug, "Designer::Core", "loadReportDialog", "loadMethod = 0");
        CuteReport::StdStorageDialog d(reportCore(), mainWindow(), title);
        d.setUrl(fileURL.isEmpty() ? lastURL : fileURL);
        if (!d.exec())
            return QString();
        fileURL = d.currentObjectUrl();
    } else {
        m_reportCore->log(CuteReport::LogDebug, "Designer::Core", "loadReportDialog", "loadMethod != 0");
        QFileInfo fInfo(lastURL.section(":",1,1));
        fileURL = QFileDialog::getOpenFileName(reportCore()->rootWidget(), title, fInfo.absoluteDir().absolutePath(),
                                               tr("CuteReport templates (*.qtrp)"), 0, QFileDialog::Options(m_loadMethod == 3 ? QFileDialog::DontUseNativeDialog : 0));
        if (!fileURL.isEmpty())
            fileURL = "file:" + fileURL;
        m_reportCore->log(CuteReport::LogDebug, "Designer::Core", "loadReportDialog", QString("selected file: \'%1\'')").arg(fileURL));
    }

    if (!fileURL.isEmpty())
        setSettingValue("CuteReport_ReportEditor/lastURL", fileURL);
    return fileURL;
}


QString Core::saveCurrentReportDialog()
{
    if (!m_currentReport)
        return QString();

     QString title = tr("Save Report");
     QString lastURL = getSettingValue("CuteReport_ReportEditor/lastURL").toString();
     QString fileURL = m_currentReport->fileUrl();

    if (m_loadMethod == 0) {
        CuteReport::StdStorageDialog d(reportCore(), mainWindow(), title);
        d.setUrl(fileURL.isEmpty() ? lastURL : fileURL);
        d.setObjectName(m_currentReport->fileUrl());
    //        d.setUrlHint("reports");
    //        d.setObjectHint(report->fileUrl());
        if (!d.exec())
            return QString();
        fileURL = d.currentObjectUrl();
    } else {
        fileURL = QFileDialog::getSaveFileName(reportCore()->rootWidget(), title, fileURL.isEmpty() ? lastURL : fileURL, QString(), 0,
                                               QFileDialog::Options(m_loadMethod == 3 ? QFileDialog::DontUseNativeDialog : 0));
    }

    if (!fileURL.isEmpty())
        setSettingValue("CuteReport_ReportEditor/lastURL", fileURL);
    return fileURL;
}


void Core::emitNewReportBefore()
{
    sync();
    emit newReport_before();
}


void Core::emitNewReportAfter(ReportInterface *report)
{
    emit newReport_after(report);
}


void Core::emitLoadReportBefore(QString url)
{
    sync();
    emit loadReport_before(url);
}


void Core::emitLoadReportAfter(ReportInterface *report)
{
    emit loadReport_after(report);
}


void Core::emitDeleteReportBefore(ReportInterface *report)
{
    emit deleteReport_before(report);
}


void Core::emitDeleteReportAfter(ReportInterface *report)
{
    emit deleteReport_after(report);
}


QList<CuteDesigner::ModuleInterface *> Core::modules() const
{
    return m_modules;
}


} //namespaca

