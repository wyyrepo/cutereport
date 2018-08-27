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
#include "reportcore.h"
#include "baseiteminterface.h"
#include "bandinterface.h"
#include "pageinterface.h"
#include "datasetinterface.h"
#include "reportplugininterface.h"
#include "reportinterface.h"
#include "storageinterface.h"
#include "rendererinterface.h"
#include "printerinterface.h"
#include "forminterface.h"
#include "serializerinterface.h"
#include "importinterface.h"
#include "exportinterface.h"
#include "cutereport_types.h"
#include "log/log.h"
#include "renderedreportinterface.h"
#include "scriptextensioninterface.h"
#include "designeriteminterface.h"
#include "reportcoreconfigcleanup.h"
#include "inheritancedatacontainer.h"


#include <QApplication>
#include <QtCore>
#include <QUrl>
#include <QtXml>
#include <QDesktopWidget>
#include <iostream>

void * gDesigner = 0;

static const QString MODULENAME = "ReportCore";
static const int MaxRenderingThreads = 100;


static const char * ModuleTypeStr[] = {QT_TR_NOOP("Unknown"), QT_TR_NOOP("Item"), QT_TR_NOOP("Page"), QT_TR_NOOP("Dataset"),
                                       QT_TR_NOOP("Storage"), QT_TR_NOOP("Renderer"), QT_TR_NOOP("Printer"), QT_TR_NOOP("Form"),
                                       QT_TR_NOOP("Import"), QT_TR_NOOP("Export"), QT_TR_NOOP("Serializer"), QT_TR_NOOP("ScriptExtension")
                                      };

inline void initMyResource() { Q_INIT_RESOURCE(core_resources); }

namespace CuteReport
{


int                                 ReportCore::m_refCount = 0;
QList<ReportPluginInterface*> *     ReportCore::m_itemPlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_pagePlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_datasetPlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_storagePlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_serializePlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_rendererPlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_printerPlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_formPlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_importPlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_exportPlugins = 0;
QList<ReportPluginInterface*> *     ReportCore::m_scriptExtensionPlugins = 0;


QueueReport::QueueReport()
    : report(0), success(true)
{

}

QueueReport::~QueueReport()
{
    if (!url.isEmpty()) report->deleteLater();
}


ReportCore::ReportCore(QObject *parent, QSettings * settings, bool interactive, bool initLogSytem) :
    QObject(parent),
    m_interactive(interactive)
{
    init(settings, initLogSytem);
}


ReportCore::ReportCore(QSettings *settings, bool interactive) :
    QObject(0),
    m_interactive(interactive)
{
    init(settings, true);
}


ReportCore::~ReportCore()
{

    --m_refCount;

    qDeleteAll(m_serializers);
    qDeleteAll(m_storages);
    qDeleteAll(m_printers);
    qDeleteAll(m_renderers);
    qDeleteAll(m_exporters);

    if (m_refCount == 0) {
        qDeleteAll(*m_itemPlugins);
        delete m_itemPlugins;
        qDeleteAll(*m_pagePlugins);
        delete m_pagePlugins;
        qDeleteAll(*m_datasetPlugins);
        delete m_datasetPlugins;
        qDeleteAll(*m_storagePlugins);
        delete m_storagePlugins;
        qDeleteAll(*m_serializePlugins);
        delete m_serializePlugins;
        qDeleteAll(*m_rendererPlugins);
        delete m_rendererPlugins;
        qDeleteAll(*m_printerPlugins);
        delete m_printerPlugins;
        qDeleteAll(*m_formPlugins);
        delete m_formPlugins;
        qDeleteAll(*m_importPlugins);
        delete m_importPlugins;
        qDeleteAll(*m_exportPlugins);
        delete m_exportPlugins;
        qDeleteAll(*m_scriptExtensionPlugins);
        delete m_scriptExtensionPlugins;
    }

    log(LogDebug, MODULENAME, "DTOR");

    if (Log::instance())
        Log::refCounterDec(this);

    m_settings->setValue("CuteReport/LastExitSuccess", true);
}


int ReportCore::versionMajor() const
{
    return QString(REPORT_VERSION).section('.', 0, 0).toInt();
}


int ReportCore::versionMinor() const
{
    return QString(REPORT_VERSION).section('.', 1, 1).toInt();
}

QString ReportCore::license() const
{
    return QString(REPORT_LICENSE);
}


bool ReportCore::interactive() const
{
    return m_interactive;
}


void ReportCore::finish()
{
    //TODO: implement syncing Storage modules with remote data
    emit finished(true);
}


void ReportCore::restoreState(bool cleanRestore)
{
    log(LogDebug, MODULENAME, "restoreState", QString("cleanRestore: %1").arg(cleanRestore?"true":"false"));

    QString defStorage = defaultStorageName();
    QString defRenderer = defaultRendererName();
    QString defPrinter = defaultPrinterName();

    if (cleanRestore) {
        loadObjectsOptions("Storage", m_storages, m_storagePlugins);
        if (m_storages.isEmpty())
            createLocalObjects(m_storages, m_storagePlugins);
        loadObjectsOptions("Renderer", m_renderers, m_rendererPlugins);
        if (m_renderers.isEmpty())
            createLocalObjects(m_renderers, m_rendererPlugins);
        loadObjectsOptions("Printer", m_printers, m_printerPlugins);
        if (m_printers.isEmpty())
            createLocalObjects(m_printers, m_printerPlugins);
    } else {
        QList<ReportPluginInterface*> list;
        loadObjectsOptions("Storage", list, m_storagePlugins);
        foreach (ReportPluginInterface* object, list)
            setStorage(static_cast<StorageInterface*>(object));
        list.clear();

        loadObjectsOptions("Renderer", list, m_rendererPlugins);
        foreach (ReportPluginInterface* object, list)
            setRenderer(static_cast<RendererInterface*>(object));
        list.clear();

        loadObjectsOptions("Printer", list, m_printerPlugins);
        foreach (ReportPluginInterface* object, list)
            setPrinter(static_cast<PrinterInterface*>(object));
        list.clear();
    }

    setDefaultStorage( m_settings->value("CuteReport/DefaultStorage", defStorage).toString());
    setDefaultRenderer( m_settings->value("CuteReport/DefaultRenderer", defRenderer).toString());
    setDefaultPrinter( m_settings->value("CuteReport/DefaultPrinter", defPrinter).toString());
}


void ReportCore::storeState()
{
    log(LogDebug, MODULENAME, "storeState");

    saveObjectsOptions("Storage", m_storages);
    saveObjectsOptions("Renderer", m_renderers);
    saveObjectsOptions("Printer", m_printers);
    m_settings->setValue("CuteReport/DefaultStorage", defaultStorageName());
    m_settings->setValue("CuteReport/DefaultRenderer", defaultRendererName());
    m_settings->setValue("CuteReport/DefaultPrinter", defaultPrinterName());
}


void ReportCore::saveObjectsOptions(const QString & prefix, const QList<ReportPluginInterface*> & objects, bool clearPrefix)
{
    if (clearPrefix) {
        m_settings->beginGroup("CuteReport");
        QStringList keys = m_settings->childKeys();
        QString mask = QString("%1Options_").arg(prefix);
        foreach(const QString & key, keys) {
            if (key.startsWith(mask))
                m_settings->remove(key);
        }
        m_settings->endGroup();
    }
    foreach (ReportPluginInterface * object, objects) {
        QString objectName = object->objectName();
        m_settings->setValue(QString("CuteReport/%1Options_%2").arg(prefix, objectName), moduleOptionsStr(object));
    }
}


void ReportCore::loadObjectsOptions(const QString & prefix, QList<ReportPluginInterface*> & objects,  const QList<ReportPluginInterface*> * plugins)
{
    foreach (ReportPluginInterface* m, objects)
        if (!m->moduleFlags().testFlag(ReportPluginInterface::Unremovable))
            delete m;

    objects.clear();
    m_settings->beginGroup("CuteReport");
    QStringList keys = m_settings->childKeys();
    QString mask = QString("%1Options_").arg(prefix);
    foreach(const QString & key, keys) {
        if (key.startsWith(mask)) {
            QString fullOptions = m_settings->value(key).toString().trimmed();
            if (fullOptions.isEmpty())
                continue;
            QString delimiter = fullOptions.at(0);
            QStringList list = fullOptions.mid(1).split(delimiter);
            if (!list.size())
                continue;
            QString moduleName = list.takeFirst();
            ReportPluginInterface* newObject = 0;
            foreach (ReportPluginInterface* plugin, *plugins) {
                if (plugin->moduleFullName() == moduleName) {
                    newObject = plugin->createInstance(this);
                    if (newObject) {
                        newObject->setReportCore(this);
                        setModuleOptionsStr(newObject, fullOptions, true);
                    }
                    else
                        log(LogError, MODULENAME, "Plugin returns empty instance", QString("Plugin name is \'%1\'").arg(moduleName));
                    break;
                }
            }
            if (newObject) {
                replaceObject(newObject, objects, QPointer<ReportPluginInterface>());
            } else {
                log(LogError, MODULENAME, "Restoring Module not found", QString("Requestred module name is \'%1\'").arg(moduleName));
            }
        }
    }
    m_settings->endGroup();
}


bool ReportCore::removeObject(const QString &objectName, QList<ReportPluginInterface *> &objects)
{
    ReportPluginInterface * curObject = 0;
    foreach (ReportPluginInterface * object, objects) {
        if (object->objectName() == objectName) {
            curObject = object;
        }
    }

    if (!curObject)
        return false;

    objects.removeOne(curObject);
    delete curObject;
    return true;
}


bool ReportCore::replaceObject(ReportPluginInterface* object, QList<ReportPluginInterface *> &objects, QPointer<ReportPluginInterface> defaultObject)
{
    if (!object)
        return false;
    bool isDefault = defaultObject && (defaultObject->objectName() == object->objectName());
    removeObject(object->objectName(), objects);
    object->setParent(this);
    objects.append(object);
    if (isDefault)
        defaultObject = object;
    return true;
}


QStringList ReportCore::objectNames(const QList<ReportPluginInterface *> &objects) const
{
    QSet<QString> list;
    foreach (CuteReport::ReportPluginInterface * object, objects)
        list << object->objectName();
    return list.toList();
}


ReportPluginInterface *ReportCore::getObjectByName(const QString &objectName, const QList<ReportPluginInterface *> &list) const
{
    foreach (ReportPluginInterface * module, list) {
        if (module->objectName() == objectName) {
            return module;
        }
    }
    return 0;
}


void ReportCore::createLocalObjects(QList<ReportPluginInterface *> &objects, const QList<ReportPluginInterface *> *plugins)
{
    foreach (ReportPluginInterface * plugin, *plugins) {
        ReportPluginInterface * object = plugin->createInstance(this);
        object->setReportCore(this);
        object->setParent(this);
        QString name = plugin->objectNameHint().isEmpty() ? QString(plugin->metaObject()->className()).toLower() : plugin->objectNameHint();
        QString newName = uniqueName(object, name, this);
        object->setObjectName(newName);
        objects.append(object);
    }
}


ReportPluginInterface *ReportCore::findDefaultObject(const QString & type, const QString &fullModuleName,  QList<ReportPluginInterface *> &objects)
{
    ReportPluginInterface * object = getOriginalModuleByName(m_settings->value(QString("CuteReport/Default%1").arg(type)).toString(), objects);
    if (!object)
        object = getOriginalModuleByName(fullModuleName, objects);
    if (!object) {
        QString shorName = fullModuleName.section("::", 1, 1);
        object = getOriginalModuleByName(shorName, objects);
    } else { // try to find of some other module extends current one
        QString fnLower = fullModuleName.toLower();
        foreach(ReportPluginInterface * obj, objects) {
            QStringList extendsList = obj->extendsModules();
            foreach(const QString & name, extendsList)
                if (name.toLower() == fnLower) {
                    return obj;
                }
        }
    }
    return object;
}


bool ReportCore::checkReportPointer(CuteReport::ReportInterface * report, QString * errorText) const
{
    Q_UNUSED(errorText)

    try {
        report->isValid();
    }
    catch (...)
    {
        return false;
    }
    return true;
}


//CuteReport::StorageInterface * ReportCore::getStorageModule(const QString & moduleName, QString * errorText) const
//{
//    CuteReport::StorageInterface * module = moduleName.isEmpty() ? m_defaultStorage : storageModule(moduleName);

//    if (!module) {
//        QString error = tr("Storage Module name \'%1\' not found").arg(module->moduleFullName());
//        if (errorText)
//            *errorText = error;
//        log(LogWarning, MODULENAME, error);
//        return 0;
//    }
//    return module;
//}


//CuteReport::RendererInterface * ReportCore::getRenderer(const CuteReport::ReportInterface * report) const
//{
//    CuteReport::RendererInterface * module = report->renderer();
//    if (!module)
//        module = static_cast<CuteReport::RendererInterface*>(m_defaultRenderer.data());
//    return module;
//}


QSettings *ReportCore::settings()
{
    return m_settings;
}


QString ReportCore::resourcesPath() const
{ 
    QString path = m_settings->value("CuteReport/ResourcesPath").toString();
    if (path.isEmpty())
        path = REPORT_RESOURCES_PATH;

    if (QDir::isRelativePath(path))
        path = QCoreApplication::applicationDirPath() + "/" + path;

    return QDir::cleanPath(path);
}


QString ReportCore::imagesPath() const
{
    QString path = m_settings->value("CuteReport/ImagesPath").toString();
    if (path.isEmpty())
        path = REPORT_IMAGES_PATH;

    if (QDir::isRelativePath(path))
        path = QCoreApplication::applicationDirPath() + "/" + path;

    return QDir::cleanPath(path);
}


QString ReportCore::pluginsPath() const
{
    QString path = m_settings->value("CuteReport/PluginsPath").toString();
    if (!path.isEmpty())
        return QDir::cleanPath(path);
    path = m_settings->value("CuteReport/RootPath").toString();
    return QDir::cleanPath(path.isEmpty() ? REPORT_PLUGINS_PATH : path + "/plugins");
}


QString ReportCore::settingsPath() const
{
    return m_settings->fileName();
}


bool ReportCore::inheritReport(ReportInterface *report, const QString &baseReportUrl)
{
    QString filePath = baseReportUrl.section(":", 1);
    QFileInfo fi (filePath);
    QString fileName = fi.fileName();
    QString path = fi.path();
    return inheritReport(report, fileName, path);
}


bool ReportCore::inheritReport(ReportInterface *report, const QString &baseReportFileName, const QString &baseReportPath)
{
    QString baseReportPathFinal = baseReportPath;
    if (!baseReportPathFinal.isEmpty())
           baseReportPathFinal = processString(CuteReport::ScriptString(report, this, baseReportPath)).resultStr;

    QStringList pathList = baseReportPathFinal.split(";");
    baseReportPathFinal = "";

    foreach (const QString & path, pathList) {
        QString p = path.trimmed();
        if (!p.endsWith("/"))
            p += "/";
        QString filePath = p + baseReportFileName;
        if (reportExists(filePath)) {
            baseReportPathFinal = filePath;
            break;
        }
    }

    CuteReport::ReportInterface * baseReport = baseReportPathFinal.isEmpty() ? 0 : loadReport(baseReportPathFinal);
    if (!baseReport) {
        InheritanceDataContainer inheritanceData = report->inheritanceData();
        inheritanceData.setBaseReportFound(false);
        report->setInheritanceData(inheritanceData);
        return false;
    }

    // if inheritanceData().baseReportPath() == baseReportPath then possible core tries to load report
    if (!report->inheritanceData().baseReportPath().isEmpty() /*&& report->inheritanceData().baseReportPath() != baseReportPath*/) {
        detachReport(report);
    }

    foreach (PageInterface * page, baseReport->pages()) {

        PageInterface * replacedPage = report->page(page->objectName());
        if (!replacedPage) {
            replacedPage = page->clone(false, false);
            replacedPage->setDpi(QApplication::desktop()->physicalDpiX());
            replacedPage->init();
            report->addPage(replacedPage);
        }

        replacedPage->setPageFlags(PageInterface::BaseReportPage);

        foreach (BaseItemInterface * item, page->items()) {
            if (item->parentItem())
                continue;
            BaseItemInterface * clonedBaseReportItem = item->clone(true, false);
            QString clonedBaseReportItemName = clonedBaseReportItem->objectName();
            int clonedBaseReportItemOrder = clonedBaseReportItem->order();

            BaseItemInterface * replacedPageExistentItem = replacedPage->item(clonedBaseReportItemName);
            if (replacedPageExistentItem)   /// to prevent renaming of clonedBaseReportItem
                replacedPageExistentItem->setObjectName("");
            replacedPage->addItem(clonedBaseReportItem);
            clonedBaseReportItem->init();
            clonedBaseReportItem->setOrder(clonedBaseReportItemOrder);
            if (replacedPageExistentItem) {  /// instead we rename existent one
                replacedPageExistentItem->setObjectName(uniqueName(replacedPageExistentItem, clonedBaseReportItemName, report));
                replacedPageExistentItem->setBaseItemFlags(BaseItemInterface::AutoRenamedFromBaseReport, true );
            }

            /// check other pages items
            foreach (PageInterface* page, report->pages())
                if (page == replacedPage)
                    continue;
                foreach (BaseItemInterface * item, page->items()) {
                    if (item->objectName() == clonedBaseReportItemName) {
                        item->setObjectName(clonedBaseReportItemName + "_inherited");
                        item->setBaseItemFlags(BaseItemInterface::AutoRenamedFromBaseReport, true );
                    }
                }

            clonedBaseReportItem->setBaseItemFlags(BaseItemInterface::BaseReportItem, true );
            clonedBaseReportItem->setBaseItemFlags(BaseItemInterface::LockedItem, true );
            QList<BaseItemInterface*> clonedBaseReportItemChildren = clonedBaseReportItem->findChildren<BaseItemInterface*>();
            foreach (BaseItemInterface * child, clonedBaseReportItemChildren) {
                child->setBaseItemFlags(BaseItemInterface::BaseReportItem, true );
                child->setBaseItemFlags(BaseItemInterface::LockedItem, true );

                /// check other pages items for every children
                QList<PageInterface *> origReportPages = report->pages();
                foreach (PageInterface* origPage, origReportPages) {
                    if (origPage == replacedPage)
                        continue;
                    foreach (BaseItemInterface * origItem, origPage->items()) {
                        if (origItem->objectName() == child->objectName()) {
                            origItem->setObjectName(child->objectName() + "_inherited");
                            origItem->setBaseItemFlags(BaseItemInterface::AutoRenamedFromBaseReport, true );
                        }
                    }
                }

            }

        }

    }


    foreach (DatasetInterface * ds, baseReport->datasets()) {
        DatasetInterface * clonedDs = ds->clone();
        clonedDs->setDatasetFlag(DatasetInterface::BaseReportDataset);
        report->addDataset(clonedDs);
    }

//    foreach (FormInterface * form, baseReport->forms()) {
//        report->addForm(form->clone());
//    }

    InheritanceDataContainer inheritanceData = report->inheritanceData();
    inheritanceData.setBaseReportName(baseReport->name());
    inheritanceData.setBaseReportPath(baseReportPath);
    inheritanceData.setBaseReportFileName(baseReportFileName);
    inheritanceData.setBaseReportFound(true);
    report->setInheritanceData(inheritanceData);

    delete baseReport;
    return true;
}


bool ReportCore::detachReport(ReportInterface *report, bool ownInheritedItems)
{
    QList<PageInterface *> pages = report->pages();
    foreach (PageInterface * page, pages) {
        QList<QPointer<BaseItemInterface> > itemsToDelete;
        foreach (BaseItemInterface * item, page->items()) {
            if (item->isBaseItemFlagSet(BaseItemInterface::BaseReportItem))
                itemsToDelete << QPointer<BaseItemInterface>(item);
        }
        foreach (QPointer<BaseItemInterface> item, itemsToDelete) {
            if (!item.data())
                continue;
            if (ownInheritedItems) {
                item->setBaseItemFlags(BaseItemInterface::BaseReportItem, false );
                item->setBaseItemFlags(BaseItemInterface::LockedItem, false );
            } else {
                page->deleteItem(item);
            }
        }
    }

    foreach (DatasetInterface * ds, report->datasets()) {
        if (ds->isDatasetFlagSet(DatasetInterface::BaseReportDataset)) {
            if (ownInheritedItems) {
                ds->setDatasetFlag(DatasetInterface::BaseReportDataset, false );
            } else {
                report->deleteDataset(ds);
            }
        }
    }

    report->setInheritanceData(InheritanceDataContainer());
    return true;
}


CuteReport::StorageInterface * ReportCore::storageByUrl(const QString & urlString, CuteReport::ReportInterface * report) const
{
    log(CuteReport::LogDebug, "ReportCore", "storageByUrl", QString("url: \'%1\'").arg(urlString));

    QUrl url(urlString);
    QString objectName = url.scheme();

#ifdef WIN32
    if (objectName.size() == 1) {// windows disk label
        objectName = "file"; // TODO: is "file" always correct?
    }
#endif
    log(CuteReport::LogDebug, "ReportCore", "storageByUrl", QString("objectName: \'%1\'").arg(objectName));
    return storage(objectName, report);
}


void ReportCore::setStorage(StorageInterface *storage)
{
    replaceObject(storage, m_storages, m_defaultStorage);
}


void ReportCore::deleteStorage(const QString &objectName)
{
    removeObject(objectName, m_storages);
}


QList<RendererInterface *> ReportCore::rendererList(ReportInterface *report) const
{
    QList<RendererInterface *> list;
    if (report)
        list << report->renderers();

    foreach (CuteReport::ReportPluginInterface * m, m_renderers) {
        bool exists = false;
        foreach(CuteReport::RendererInterface * mExists, list) {
            if (mExists->objectName() == m->objectName()) {
                exists =  true;
                break;
            }
        }
        if (!exists)
            list << static_cast<CuteReport::RendererInterface *>(m);
    }

    return list;
}


QStringList ReportCore::rendererNameList(ReportInterface *report) const
{
    QStringList list;
    if (report)
        list << report->rendererNames();
    list << objectNames(m_renderers);
    return list.toSet().toList();
}


RendererInterface *ReportCore::renderer(const QString &objectName) const
{
    return renderer(0, objectName);
}


RendererInterface *ReportCore::renderer(ReportInterface *report, const QString &objectName) const
{
    CuteReport::ReportPluginInterface * module = 0;
    if (report)
        module = report->renderer(objectName);

    if (!module && !objectName.isEmpty())
        module = getObjectByName(objectName, m_renderers);
    if (!module)
        module = m_defaultRenderer.data();
    if (!module && !m_renderers.isEmpty())
        module = m_renderers.at(0);
    return static_cast<CuteReport::RendererInterface*>(module);
}


void ReportCore::setRenderer(RendererInterface *renderer)
{
    replaceObject(renderer, m_renderers, m_defaultRenderer);
}


void ReportCore::deleteRenderer(const QString &objectName)
{
    removeObject(objectName, m_renderers);
}


QList<PrinterInterface *> ReportCore::printerList(ReportInterface *report) const
{
    QList<PrinterInterface *> list;
    if (report)
        list << report->printers();

    foreach (CuteReport::ReportPluginInterface * m, m_printers) {
        bool exists = false;
        foreach(CuteReport::PrinterInterface * mExists, list) {
            if (mExists->objectName() == m->objectName()) {
                exists =  true;
                break;
            }
        }
        if (!exists)
            list << static_cast<CuteReport::PrinterInterface *>(m);
    }

    return list;
}


QStringList ReportCore::printerNameList(ReportInterface *report) const
{
    QStringList list;
    if (report)
        list << report->printerNames();
    list << objectNames(m_printers);
    return list.toSet().toList();
}


PrinterInterface *ReportCore::printer(const QString &objectName, ReportInterface *report) const
{
    CuteReport::ReportPluginInterface * module = 0;
    if (report)
        module = report->printer(objectName);
    if (!module)
        module = getObjectByName(objectName, m_printers);
    if (!module)
        module = m_defaultPrinter;
    if (!module && !m_printers.isEmpty())
        module = m_printers.at(0);
    return static_cast<CuteReport::PrinterInterface*>(module);
}


void ReportCore::setPrinter(PrinterInterface *printer)
{
    replaceObject(printer, m_printers, m_defaultPrinter);
}


void ReportCore::deletePrinter(const QString &objectName)
{
    removeObject(objectName, m_printers);
}


CuteReport::StorageInterface * ReportCore::storage(const QString & objectName, CuteReport::ReportInterface * report) const
{
    CuteReport::ReportPluginInterface * module = 0;
    if (report)
        module = report->storage(objectName);
    if (!module)
        module = getObjectByName(objectName, m_storages);
    if (!module)
        module = m_defaultStorage;

    return static_cast<CuteReport::StorageInterface*>(module);
}


bool ReportCore::setDefaultStorage(const QString & objectName)
{
    if (objectName.isEmpty()) {
        m_defaultStorage = 0;
        return true;
    }
    m_defaultStorage = storage(objectName);
    return m_defaultStorage;
}


QString ReportCore::defaultStorageName() const
{
    return m_defaultStorage ? m_defaultStorage->objectName() : QString();
}


bool ReportCore::setDefaultPrinter(const QString objectName)
{
    if (objectName.isEmpty()) {
        m_defaultPrinter = 0;
        return true;
    }
    m_defaultPrinter = printer(objectName);
    return m_defaultPrinter;
}


QString ReportCore::defaultPrinterName() const
{
    return m_defaultPrinter ? m_defaultPrinter->objectName() : QString();
}


bool ReportCore::setDefaultRenderer(const QString & objectName)
{
    if (objectName.isEmpty()) {
        m_defaultRenderer = 0;
        return true;
    }
    m_defaultRenderer = renderer(objectName);
    return m_defaultRenderer;
}


QString ReportCore::defaultRendererName() const
{
    return m_defaultRenderer ? m_defaultRenderer->objectName() : QString();
}


void ReportCore::setRootWidget(QWidget * widget)
{
    m_rootWidget = widget;
}


QWidget * ReportCore::rootWidget()
{
    return m_rootWidget;
}


void ReportCore::init(QSettings *settings, bool initLogSystem)
{
    initMyResource();
    m_rootWidget = 0;
    m_maxRenderingThreads = MaxRenderingThreads;
    m_settings = settings;

    QString localAppSettingsFile;
    bool localAppSettingsFileExists = false;

    if (!m_settings) {
        localAppSettingsFile = QApplication::applicationDirPath() + "/cutereport.ini";
        if (QFile::exists(localAppSettingsFile)) {
            localAppSettingsFileExists = true;
            m_settings = new QSettings(localAppSettingsFile, QSettings::IniFormat, this);
        } else
            m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "ExaroLogic", "CuteReport", this);
    }

    if (initLogSystem) {
        Log::createInstance(m_settings);
    }
    /** FIXME - log can be inited by other ReportCore instance. As a temporary solution we still track reference counter */
    Log::refCounterInc(this);


    log(CuteReport::LogDebug, MODULENAME, "Version", QString("CuteReport version: %1").arg(REPORT_VERSION));
    log(CuteReport::LogDebug, MODULENAME, "Appdir", "Application path: " + QApplication::applicationDirPath() );
    log(CuteReport::LogDebug, MODULENAME, "Workdir", "Current path: " + QDir::currentPath() );
    if (localAppSettingsFile.isEmpty())
        log(CuteReport::LogDebug, MODULENAME, "SettingsFile", QString("Settings file specified: %1").arg(m_settings->fileName()));
    else if (localAppSettingsFileExists)
        log(CuteReport::LogDebug, MODULENAME, "SettingsFile", QString("Local settings file: %1").arg(localAppSettingsFile));
    else
        log(CuteReport::LogDebug, MODULENAME, "SettingsFile", QString("Default settings file used: %1").arg(m_settings->fileName()));
    log(CuteReport::LogDebug, MODULENAME, "LogFile", QString("Log file path: %1").arg(Log::instance() ? Log::instance()->logFileName() : "No log file - Log is not inited!"));
    log(CuteReport::LogDebug, MODULENAME, "QApp pointer:", QString("%1").arg((long long)QCoreApplication::instance()));
    log(CuteReport::LogDebug, MODULENAME, "Qt version:", QT_VERSION_STR);
#ifdef QT_DEBUG
    log(CuteReport::LogDebug, MODULENAME, "CuteReport build:", "Debug");
#else
    log(CuteReport::LogDebug, MODULENAME, "CuteReport build:", "Release");
#endif

    if (!QCoreApplication::instance()) {
        log(CuteReport::LogCritical, MODULENAME, "QCoreApplication is not instantiated!", QString("Possible reasons: 1 - QCoreApplication is not instantiated in your main.cpp file; 2) The application build (Release/Debug) does not match CuteReport build 3) The application Qt major version does not match CuteReport build major Qt major version."));
        std::cerr << "The application has stopped because of critical error in CuteReport. Please, check CuteReport log for details." << std::endl;
        exit(1);
    }

    ReportCoreConfigCleanUp::process(m_settings, this);

    bool pluginsResultOk = true;
    if (m_refCount == 0)
        pluginsResultOk = loadPlugins(m_settings);
    else
        log(CuteReport::LogDebug, MODULENAME, "Plugins are already preloaded!");

    if (!pluginsResultOk) {
        log(CuteReport::LogCritical, MODULENAME, "Application is exiting because of critial error in CuteReport!");
        exit(1);
    }

    m_settings->setValue("CuteReport/LastExitSuccess", false); // in case of crash we will have false here

    bool restoreState = m_settings->value("CuteReport/AutoRestore", true).toBool();

    /// clean restore is true by default to prevent duplication when we have settings
    /// for one renderer and it is empty for another one
    bool cleanRestore = m_settings->value("CuteReport/CleanRestore", true).toBool();

    /// Init anyway even if it will be cleaned up by restoring
    /// For a new installation it sets defaults

    createLocalObjects(m_storages, m_storagePlugins);
    createLocalObjects(m_printers, m_printerPlugins);
    createLocalObjects(m_renderers, m_rendererPlugins);
    createLocalObjects(m_serializers, m_serializePlugins);
    createLocalObjects(m_exporters, m_exportPlugins);

    m_defaultStorage = findDefaultObject("Storage", "Standard::Filesystem", m_storages);
    m_defaultPrinter = findDefaultObject("Printer", "standard::printer", m_printers);
    m_defaultRenderer = findDefaultObject("Renderer", "standard::renderer",  m_renderers);
    m_defaultSerializer = findDefaultObject("Serializer", "Standard::XML", m_serializers);

    if (restoreState)
        this->restoreState(cleanRestore);

    ++m_refCount;
}

bool ReportCore::loadPlugins(QSettings *settings)
{
    //    qRegisterMetaType<CuteReport::Unit>("CuteReport::Unit");
    qRegisterMetaType<CuteReport::Units>("CuteReport::Units");
    qRegisterMetaType<CuteReport::Margins>("CuteReport::Margins");
    qRegisterMetaType<QList<CuteReport::PageAction*> >("QList<CuteReport::PageAction*>");
    qRegisterMetaType<CuteReport::LogLevel>("CuteReport::LogLevel");
    qRegisterMetaType<CuteReport::RenderingType>("CuteReport::RenderingType");

    QFileInfoList files;
    QList<QDir> dirs;
    QString iniPath = settings->value("CuteReport/PluginsPath").toString();
    if (iniPath.isEmpty()) {
        log(CuteReport::LogDebug, MODULENAME, "CuteReport/PluginsPath is empty. trying hardcoded path... " );
        QString hardPath = QString(REPORT_PLUGINS_PATH).trimmed();
        if (hardPath.isEmpty()) {
            log(CuteReport::LogDebug, MODULENAME, "Hardcoded path is empty" );
        } else {
            log(CuteReport::LogDebug, MODULENAME, QString("Hardcoded path is \"%1\"").arg(hardPath) );
            QDir dirHard(REPORT_PLUGINS_PATH);
            dirHard.setCurrent(QCoreApplication::applicationDirPath());
            if (!dirs.contains(dirHard))
                dirs << dirHard;
            if (!dirHard.isAbsolute()) {
                log(CuteReport::LogDebug, MODULENAME, "Hardcoded path is relative, so trying to check system install path too..." );
                QString appPath = settings->value("CuteReport/AppPath").toString();
                log(CuteReport::LogDebug, MODULENAME, QString("CuteReport/AppPath is \"%1\"").arg(appPath) );
                if (!appPath.isEmpty()) {
                    /// FIXME: works for windows. Make sure it is correct for other OSs
                    QString plPath = settings->value("CuteReport/AppPath").toString() + "/" + REPORT_PLUGINS_PATH;
                    QDir plDir(plPath);
                    if (!dirs.contains(plDir)) {
                        dirs << plPath;
                        log(CuteReport::LogDebug, MODULENAME, QString("System path \"%1\" has been added to check list").arg(plPath));
                    } else
                        log(CuteReport::LogDebug, MODULENAME, QString("System path \"%1\" is already added").arg(plPath));
                }
            }
        }
    } else {
        QDir iniDir(iniPath);
        if (!dirs.contains(iniDir))
            dirs << iniDir;
    }

    QList<ReportPluginInterface *> pluginList;
    foreach (QObject *pluginObject, QPluginLoader::staticInstances()) {
        ReportPluginInterface * plugin = dynamic_cast<ReportPluginInterface *>(pluginObject);
        if (plugin) {
            log(LogDebug, MODULENAME, QString("Found static plugin: %1").arg(plugin->moduleFullName()), "");
            pluginList.push_back(plugin);
        }
    }

    foreach (const QDir & dir, dirs) {
//        log(CuteReport::LogDebug, MODULENAME, "applicationDirPath: " + QCoreApplication::applicationDirPath() );
//        dir.setPath(QDir::isRelativePath(dirStr) ? QCoreApplication::applicationDirPath() + "/" + dirStr : dirStr);
//        log(CuteReport::LogDebug, MODULENAME, "Raw Plugin dir: " + dirStr );
        log(CuteReport::LogDebug, MODULENAME, QString("Plugin dir to load: \"%1\"").arg(dir.absolutePath()) );
        files += dir.entryInfoList(QStringList() << "*.so" << "*.dll" << "*.dylib", QDir::Files);
    }

    QPluginLoader loader;
    loader.setLoadHints(QLibrary::ResolveAllSymbolsHint|QLibrary::ExportExternalSymbolsHint);

    foreach(const QFileInfo & fileName, files) {
        loader.setFileName(fileName.absoluteFilePath());
        if (!loader.load()) {
            log(LogWarning, MODULENAME, "Error while loading plugin " + fileName.fileName() + ": " + loader.errorString() );
            continue;
        }

        ReportPluginInterface * plugin = dynamic_cast<ReportPluginInterface *>(loader.instance());
        if (plugin) {
            log(CuteReport::LogDebug, MODULENAME, "Loading plugin: " + fileName.baseName() );
            pluginList.append(plugin);
        } else {
            log(CuteReport::LogDebug, MODULENAME, "Plugin has not CuteReport's type: " + fileName.baseName() );
            loader.unload();
        }
    }

    m_itemPlugins = new QList<ReportPluginInterface*>();
    m_pagePlugins = new QList<ReportPluginInterface*>();
    m_datasetPlugins = new QList<ReportPluginInterface*>();
    m_storagePlugins = new QList<ReportPluginInterface*>();
    m_serializePlugins = new QList<ReportPluginInterface*>();
    m_rendererPlugins = new QList<ReportPluginInterface*>();
    m_printerPlugins = new QList<ReportPluginInterface*>();
    m_formPlugins = new QList<ReportPluginInterface*>();
    m_importPlugins = new QList<ReportPluginInterface*>();
    m_exportPlugins = new QList<ReportPluginInterface*>();
    m_scriptExtensionPlugins = new QList<ReportPluginInterface*>();

    foreach (ReportPluginInterface * plugin, pluginList) {
        if (qobject_cast<CuteReport::BaseItemInterface*>(plugin))
            m_itemPlugins->append(static_cast<CuteReport::BaseItemInterface*>(plugin));

        else if (qobject_cast<CuteReport::PageInterface*>(plugin))
            m_pagePlugins->append(static_cast<CuteReport::PageInterface*>(plugin));

        else if (qobject_cast<CuteReport::DatasetInterface*>(plugin))
            m_datasetPlugins->append(static_cast<CuteReport::DatasetInterface*>(plugin));

        else if (qobject_cast<CuteReport::StorageInterface*>(plugin))
            m_storagePlugins->append(static_cast<CuteReport::StorageInterface*>(plugin));

        else if (qobject_cast<CuteReport::RendererInterface*>(plugin))
            m_rendererPlugins->append(static_cast<CuteReport::RendererInterface*>(plugin));

        else if (qobject_cast<CuteReport::PrinterInterface*>(plugin))
            m_printerPlugins->append(static_cast<CuteReport::PrinterInterface*>(plugin));

        else if (qobject_cast<CuteReport::FormInterface*>(plugin))
            m_formPlugins->append(static_cast<CuteReport::FormInterface*>(plugin));

        else if (qobject_cast<CuteReport::SerializerInterface*>(plugin))
            m_serializePlugins->append(static_cast<CuteReport::SerializerInterface*>(plugin));

        else if (qobject_cast<CuteReport::ImportInterface*>(plugin))
            m_importPlugins->append(static_cast<CuteReport::ImportInterface*>(plugin));

        else if (qobject_cast<CuteReport::ExportInterface*>(plugin))
            m_exportPlugins->append(static_cast<CuteReport::ExportInterface*>(plugin));

        else if (qobject_cast<CuteReport::ScriptExtensionInterface*>(plugin))
            m_scriptExtensionPlugins->append(static_cast<CuteReport::ScriptExtensionInterface*>(plugin));
    }

    processModuleList(*m_itemPlugins);
    processModuleList(*m_pagePlugins);
    processModuleList(*m_datasetPlugins);
    processModuleList(*m_serializePlugins);
    processModuleList(*m_storagePlugins);
    processModuleList(*m_printerPlugins);
    processModuleList(*m_rendererPlugins);
    processModuleList(*m_importPlugins);
    processModuleList(*m_exportPlugins);
    processModuleList(*m_scriptExtensionPlugins);

    /// Checking critical plugins presence and default plugin settings
    bool error = false;

    if (!m_itemPlugins->size()) {
        error = true;
        log(LogCritical, MODULENAME, "Item plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 item plugins found").arg(m_itemPlugins->count()) );

    if (!m_pagePlugins->size()) {
        error = true;
        log(LogCritical, MODULENAME, "Page plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 page plugins found").arg(m_pagePlugins->count()) );

    if (!m_datasetPlugins->size()) {
        error = true;
        log(LogCritical, MODULENAME, "Dataset plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 dataset plugins found").arg(m_datasetPlugins->count()) );

    if (!m_storagePlugins->size()) {
        error = true;
        log(LogCritical, MODULENAME, "Storage plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 storage plugins found").arg(m_storagePlugins->count()) );

    if (!m_serializePlugins->size()) {
        error = true;
        log(LogCritical, MODULENAME, "Serialize plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 serialize plugins found").arg(m_serializePlugins->count()) );

    if (!m_rendererPlugins->size()) {
        error = true;
        log(LogCritical, MODULENAME, "Renderer plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 renderer plugins found").arg(m_rendererPlugins->count()) );

    if (!m_printerPlugins->size()) {
        error = true;
        log(LogCritical, MODULENAME, "Printer plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 printer plugins found").arg(m_printerPlugins->count()) );

    if (!m_formPlugins->size()) {
        log(LogCritical, MODULENAME, "Forms plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 printer plugins found").arg(m_formPlugins->count()) );

    if (!m_importPlugins->size()) {
        log(LogWarning, MODULENAME, "Import plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 import plugins found").arg(m_importPlugins->count()) );

    if (!m_exportPlugins->size()) {
        log(LogWarning, MODULENAME, "Export plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 export plugins found").arg(m_exportPlugins->count()) );

    if (!m_scriptExtensionPlugins->size()) {
        log(LogWarning, MODULENAME, "Script extension plugins not found!" );
    } else
        log(LogDebug, MODULENAME, QString("%1 script extension plugins found").arg(m_scriptExtensionPlugins->count()) );

    if (error) {
        log(LogCritical, MODULENAME, "CRITICAL ERRORS FOUND!" );
        return false;
    }

    return true;
}


void ReportCore::processModuleList(QList<ReportPluginInterface*> &list)
{
    // check and remove duplicates first (might come from different sources), leave only the first module found
    QStringList nameList;
    foreach (ReportPluginInterface * module, list) {
        nameList << module->moduleFullName();
    }

    if (nameList.toSet().count() != nameList.count()) {
        /// there are duplicates found
        QList<ReportPluginInterface*> duplicatesToRemove;
        QStringList duplicatesNames;
        for (int i = list.count() -1; i>=0; --i){
            ReportPluginInterface * plugin = list.at(i);
            int index = nameList.indexOf(plugin->moduleFullName());
            if (index != i) {
                duplicatesToRemove << plugin;
                duplicatesNames << plugin->moduleFullName();
            }
        }

        log(LogDebug, MODULENAME, QString("Duplicate modules to be removed: %1").arg(duplicatesNames.join(", ")) );

        foreach (ReportPluginInterface * plugin, duplicatesToRemove) {
           list.removeOne(plugin);
        }
    }


    QStringList removeList;
    foreach (ReportPluginInterface * module, list) {
        removeList.append(module->removesModules());
    }
    QMutableListIterator<ReportPluginInterface *> i(list);
    while (i.hasNext()) {
        ReportPluginInterface * module = i.next();
        if (removeList.contains(module->moduleFullName()) && !module->moduleFlags().testFlag(CuteReport::ReportPluginInterface::Unremovable)) {
            log(LogDebug, MODULENAME, QString("Module to be removed: %1").arg(module->moduleFullName()) );
            delete module;
            i.remove();
        }
    }
    foreach (ReportPluginInterface * module, list)
        module->moduleInit();
}


void ReportCore::fixLoadedReportIssues(ReportInterface *report)
{
    QStringList itemNames = report->itemNames();
    if (itemNames.removeDuplicates() > 0) {
        QStringList itemNamesWithDubs = report->itemNames();
        QStringList duplicates;
        while (itemNamesWithDubs.count()) {
            QString name = itemNamesWithDubs.takeFirst();
            if (itemNames.contains(name)) {
                duplicates << name;
                itemNamesWithDubs.removeAll(name);
            }
        }

        QList<BaseItemInterface*> items = report->items();
        foreach (BaseItemInterface * item, items) {
            if (duplicates.contains(item->objectName()))
                item->setObjectName(uniqueName(item, item->objectName() + "_duplicate", report));
        }

    }

    /// rename storage duplicates
    QStringList storageNames = report->storageNames();
    if (storageNames.removeDuplicates() > 0) {
        QStringList storageNamesWithDubs = report->storageNames();
        QStringList duplicates;
        while (storageNamesWithDubs.count()) {
            QString name = storageNamesWithDubs.takeFirst();
            if (storageNames.contains(name)) {
                duplicates << name;
                storageNamesWithDubs.removeAll(name);
            }
        }

        QList<StorageInterface*> storages = report->storages();
        foreach (StorageInterface * storage, storages) {
            if (duplicates.contains(storage->objectName()))
                storage->setObjectName(uniqueName(storage, storage->objectName() + "_duplicate", report));
        }
    }

    /// rename renderer duplicates
    QStringList rendererNames = report->rendererNames();
    if (rendererNames.removeDuplicates() > 0) {
        QStringList rendererNamesWithDubs = report->rendererNames();
        QStringList duplicates;
        while (rendererNamesWithDubs.count()) {
            QString name = rendererNamesWithDubs.takeFirst();
            if (rendererNames.contains(name)) {
                duplicates << name;
                rendererNamesWithDubs.removeAll(name);
            }
        }

        QList<RendererInterface*> renderers = report->renderers();
        foreach (RendererInterface * renderer, renderers) {
            if (duplicates.contains(renderer->objectName()))
                renderer->setObjectName(uniqueName(renderer, renderer->objectName() + "_duplicate", report));
        }
    }
}



const QList<ReportPluginInterface *> ReportCore::modules(ModuleType moduleType) const
{
    switch (moduleType) {
        case UnknownModuleType: return QList<ReportPluginInterface *>();
        case ItemModule:        return *m_itemPlugins;
        case PageModule:        return *m_pagePlugins;
        case DatasetModule:     return *m_datasetPlugins;
        case StorageModule:     return *m_storagePlugins;
        case RendererModule:    return *m_rendererPlugins;
        case PrinterModule:     return *m_printerPlugins;
        case FormModule:        return *m_formPlugins;
        case ImportModule:      return *m_importPlugins;
        case ExportModule:      return *m_exportPlugins;
        case SerializerModule:  return *m_serializePlugins;
        case ScriptExtensionModule: return *m_scriptExtensionPlugins;
        case MAX_MODULE_TYPE:   break;
    }

    return QList<ReportPluginInterface *>();
}


QList<StorageInterface *> ReportCore::storageList(ReportInterface *report) const
{
    QList<StorageInterface *> list;
    if (report) {
        QList<CuteReport::StorageInterface *> storages = report->storages();
        list << storages;
    }

    foreach (CuteReport::ReportPluginInterface * m, m_storages) {
        bool exists = false;
        foreach(CuteReport::StorageInterface * mExists, list) {
            if (mExists->objectName() == m->objectName()) {
                exists =  true;
                break;
            }
        }
        if (!exists)
            list << static_cast<CuteReport::StorageInterface *>(m);
    }

    return list;
}


QStringList ReportCore::storageNameList(ReportInterface *report) const
{
    QStringList list;

    if (report)
        list << report->storageNames();
    list << objectNames(m_storages);
    return list.toSet().toList();
}


QStringList ReportCore::moduleNames(ModuleType moduleType, bool shortName) const
{
    QStringList list;
    switch (moduleType) {
        case UnknownModuleType: break;
        case ItemModule:
            foreach (ReportPluginInterface * module, *m_itemPlugins)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case PageModule:
            foreach (ReportPluginInterface * module, *m_pagePlugins)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case DatasetModule:
            foreach (ReportPluginInterface * module, *m_datasetPlugins)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case StorageModule:
            foreach (ReportPluginInterface * module, m_storages)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case RendererModule:
            foreach (ReportPluginInterface * module, m_renderers)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case PrinterModule:
            foreach (ReportPluginInterface * module, m_printers)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case FormModule:
            foreach (ReportPluginInterface * module, *m_formPlugins)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case ExportModule:
            foreach (ReportPluginInterface * module, m_exporters)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case ImportModule:
            foreach (ReportPluginInterface * module, *m_importPlugins)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case SerializerModule:
            foreach (ReportPluginInterface * module, m_serializers)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case ScriptExtensionModule:
            foreach (ReportPluginInterface * module, *m_scriptExtensionPlugins)
                list.append(shortName ? module->moduleShortName() : module->moduleFullName());
            break;
        case MAX_MODULE_TYPE:
            break;
    }

    return list;
}


QString ReportCore::moduleTypeToString(ModuleType type) const
{
    if (type >= 0 && type < MAX_MODULE_TYPE)
        return ModuleTypeStr[type];
    return QString();

}


ModuleType ReportCore::moduleTypeFromString(const QString & type) const
{
    QString typeLower = type.toLower();
    for (int i = 0; i < MAX_MODULE_TYPE; ++i) {
        if (QString(ModuleTypeStr[i]).toLower() == typeLower)
            return ModuleType(i);
    }
    return UnknownModuleType;
}


const CuteReport::ReportPluginInterface * ReportCore::module(ModuleType moduleType, const QString & moduleName) const
{
    switch (moduleType) {
        case UnknownModuleType:     return 0;
        case ItemModule:            return getOriginalModuleByName(moduleName, *m_itemPlugins);
        case PageModule:            return getOriginalModuleByName(moduleName, *m_pagePlugins);
        case DatasetModule:         return getOriginalModuleByName(moduleName, *m_datasetPlugins);
        case StorageModule:         return getOriginalModuleByName(moduleName, *m_storagePlugins);
        case RendererModule:        return getOriginalModuleByName(moduleName, *m_rendererPlugins);
        case PrinterModule:         return getOriginalModuleByName(moduleName, *m_printerPlugins);
        case FormModule:            return getOriginalModuleByName(moduleName, *m_formPlugins);
        case ImportModule:          return getOriginalModuleByName(moduleName, *m_importPlugins);
        case ExportModule:          return getOriginalModuleByName(moduleName, *m_exportPlugins);
        case SerializerModule:      return getOriginalModuleByName(moduleName, *m_serializePlugins);
        case ScriptExtensionModule: return getOriginalModuleByName(moduleName, *m_scriptExtensionPlugins);
        case MAX_MODULE_TYPE:       break;
    }

    return 0;
}


QStringList ReportCore::moduleOptions(ReportPluginInterface * module, bool includeObjectName)
{
    QStringList list;

    if (!module)
        return list;

#if QT_VERSION >= 0x050000
    int firstIndex = 0;
#else
    int firstIndex = 1;
#endif

    if (!includeObjectName)
        firstIndex++;

    for (int i = firstIndex; i<module->metaObject()->propertyCount(); ++i) {
        QString propertyName = module->metaObject()->property(i).name();
        if (!propertyName.isEmpty() && propertyName.at(0) != '_') {
            QString str = QString("%1=%2").arg(propertyName).arg(module->metaObject()->property(i).read(module).toString());
            list << str;
        }
    }
    return list;
}


void ReportCore::setModuleOptions(ReportPluginInterface *module, const QStringList &options, bool setObjectNameIfDefined)
{
    if (!module || options.isEmpty())
        return;

    for (int i = 0; i<options.size(); ++i) {
        QString property = options[i].section("=",0,0);
        if (property == QString("objectName") && !setObjectNameIfDefined)
            continue;

        QString value = options[i].section("=",1,1);
        if (value.isEmpty() || property.isEmpty() || property.at(0) == '_')
            continue;
        module->setProperty(property.toLatin1(), value);
    }
}


QString ReportCore::moduleOptionsStr(ReportPluginInterface *module, bool includeObjectName, const QString &delimiter)
{
    QString result;
    QString delimiter_;
    if (!delimiter.isEmpty()) {
        result = moduleOptions(module, includeObjectName).join(delimiter);
        delimiter_ = delimiter;
    }
    else {
        QStringList list = moduleOptions(module, includeObjectName);
        QStringList defaultDelimiters;
        defaultDelimiters << "," << ";" << "|" << QChar(254) << "\t";

        foreach (const QString & del, defaultDelimiters) {
            bool ok = true;
            foreach (const QString & value, list) {
                if (value.contains(del)) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                result = list.join(del);
                delimiter_ = del;
                break;
            }
        }
    }

    return delimiter_+ module->moduleFullName() + delimiter_ + result;
}


void ReportCore::setModuleOptionsStr(ReportPluginInterface * module, const QString & options, bool setObjectNameIfDefined)
{
    if (options.size() < 3)
        return;
    QString delimiter = options.at(0);
    QString str = options.mid(1);
    QStringList list = str.split(delimiter, QString::KeepEmptyParts);
    if (!list.size())
        return;
    QString moduleName = list.takeFirst();
    if (moduleName.toLower() != module->moduleFullName().toLower()) {
        log(LogWarning, MODULENAME, "Options passed to inappropriate module", QString("Module name \'%1\', options module name \'%2\'").arg(module->moduleFullName(), moduleName));
        return;
    }
    setModuleOptions(module, list, setObjectNameIfDefined);
}


ReportPluginInterface * ReportCore::getOriginalModuleByName(const QString & moduleName, const QList<ReportPluginInterface *> &list) const
{
    ReportPluginInterface * module = 0;
    bool fullName = moduleName.contains("::");
    QString mName = moduleName.toLower();
    foreach (ReportPluginInterface * m, list) {
        QString name = fullName ? m->moduleFullName().toLower() : m->moduleShortName().toLower();
        if (name == mName) {
            module = m;
            break;
        }
    }
    return module;
}


ReportPluginInterface *ReportCore::getExtendedModuleByName(const QString &origModuleName, const QList<ReportPluginInterface *> &list) const
{
    ReportPluginInterface * module = 0;
    QString moduleShortName = origModuleName.section("::",1,1);
    foreach  (ReportPluginInterface * m, list)
        if (m->extendsModules().contains(origModuleName) || m->extendsModules().contains(moduleShortName)) {
            module = m;
            break;
        }
    return module;
}


CuteReport::ReportPluginInterface * ReportCore::getModuleByName(const QString & moduleName, const QList<ReportPluginInterface *> &list)
{
    CuteReport::ReportPluginInterface * plugin = 0;
    QRegExp moduleSeparator("[,;]");
    if (moduleName.contains(moduleSeparator)) {
        QStringList moduleNames = moduleName.split(moduleSeparator);
        foreach (const QString & moduleName, moduleNames) {
            const QString name = moduleName.trimmed();
            plugin = getOriginalModuleByName(name, list);
            if (plugin)
                break;
        }
    } else {
        plugin = getOriginalModuleByName(moduleName, list);
        if (!plugin)
            plugin = getExtendedModuleByName(moduleName, list);
    }

    return plugin;
}


//const BaseItemInterface * ReportCore::itemModule(const QString & moduleName) const
//{
//    BaseItemInterface * module = 0;
//    foreach (BaseItemInterface * m, *m_itemPlugins) {
//        if (m->moduleFullName() == moduleName) {
//            module = m;
//            break;
//        }
//    }

//    return module;
//    return static_cast<const BaseItemInterface *>(getModuleByName(moduleName, *m_itemPlugins));
//}


//const PageInterface *ReportCore::pageModule(const QString & moduleName) const
//{
//    PageInterface * module = 0;
//    if (moduleName.isEmpty() && m_pagePlugins->count())
//        module = m_pagePlugins->at(0);
//    else {
//        foreach (PageInterface * m, *m_pagePlugins) {
//            if (m->moduleFullName() == moduleName) {
//                module = m;
//                break;
//            }
//        }
//    }

//    return module;
//    return static_cast<const PageInterface *>(getModuleByName(moduleName, *m_pagePlugins));
//}


//const DatasetInterface *ReportCore::datasetModule(const QString & moduleName) const
//{
//    DatasetInterface * module = 0;
//    if (moduleName.isEmpty() && m_datasetPlugins->count())
//        module = m_datasetPlugins->at(0);
//    else {
//        foreach (DatasetInterface * m, *m_datasetPlugins) {
//            if (m->moduleFullName() == moduleName) {
//                module = m;
//                break;
//            }
//        }
//    }

//    return module;

//    return static_cast<const DatasetInterface *>(getModuleByName(moduleName, *m_datasetPlugins));
//}


//CuteReport::SerializerInterface * ReportCore::serializerModule(const QString & moduleName) const
//{
//    SerializerInterface * module = 0;
//    if (moduleName.isEmpty() && m_serializers.count())
//        module = m_serializers.at(0);
//    else {
//        foreach (SerializerInterface * m, m_serializers) {
//            if (m->moduleFullName() == moduleName) {
//                module = m;
//                break;
//            }
//        }
//    }

//    return module;
//    return static_cast<const SerializerInterface *>(getModuleByName(moduleName, *m_serializers));
//}


//StorageInterface *ReportCore::storageModule(const QString & moduleName) const
//{
//    StorageInterface * module = 0;
//    if (moduleName.isEmpty() && m_storages.count())
//        module = m_storages.at(0);
//    else {
//        foreach (StorageInterface * m, m_storages) {
//            if (m->moduleFullName() == moduleName) {
//                module = m;
//                break;
//            }
//        }
//    }

//    return module;
//}


//CuteReport::StorageInterface* ReportCore::storageModuleByScheme(const QString & scheme) const
//{
//    StorageInterface * module = 0;
//    foreach (StorageInterface * m, m_storages) {
//        if (m->urlScheme() == scheme) {
//            module = m;
//            break;
//        }
//    }

//    return module;
//}


//ReportInterface * ReportCore::reportByName(const QString & reportName) const
//{
//    foreach (CuteReport::ReportInterface * report, m_reports)
//        if (report->objectName() == reportName)
//            return report;
//    return 0;
//}


PageInterface *  ReportCore::pageByName(const QString & pageName, CuteReport::ReportInterface * report) const
{
    return report->findChild<PageInterface *>(pageName);
}


//CuteReport::PrinterInterface*  ReportCore::printerModule(const QString & moduleName) const
//{
//    PrinterInterface * printer = 0;
//    if (moduleName.isEmpty() && m_printers.count())
//        printer = m_printers.at(0);
//    else {
//        foreach (PrinterInterface * i, m_printers) {
//            if (i->moduleFullName() == moduleName) {
//                printer = i;
//                break;
//            }
//        }
//    }
//    return printer;
//}


//CuteReport::FormInterface* ReportCore::formModule(const QString & moduleName) const
//{
//    FormInterface * form = 0;
//    if (moduleName.isEmpty() && m_formPlugins->count())
//        form = m_formPlugins->at(0);
//    else {
//        foreach (FormInterface * i, *m_formPlugins) {
//            if (i->moduleFullName() == moduleName) {
//                form = i;
//                break;
//            }
//        }
//    }
//    return form;
//}


//const ImportInterface *ReportCore::importModule(const QString &moduleName) const
//{
//    ImportInterface * module = 0;

//    foreach (ImportInterface * m, *m_importPlugins) {
//        if (m->moduleFullName() == moduleName) {
//            module = m;
//            break;
//        }
//    }

//    return module;
//}


//CuteReport::ExportInterface* ReportCore::exportModule(const QString & moduleName) const
//{
//    ExportInterface * exporter = 0;
//    foreach (ExportInterface * i, m_exporters) {
//        if (i->moduleFullName() == moduleName) {
//            exporter = i;
//            break;
//        }
//    }
//    return exporter;
//}


//CuteReport::RendererInterface * ReportCore::rendererModule(const QString & moduleName) const
//{
//    RendererInterface * renderer = 0;
//    if (moduleName.isEmpty() && m_renderers.count()) {
//        renderer = m_renderers.at(0);
//        QString moduleShortName = moduleName.section("::",1,1);
//        foreach (RendererInterface * r, m_renderers) {
//            if (r->extendsModules().contains(renderer->moduleFullName())  || r->extendsModules().contains(moduleShortName)) {
//                renderer = r;
//                break;
//            }
//        }
//    } else {
//        foreach (RendererInterface * r, m_renderers) {
//            if (r->moduleFullName() == moduleName) {
//                renderer = r;
//                break;
//            }
//        }
//    }

//    return renderer;
//}


BaseItemInterface * ReportCore::itemByName(const QString & itemName, CuteReport::PageInterface * page) const
{
    BaseItemInterface * resultItem = 0;
    QList <BaseItemInterface*> items = page->items();
    foreach (BaseItemInterface* item, items)
        if (item->objectName() == itemName) {
            resultItem = item;
            break;
        }

    return resultItem;
}


BaseItemInterface * ReportCore::itemByName(const QString & itemName, const QString & pageName, CuteReport::ReportInterface * report) const
{
    PageInterface * resultPage = 0;
    QList <PageInterface*> pages = report->pages();
    foreach (PageInterface* page, pages)
        if (page->objectName() == pageName) {
            resultPage = page;
            break;
        }

    if (resultPage)
        return itemByName(itemName, resultPage);
    else
        return 0;
}


DatasetInterface * ReportCore::datasetByName(const QString & datasetName, CuteReport::ReportInterface * report) const
{
    return report->dataset(datasetName);
}


FormInterface * ReportCore::formByName(const QString & formName, CuteReport::ReportInterface * report) const
{
    return report->form(formName);
}


QList<ReportInterface *> ReportCore::reports() const
{
    return findChildren<CuteReport::ReportInterface*>();
}


//const QList<CuteReport::ReportInterface*> & ReportCore::reports() const
//{
//    return m_reports;
//}


ReportInterface *ReportCore::createReport()
{
    CuteReport::ReportInterface * newReport = new CuteReport::ReportInterface(this);
    _reportObjectCreated(newReport);
    return newReport;
}


void ReportCore::deleteReport(CuteReport::ReportInterface * report)
{
    stopRendering(report, true);
    delete report;
}


bool ReportCore::saveReport(const QString & urlString, CuteReport::ReportInterface * report, QString * errorText)
{
    if (urlString.isEmpty() || !report)
        return false;

    CuteReport::StorageInterface * module = 0;
    if (!checkReportPointer(report, errorText) || !(module = storageByUrl(urlString, report) ))
        return false;

    bool saveResultOk = module->saveObject(urlString, serialize(report));

    if (saveResultOk) {
        report->setFileUrl(module->normalizedURL(urlString));
        report->setDirty(false);
    } else {
        if (errorText)
            *errorText = module->lastError();
    }

    return saveResultOk;
}


bool ReportCore::reportExists(const QString &urlString)
{
    log(CuteReport::LogDebug, "ReportCore", "reportExists", QString("url: \'%1\'").arg(urlString));

    if (urlString.isEmpty())
        return false;

    CuteReport::StorageInterface * module = 0;
    if (!(module = storageByUrl(urlString, 0)))
        return false;

    return module->objectExists(urlString);
}


CuteReport::ReportInterface * ReportCore::loadReport(const QString & urlString, QString * errorText)
{
    log(CuteReport::LogDebug, "ReportCore", "loadReport", QString("url: \'%1\'").arg(urlString));

    if (urlString.isEmpty())
        return 0;

    CuteReport::StorageInterface * module = 0;
    if (!(module = storageByUrl(urlString, 0)))
        return 0;

    QVariant object = module->loadObject(urlString);
    CuteReport::ReportInterface * report = dynamic_cast<CuteReport::ReportInterface*> (deserialize(object.toByteArray() /*, &ok, &error*/));

    if (report && !report->inheritanceData().baseReportFileName().isEmpty())
        inheritReport(report, report->inheritanceData().baseReportFileName(), report->inheritanceData().baseReportPath());

    if (!report && errorText)
        *errorText = module->lastError();

    if (report) {
        report->setFileUrl(module->normalizedURL(urlString));
        _reportObjectCreated(report);
    }
    fixLoadedReportIssues(report);

    return report;
}


bool ReportCore::saveObject(const QString & urlString,
                            CuteReport::ReportInterface * report,
                            const QByteArray &objectData,
                            QString * errorText)
{
    CuteReport::StorageInterface * module = 0;
    if (!checkReportPointer(report, errorText) || !(module = storageByUrl(urlString, report)) )
        return false;

    bool saveResultOk = module->saveObject(urlString, objectData);
    if (!saveResultOk && errorText)
        *errorText = module->lastError();

    return saveResultOk;
}


QByteArray ReportCore::loadObject(const QString & urlString,
                                  CuteReport::ReportInterface * report,
                                  QString * errorText)
{
    CuteReport::StorageInterface * module = 0;
    if ((report && !checkReportPointer(report, errorText)) || !(module = storageByUrl(urlString, report)) )
        return QByteArray();

    QByteArray data = module->loadObject(urlString);

    if (data.isNull() && errorText)
        *errorText = module->lastError();

    return data;
}


QString ReportCore::localCachedFileName(const QString & url, CuteReport::ReportInterface * report)
{
    if (!report)
        return QString();

    CuteReport::StorageInterface * module = storageByUrl(url, report);
    return module ? module->localCachedFileName(url) : QString();
}


QList<StorageObjectInfo> ReportCore::objectsList(const QString &url, CuteReport::ReportInterface * report, QString * errorText)
{
    QList<StorageObjectInfo> list;
    CuteReport::StorageInterface * module = 0;
    if ((report && !checkReportPointer(report, errorText)) || !(module = storageByUrl(url, report)) )
        return list;

    list = module->objectsList(url);
    return list;
}


QList<StorageObjectInfo> ReportCore::objectsList(const QString &url, CuteReport::ReportInterface * report, const QStringList &nameFilters,
                                                 QDir::Filters filters, QDir::SortFlags sort, QString * errorText)
{
    QList<StorageObjectInfo> list;
    CuteReport::StorageInterface * module = 0;
    if ((report && !checkReportPointer(report, errorText)) || !(module = storageByUrl(url, report)) )
        return list;

    list = module->objectsList(url, nameFilters, filters, sort);
    return list;
}


CuteReport::PageInterface * ReportCore::createPageObject(const QString & moduleName, CuteReport::ReportInterface *report)
{
    CuteReport::ReportPluginInterface * plugin = getModuleByName(moduleName, *m_pagePlugins);

    if (!plugin) {
        log(LogError, MODULENAME, "Requested page not found", QString("item: %1; available items: %2").arg(moduleName, moduleNames(PageModule).join(", ")));
        return 0;
    }

    CuteReport::PageInterface * module = static_cast<CuteReport::PageInterface *>(plugin);

    CuteReport::PageInterface * newObject =  module->createInstance(report);
    newObject->setReportCore(this);
    /// do not set name - it could be set from serializer or direct in ReportInterface
    if (report) {
        int maxOrder = -1;
        QList<CuteReport::PageInterface *> pages = report->pages();
        foreach(CuteReport::PageInterface * p, pages)
            if (p != newObject && p->order() > maxOrder)
                maxOrder = p->order();
        newObject->setOrder(maxOrder+1);
    }
    newObject->setParent(report);
    return newObject;
}


CuteReport::BaseItemInterface * ReportCore::createItemObject(const QString & moduleName, CuteReport::ReportInterface *report, QObject * parent)
{
    Q_UNUSED(report)

    CuteReport::ReportPluginInterface * plugin = getModuleByName(moduleName, *m_itemPlugins);

    if (!plugin) {
        log(LogError, MODULENAME, "Requested item not found", QString("item: %1; available items: %2").arg(moduleName, moduleNames(ItemModule).join(", ")));
        return 0;
    }

    CuteReport::BaseItemInterface * module = static_cast<CuteReport::BaseItemInterface *>(plugin);

    if (module) {
        CuteReport::BaseItemInterface * newObject =  module->createInstance(parent);
        newObject->setReportCore(this);
        /// do not set name - it could be set from serializer or direct in ReportInterface
        return newObject;
    }

    return 0;
}


CuteReport::DatasetInterface * ReportCore::createDatasetObject(const QString & moduleName, CuteReport::ReportInterface *report)
{
    CuteReport::ReportPluginInterface * plugin = getModuleByName(moduleName, *m_datasetPlugins);

    if (!plugin) {
        log(LogError, MODULENAME, "Requested dataset not found", QString("item: %1; available items: %2").arg(moduleName, moduleNames(DatasetModule).join(", ")));
        return 0;
    }

    CuteReport::DatasetInterface * module = static_cast<CuteReport::DatasetInterface *>(plugin);

    if (module) {
        CuteReport::DatasetInterface * newObject =  module->createInstance(report);
        newObject->setReportCore(this);
        /// do not set name - it could be set from serializer or direct in ReportInterface
        return newObject;
    }

    return 0;
}


CuteReport::StorageInterface * ReportCore::createStorageObject(const QString & moduleName, CuteReport::ReportInterface *report)
{
    Q_UNUSED(report)

    CuteReport::ReportPluginInterface * plugin = getModuleByName(moduleName, *m_storagePlugins);

    if (!plugin) {
        log(LogError, MODULENAME, "Requested storage not found", QString("item: %1; available items: %2").arg(moduleName, moduleNames(StorageModule).join(", ")));
        return 0;
    }

    CuteReport::StorageInterface * module = static_cast<CuteReport::StorageInterface *>(plugin);

    if (module) {
        /** do not set parent. ReportInterface should send signal for new storage */
        CuteReport::StorageInterface * newObject =  module->createInstance();
        newObject->setReportCore(this);
        /// do not set name - it could be set from serializer or direct in ReportInterface
        return newObject;
    }

    return 0;
}


CuteReport::PrinterInterface * ReportCore::createPrinterObject(const QString & moduleName, CuteReport::ReportInterface *report)
{ 
    Q_UNUSED(report)

    CuteReport::ReportPluginInterface * plugin = getModuleByName(moduleName, *m_printerPlugins);

    if (!plugin) {
        log(LogError, MODULENAME, "Requested printer not found", QString("item: %1; available items: %2").arg(moduleName, moduleNames(PrinterModule).join(", ")));
        return 0;
    }

    CuteReport::PrinterInterface * module = static_cast<CuteReport::PrinterInterface *>(plugin);

    if (module) {
        /** do not set parent. ReportInterface should send signal for new printer */
        CuteReport::PrinterInterface * newObject =  module->createInstance();
        newObject->setReportCore(this);
        /// do not set name - it could be set from serializer or direct in ReportInterface
        return newObject;
    }

    return 0;
}


CuteReport::RendererInterface * ReportCore::createRendererObject(const QString & moduleName, CuteReport::ReportInterface *report)
{
    Q_UNUSED(report)

    CuteReport::ReportPluginInterface * plugin = getModuleByName(moduleName, *m_rendererPlugins);

    if (!plugin) {
        log(LogError, MODULENAME, "Requested renderer not found", QString("item: %1; available items: %2").arg(moduleName, moduleNames(RendererModule).join(", ")));
        return 0;
    }

    CuteReport::RendererInterface * module = static_cast<CuteReport::RendererInterface *>(plugin);

    if (module) {
        /** do not set parent. ReportInterface should send signal for new renderer */
        CuteReport::RendererInterface * newObject =  module->createInstance();
        newObject->setReportCore(this);
        return newObject;
    }

    return 0;
}


CuteReport::FormInterface * ReportCore::createFormObject(const QString & moduleName, ReportInterface *report)
{
    CuteReport::ReportPluginInterface * plugin = getModuleByName(moduleName, *m_formPlugins);

    if (!plugin) {
        log(LogError, MODULENAME, "Requested form not found", QString("item: %1; available items: %2").arg(moduleName, moduleNames(FormModule).join(", ")));
        return 0;
    }

    CuteReport::FormInterface * module = static_cast<CuteReport::FormInterface *>(plugin);

    if (module) {
        CuteReport::FormInterface * newObject =  module->createInstance(report);
        newObject->setReportCore(this);
        /// do not set name - it could be set from serializer or direct in ReportInterface
        return newObject;
    }

    return 0;
}


CuteReport::ExportInterface * ReportCore::createExportObject(const QString & moduleName, CuteReport::ReportInterface *report)
{
    CuteReport::ReportPluginInterface * plugin = getModuleByName(moduleName, m_exporters);

    if (!plugin) {
        log(LogError, MODULENAME, "Requested export not found", QString("item: %1; available items: %2").arg(moduleName, moduleNames(ExportModule).join(", ")));
        return 0;
    }

    CuteReport::ExportInterface * module = static_cast<CuteReport::ExportInterface *>(plugin);

    if (module) {
        CuteReport::ExportInterface * newObject = module->createInstance(report);
        newObject->setReportCore(this);
        /// do not set name - it could be set from serializer or direct in ReportInterface
        return newObject;
    }

    return 0;
}


ScriptExtensionInterface *ReportCore::createScriptExtentionObject(const QString &moduleName, ReportInterface *report, QObject *parent)
{
    Q_UNUSED(report)
    CuteReport::ReportPluginInterface * plugin = getModuleByName(moduleName, *m_scriptExtensionPlugins);

    if (!plugin) {
        log(LogError, MODULENAME, "Requested script extension not found", QString("item: %1; available items: %2").arg(moduleName, moduleNames(ScriptExtensionModule).join(", ")));
        return 0;
    }

    CuteReport::ScriptExtensionInterface * module = static_cast<CuteReport::ScriptExtensionInterface *>(plugin);

    if (module) {
        CuteReport::ScriptExtensionInterface * newObject = module->createInstance(parent);
        newObject->setReportCore(this);
        /// do not set name - it could be set from serializer or direct in ReportInterface
        return newObject;
    }

    return 0;
}


QByteArray ReportCore::serialize(const QObject * object, bool * ok, QString *error, const QString & moduleName)
{
    QByteArray ba;

    CuteReport::ReportPluginInterface * module = getOriginalModuleByName(moduleName, m_serializers);
    if (!module)
        module = m_defaultSerializer;
    if (!module && m_serializers.size())
        module = m_serializers.at(0);

    SerializerInterface * serializer = static_cast<SerializerInterface *>(module);

    if (!serializer) {
        QString errorStr = QString("Serializer \'%1\' not found").arg(moduleName);
        log(LogWarning, MODULENAME, errorStr);
        if (ok)
            *ok = false;
        if (error)
            *error = errorStr;
        return ba;
    }

    ba = serializer->serialize(object, ok);
    if (ok && !(*ok)) {
        if (error)
            *error = serializer->lastError();
    }

    return ba;
}


QObject * ReportCore::deserialize(const QByteArray &data, bool *ok, QString *error, const QString & moduleName)
{
    CuteReport::ReportPluginInterface * module = getOriginalModuleByName(moduleName, m_serializers);
    if (!module)
        module = m_defaultSerializer;
    if (!module && m_serializers.size())
        module = m_serializers.at(0);

    SerializerInterface * serializer = static_cast<SerializerInterface *>(module);

    if (!serializer) {
        QString errorStr = QString("Serializer \'%1\' not found").arg(moduleName);
        log(LogWarning, MODULENAME, errorStr);
        if (ok)
            *ok = false;
        if (error)
            *error = errorStr;
        return 0;
    }

    QObject * object = serializer->deserialize(data, ok);

    if (ok && !(*ok)) {
        if (error)
            *error = serializer->lastError();
    }

    return object;
}


//const QList<QString> ReportCore::renderers()
//{
//    QStringList list;
//    foreach (RendererInterface * r, *m_renderers)
//        list.append(r->moduleFullName());

//    return list;
//}


bool ReportCore::render(ReportInterface* report, const QString & rendererName)
{
    log(LogWarning, MODULENAME, QString("render: report address:\'%1\'; renderer name = \'%2\'").arg((long)report).arg(rendererName));
    QueueReport * qr = new QueueReport();
    qr->report = report;
    qr->destination = RenderToPreview;
    qr->rendererName = rendererName;

    if (m_renderingQueue.size() >= m_maxRenderingThreads) {
        log(LogWarning, MODULENAME, QString("add to queue (size is %1 max is %2)").arg(m_renderingQueue.size()).arg(m_maxRenderingThreads));
        m_waitingQueue.append(qr);
        return true;
    } else
        return _render(qr);
}


bool ReportCore::render(const QString &reportUrl, const QString & rendererName)
{
    log(LogWarning, MODULENAME, QString("render \'%1\'").arg(reportUrl) );
    QueueReport * qr = new QueueReport();
    qr->url = reportUrl;
    qr->destination = RenderToPreview;
    qr->rendererName = rendererName;

    if (m_renderingQueue.size() >= m_maxRenderingThreads) {
        m_waitingQueue.append(qr);
        return true;
    } else
        return _render(qr);
}


bool ReportCore::_render(QueueReport * queueReport)
{
    log(LogDebug, MODULENAME, QString("_render"));
    if (!queueReport->report) {
        queueReport->report = loadReport(queueReport->url);
    }

    if (!queueReport->report) {
        queueReport->success = false;
        _renderDone(queueReport);
        return false;
    }

    RendererInterface * renderer = this->renderer(queueReport->report, queueReport->rendererName);
    if (!renderer) {
        log(LogWarning, MODULENAME, QString("Can't find renderer for report  \'%1\'").arg(queueReport->report->objectName()) );
        queueReport->success = false;
        _renderDone(queueReport);
        return false;
    }

    log(LogDebug, MODULENAME, QString("render to be used: \'%1\'").arg(renderer->moduleFullName()));

    RendererInterface * rendererCopy = renderer->clone();
    m_renderingQueue.insert(rendererCopy, queueReport);

    log(LogDebug, MODULENAME, QString("render copy created"));

    connect (rendererCopy, SIGNAL(started()), this, SLOT(_rendererStarted()), Qt::UniqueConnection);
    connect (rendererCopy, SIGNAL(done(bool)), this, SLOT(_rendererDone(bool)), Qt::UniqueConnection);
    connect (rendererCopy, SIGNAL(processingPage(int,int,int,int)), this, SLOT(_rendererProcessingPage(int,int,int,int)), Qt::UniqueConnection);

    rendererCopy->run(queueReport->report);

    return true;
}


void ReportCore::_renderDone(QueueReport *queueReport)
{
    log(CuteReport::LogDebug, MODULENAME, QString("Renderer done for report \'%1\' %2")
        .arg(queueReport->report ? queueReport->report->objectName() : queueReport->url)
        .arg(queueReport->success ? "without errors" : "with errors" ) );

    if (!queueReport->url.isEmpty())
        emit rendererDone(queueReport->url, queueReport->success);

    emit rendererDone(queueReport->report, queueReport->success);

    switch (queueReport->destination) {
        case RenderToPreview: delete queueReport; break;
        case RenderToExport:  queueReport->success ? _export(queueReport) : _exportDone(queueReport); break;
        case RenderToPrinter: queueReport->success ? _print(queueReport)  : _printDone(queueReport);  break;
    }
}


void ReportCore::_export(QueueReport *queueReport)
{
    QueueReportExport * qr = reinterpret_cast<QueueReportExport*>(queueReport);

    ExportInterface * exportModule = 0;
    foreach (ReportPluginInterface * plugin, modules(ExportModule)) {
        ExportInterface * m = static_cast<ExportInterface*>(plugin);
        if (m->format().toLower() == qr->format) {
            exportModule = m;
            break;
        }
    }

    if (exportModule) {
        ExportInterface * moduleCopy =  exportModule->clone();
        if (!qr->options.isEmpty())
            setModuleOptions(moduleCopy, qr->options);
        moduleCopy->process(qr->report, qr->outputUrl);
        moduleCopy->deleteLater();
    }

    _exportDone(queueReport);
}


void ReportCore::_exportDone(QueueReport *queueReport)
{
    log(CuteReport::LogDebug, MODULENAME, QString("Export done for report: %1 %2")
        .arg(queueReport->report ? queueReport->report->objectName() : queueReport->url)
        .arg(queueReport->success ? "without errors" : "with errors" ) );

    if (!queueReport->url.isEmpty())
        emit exportDone(queueReport->url, queueReport->success);

    emit exportDone(queueReport->report, queueReport->success);

    delete queueReport;
}


void ReportCore::_print(QueueReport *queueReport)
{
    QueueReport * qr = queueReport;

    PrinterInterface * printer = qr->report->printer(qr->destinationName);
    if (!printer)
        printer = static_cast<PrinterInterface *>(m_defaultPrinter.data());

    if (printer) {
        printer->print(qr->report);
    } else {
        qr->success = true;
        log(LogWarning, MODULENAME, QString("Specified printer not found"), QString("Can't find printer:\'%1\' for report \'%2\'")
            .arg(qr->destinationName, qr->report->objectName()));
    }

    _printDone(queueReport);
}


void ReportCore::_printDone(QueueReport *queueReport)
{
    log(CuteReport::LogDebug, MODULENAME, QString("Printing done for report \'%1\' %2")
        .arg(queueReport->report ? queueReport->report->objectName() : queueReport->url)
        .arg(queueReport->success ? "without errors" : "with errors" ) );

    if (!queueReport->url.isEmpty())
        emit printingDone(queueReport->url, queueReport->success);

    emit printingDone(queueReport->report, queueReport->success);

    delete queueReport;
}


void ReportCore::stopRendering(ReportInterface* report, bool silent)
{
    log(LogWarning, MODULENAME, QString("Attempting to stop report rendering: \'%1\'").arg(report->objectName()) );

    bool exists = false;
    QHashIterator<RendererInterface*, QueueReport*> it (m_renderingQueue);
    while (it.hasNext()) {
        it.next();
        RendererInterface * renderer = it.key();
        if (renderer->report() == report) {
            log(LogWarning, MODULENAME, QString("Working renderer found for report: \'%1\'").arg(report->objectName()) );
            renderer->stop();
            exists = true;
        }
    }

    if (!exists && !silent)
        log(LogWarning, MODULENAME, QString("Working renderer not found for report: \'%1\'").arg(report->objectName()) );
}


bool ReportCore::isRendered(ReportInterface *report)
{
    foreach (RendererInterface * renderer, m_renderingQueue.keys()) {
        if (renderer->report() == report)
            return true;
    }
    return false;
}


int ReportCore::rendererTotalPages(ReportInterface * report) const
{
    if (!report)
        return 0;

    RenderedReportInterface * renderedReport = report->renderedReport();
    return renderedReport ? renderedReport->pageCount() : 0;
}


RenderedPageInterface *ReportCore::rendererGetPage(ReportInterface * report, int number) const
{
    if (!report)
        return 0;

    RenderedReportInterface * renderedReport = report->renderedReport();
    return renderedReport ? renderedReport->page(number) : 0;
}


void ReportCore::renderDataClear(ReportInterface * report)
{
    if (!report)
        return;

    report->clearRenderedReport();
}


void ReportCore::print(ReportInterface* report, const QString &printerName)
{
    QueueReport * qr = new QueueReport();
    qr->report = report;
    qr->destination = RenderToPrinter;
    qr->destinationName = printerName;

    if (m_renderingQueue.size() > m_maxRenderingThreads) {
        m_waitingQueue.append(qr);
        return;
    } else
        _print(qr);
}


void ReportCore::print(const QString url, const QString & printerName)
{
    QueueReport * qr = new QueueReport();
    qr->url = url;
    qr->destination = RenderToPrinter;
    qr->destinationName = printerName;

    if (m_renderingQueue.size() > m_maxRenderingThreads) {
        m_waitingQueue.append(qr);
        return;
    } else
        _print(qr);
}


QStringList ReportCore::importExtensions() const
{
    QStringList list;
    foreach (ReportPluginInterface * plugin, *m_importPlugins) {
        ImportInterface * module = static_cast<ImportInterface*>(plugin);
        list << module->fileExtensions();
    }
    return list;
}


bool ReportCore::canImport(const QString &reportUrl) const
{
    foreach (ReportPluginInterface * plugin, *m_importPlugins) {
        ImportInterface * module = static_cast<ImportInterface*>(plugin);
        if (module->canHandle(reportUrl))
            return true;
    }
    return false;
}


QStringList ReportCore::importModulesForFile(const QString &reportUrl) const
{
    QStringList list;
    foreach (ReportPluginInterface * plugin, *m_importPlugins) {
        ImportInterface * module = static_cast<ImportInterface*>(plugin);
        if (module->canHandle(reportUrl))
            list << module->moduleFullName();
    }
    return list;
}


ReportInterface *ReportCore::import(const QString &reportUrl, const QString &moduleName) const
{
    const ImportInterface * m = static_cast<const ImportInterface *>(module(ImportModule, moduleName));

    if (!m) {
        QList<ImportInterface*> list;
        foreach (ReportPluginInterface * plugin, *m_importPlugins) {
            ImportInterface * module = static_cast<ImportInterface*>(plugin);
            if (module->canHandle(reportUrl))
                list << module;
        }
        if (list.size() > 0)
            m = list.at(0);
    }

    ReportInterface * report = m ? m->importReport(reportUrl) : 0;
    return report;
}


void ReportCore::exportTo(ReportInterface *report, const QString &format, const QString &outputUrl, const QStringList &options)
{
    QueueReportExport * qr = new QueueReportExport();
    qr->report = report;
    qr->destination = RenderToExport;
    qr->format = format.toLower();
    qr->outputUrl = outputUrl;
    qr->options = options;

    if (m_renderingQueue.size() > m_maxRenderingThreads) {
        m_waitingQueue.append(qr);
        return;
    } else
        _render(qr);
}


void ReportCore::exportTo(const QString &reportUrl, const QString &format, const QString &outputUrl, const QStringList &options)
{
    QueueReportExport * qr = new QueueReportExport();
    qr->url = reportUrl;
    qr->destination = RenderToExport;
    qr->format = format;
    qr->outputUrl = outputUrl;
    qr->options = options;

    if (m_renderingQueue.size() > m_maxRenderingThreads) {
        m_waitingQueue.append(qr);
        return;
    } else
        _render(qr);
}


bool ReportCore::isNameUnique(QObject * object, const QString & name, QObject * rootObject)
{
    if (!rootObject)
        return true;
    if (rootObject->objectName() == name)
        return false;

    foreach(QObject * o, rootObject->children()) {
//        qDebug() << "un name: " << o->objectName();
        if (object != o && !isNameUnique(object, name, o))
            return false;
    }
    return true;
}


QString ReportCore::uniqueName(QObject * object, const QString & proposedName, QObject * rootObject)
{
    QString name = proposedName.isEmpty() ? object->metaObject()->className() : proposedName;
    QString newName = name.section("::", -1,-1);
    if (isNameUnique(object, newName, rootObject))
        return newName;

    QString baseName = newName;
    baseName.remove(QRegExp("\\d$"));
    QString indexString = newName;
    indexString.remove(0,baseName.length());
    int number = indexString.toInt();
    if (number == 0)
        number = 1;

    QString joint = indexString.isEmpty() ? "_" : "";

    forever {
        QString nName = baseName + joint + QString::number(number, 10);
        if (isNameUnique(object, nName , rootObject ))
            return nName;
        number++;
    }

    return newName;
}


void ReportCore::_rendererStarted()
{
    RendererInterface * renderer = reinterpret_cast<RendererInterface *>(sender());
    if (!renderer)
        return;

    log(CuteReport::LogDebug, MODULENAME, "Renderer started for report: " + renderer->report()->objectName());
    emit rendererStarted(renderer->report());
}


void ReportCore::_rendererDone(bool successful)
{
    RendererInterface * renderer = reinterpret_cast<RendererInterface *>(sender());
    if (!renderer)
        return;

    CuteReport::RenderedReportInterface * renderedReport = renderer->takeRenderedReport();
    renderer->report()->setRenderedReport( renderedReport );

    QueueReport * qr = m_renderingQueue.take(renderer);
    renderer->deleteLater();
    qr->success = successful;
    _renderDone(qr);

    if (m_waitingQueue.size() != 0 && m_renderingQueue.size() < m_maxRenderingThreads) {
        QueueReport * qr = m_waitingQueue.takeFirst();
        qr->success = successful;
        _render(qr);
    }
}


void ReportCore::_rendererMessage(int logLevel, QString message)
{
    RendererInterface * renderer = reinterpret_cast<RendererInterface *>(sender());
    if (!renderer)
        return;
    log(CuteReport::LogDebug, MODULENAME, QString("Renderer message for report \'%1\' : %2").arg(renderer->report()->objectName()).arg(message) );
    emit rendererMessage(renderer->report(), logLevel, message);
}


void ReportCore::_rendererProcessingPage(int page, int total, int pass, int passTotal)
{
    RendererInterface * renderer = reinterpret_cast<RendererInterface *>(sender());
    if (!renderer)
        return;
    log(CuteReport::LogDebug, MODULENAME, QString("Rendering report \'%1\': page %2 of %3").arg(renderer->report()->objectName()).arg(page).arg(total) );
    emit rendererProcessingPage(renderer->report(), page, total, pass, passTotal);
}


void ReportCore::_reportObjectCreated(CuteReport::ReportInterface * report)
{
    if (!report)
        return;

    report->setParent(this);
    /// if looking for aa unique name it search for every report copy created in renderer
    //report->setObjectName(uniqueName(report, report->objectName().isEmpty() ? "report" : report->objectName(), this));

//        connect(report, SIGNAL(destroyed(QObject*)), this, SLOT(_reportObjectDestroyed(QObject*)), Qt::UniqueConnection);
    emit reportObjectCreated(report);
}


void ReportCore::log(LogLevel level, const QString & sender, const QString & message )
{
    if (Log::instance())
        Log::instance()->push(level, sender, message);
}


void ReportCore::log(LogLevel level, const QString & sender, const QString & shortMessage, const QString & fullMessage)
{
    if (Log::instance())
        Log::instance()->push(level, sender, shortMessage, fullMessage);
}


void ReportCore::sendMetric(MetricType type, const QVariant &value)
{
    emit this->metricUpdated(type, value);
}


int ReportCore::maxRenderingThreads() const
{
    return m_maxRenderingThreads;
}


void ReportCore::setMaxRenderingThreads(int maxRenderingThreads)
{
    m_maxRenderingThreads = maxRenderingThreads;
}


ScriptString ReportCore::processString(ScriptString &scriptString)
{
    RendererInterface * renderer = getRendererForReport(scriptString.report, &scriptString.error);
    if (!renderer) {
        scriptString.result = false;
        return scriptString;
    }
    return renderer->processString(scriptString);
}


ScriptString ReportCore::processString(const ScriptString &scriptString)
{
    ScriptString scriptStr(scriptString);

    // TODO: try to find active renderer for report

    RendererInterface * renderer = getRendererForReport(scriptStr.report, &scriptStr.error);
    if (!renderer) {
        scriptStr.result = false;
        return scriptStr;
    }
    return renderer->processString(scriptStr);
}


QSet<QString> ReportCore::getReportParameters(ReportInterface *report, QString *error)
{
    QSet<QString> set;
    RendererInterface * renderer = getRendererForReport(report, error);
    if (!renderer)
        return set;

    return renderer->reportParameters(report);
}


RendererInterface * ReportCore::getRendererForReport(ReportInterface *report, QString *error)
{
    if (!report) {
        if (error)
            *error = tr("Report object is not specified");
        return 0;
    }

    RendererInterface * renderer = this->renderer(report);

    if (!renderer) {
        if (error)
            *error = tr("Renderer is not found for report \'%1\'").arg(report->objectName());
        return 0;
    }

    return renderer;
}

} //namespace
