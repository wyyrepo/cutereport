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

#include "reportinterface.h"
#include "pageinterface.h"
#include "iteminterface.h"
#include "datasetinterface.h"
#include "forminterface.h"
#include "rendererinterface.h"
#include "printerinterface.h"
#include "storageinterface.h"
#include "log/log.h"
#include "cutereport_globals.h"
#include "renderedreportinterface.h"
#include "reportcore.h"
#include "scriptengineinterface.h"
#include "inheritancedatacontainer.h"

using namespace CuteReport;

static const char* SENDERNAME = "ReportInterface";

ReportInterface::ReportInterface(ReportCore *reportCore)
    : QObject(reportCore),
      //      m_reportCore(0),
      m_version(0.0),
      m_isDirty(false),
      m_isValid(true),
      m_inheritanceData(new InheritanceDataContainer()),
      m_renderedReport(0),
      m_isBeingRendered(false)
{
    Log::refCounterInc(this);
}


ReportInterface::ReportInterface(const ReportInterface & dd, ReportCore *reportCore)
    :QObject(reportCore),
      //      m_reportCore(dd.m_reportCore),
      m_name(dd.m_name),
      m_author(dd.m_author),
      m_script(dd.m_script),
      m_version(dd.m_version),
      m_description(dd.m_description),
      m_fileUrl(dd.m_fileUrl),
      m_defaultStorageName(dd.m_defaultStorageName),
      m_variables(dd.m_variables),
      m_flags(0),
      m_isDirty(dd.m_isDirty),
      m_isValid(dd.m_isValid),
      m_inheritanceData(new InheritanceDataContainer(*(dd.m_inheritanceData))),
      m_renderedReport(0),
      m_isBeingRendered(false)
{
    Log::refCounterInc(this);
    setFlags(dd.m_flags);
}


ReportInterface::~ReportInterface()
{
    ReportCore * core = qobject_cast<ReportCore*>(parent());
    if (core)
        core->stopRendering(this, true);
    delete m_renderedReport;
    delete m_inheritanceData;
    Log::refCounterDec(this);
}


void ReportInterface::init()
{
//    QList<CuteReport::ReportPluginInterface*> modules = this->findChildren<CuteReport::ReportPluginInterface*>();
    //qDebug() << modules.count();
    QList<CuteReport::PageInterface*> pages = this->findChildren<CuteReport::PageInterface*>();
    foreach(CuteReport::PageInterface* page, pages) {
        page->init();
        connect(page, SIGNAL(destroyed(QObject*)), this, SLOT(childDestroyed(QObject*)));
        connect(page, SIGNAL(afterNewItemAdded(CuteReport::BaseItemInterface*)),
                this, SLOT(slotItemAdded(CuteReport::BaseItemInterface*)));
        connect(page, SIGNAL(afterItemRemoved(CuteReport::BaseItemInterface*,QString,bool)),
                this, SLOT(slotItemRemoved(CuteReport::BaseItemInterface*, QString, bool)));
    }

    //    QList<CuteReport::BaseItemInterface*> items = this->findChildren<CuteReport::BaseItemInterface*>();
    //    foreach(CuteReport::BaseItemInterface* item, items) {
    //        item->init();
    //    }

    QList<CuteReport::DatasetInterface*> datasets = this->findChildren<CuteReport::DatasetInterface*>();
    foreach(CuteReport::DatasetInterface* dataset, datasets) {
        dataset->init();
        connect(dataset, SIGNAL(destroyed(QObject*)), this, SLOT(childDestroyed(QObject*)));
    }

    QList<CuteReport::FormInterface*> forms = this->findChildren<CuteReport::FormInterface*>();
    foreach(CuteReport::FormInterface* form, forms) {
        form->init();
        connect(form, SIGNAL(destroyed(QObject*)), this, SLOT(childDestroyed(QObject*)));
    }

    /// TODO: remove this workaround in v1.4
    m_moduleVariables = customData("moduleVariables").toStringList().toSet();

    precessFlags(m_flags);
}


ReportInterface * ReportInterface::clone(bool withChildren, bool init)
{
    ReportInterface * newReport = new ReportInterface(*this, reinterpret_cast<CuteReport::ReportCore*>(this->parent()));
    if (withChildren) {
        foreach (QObject * child, children()) {
            if (PageInterface * page = dynamic_cast<PageInterface*>(child)) {
                PageInterface * newPage = page->clone(true, false);
                newPage->setParent(newReport);
                newPage->setObjectName(page->objectName());
            }
            if (DatasetInterface * ds = dynamic_cast<DatasetInterface*>(child)) {
                DatasetInterface * newds = ds->clone();
                newds->setParent(newReport);
                newds->setObjectName(ds->objectName());
            }
            if (FormInterface * form = dynamic_cast<FormInterface*>(child)) {
                FormInterface * newForm = form->clone();
                newForm->setParent(newReport);
                newForm->setObjectName(form->objectName());
            }
            if (RendererInterface * renderer = dynamic_cast<RendererInterface*>(child)) {
                RendererInterface * newRenderer = renderer->clone();
                newRenderer->setParent(newReport);
                newRenderer->setObjectName(renderer->objectName());
            }
            if (PrinterInterface * printer = dynamic_cast<PrinterInterface*>(child)) {
                PrinterInterface * newPrinter = printer->clone();
                newPrinter->setParent(newReport);
                newPrinter->setObjectName(printer->objectName());
            }
            if (StorageInterface * storage = dynamic_cast<StorageInterface*>(child)) {
                StorageInterface * newStorage = storage->clone();
                newStorage->setParent(newReport);
                newStorage->setObjectName(storage->objectName());
            }
        }
    }

    if (init)
        newReport->init();

    return newReport;
}


QString ReportInterface::script()
{
    return m_script;
}


void ReportInterface::setScript(const QString & script)
{
    if (m_script == script)
        return;

    m_script = script;

    emit scriptChanged(m_script);
    emit changed();
    emit propertyChanged();
}


QString ReportInterface::name()
{
    return m_name;
}


void ReportInterface::setName(const QString & name)
{
    if (m_name == name)
        return;
    m_name = name;

    emit nameChanged(m_name);
    emit changed();
    emit propertyChanged();
}


QString ReportInterface::author()
{
    return m_author;
}


void ReportInterface::setAuthor(const QString & author)
{
    if (m_author == author)
        return;
    m_author = author;

    emit authorChanged(m_author);
    emit changed();
    emit propertyChanged();
}


QList<BaseItemInterface *> ReportInterface::items(const QString &pageObjectName)
{
    if (pageObjectName.isEmpty()) {
        QList<BaseItemInterface *> items;
        foreach (PageInterface * page, pages()) {
            items << page->items();
        }
        return items;
    }

    const PageInterface * p = page(pageObjectName);
    if (!p)
        return QList<BaseItemInterface *>();

    return p->items();
}


CuteReport::BaseItemInterface * ReportInterface::item (const QString & itemName)
{
    foreach (PageInterface* page, pages())
        foreach (BaseItemInterface * item, page->items()) {
            if (item->objectName() == itemName) {
                return item;
            }
        }
    return 0;
}


QStringList ReportInterface::itemNames(const QString &pageObjectName)
{
    QStringList list;
    foreach (BaseItemInterface * item, items(pageObjectName))
        list << item->objectName();
    return list;
}


QList<PageInterface *> ReportInterface::pages()
{
#if QT_VERSION < 0x050000
    QList<CuteReport::PageInterface*> pages = findChildren<CuteReport::PageInterface*>();
    QList<CuteReport::PageInterface*> pagesFinal;
    foreach (CuteReport::PageInterface * page, pages) {
        if (page->parent() == this)
            pagesFinal << page;
    }
    return pagesFinal;
#else
    return findChildren<CuteReport::PageInterface*>(QString(), Qt::FindDirectChildrenOnly);
#endif
}


QStringList ReportInterface::pageNames()
{
    QStringList list;
    foreach (PageInterface * page, pages())
        list << page->objectName();
    return list;
}


PageInterface *ReportInterface::page(const QString &pageName)
{
    if (pageName.isEmpty())
        return 0;
#if QT_VERSION < 0x050000
    QList<CuteReport::PageInterface*> pages = findChildren<CuteReport::PageInterface*>(pageName);
    QList<CuteReport::PageInterface*> pagesFinal;
    foreach (CuteReport::PageInterface * page, pages) {
        if (page->parent() == this)
            pagesFinal << page;
    }
    return pagesFinal.isEmpty() ? 0 : pagesFinal.first();
#else
    return findChild<CuteReport::PageInterface*>(pageName, Qt::FindDirectChildrenOnly);
#endif
}


void ReportInterface::addPage (PageInterface * page)
{
    if (!page) {
        ReportCore::log(CuteReport::LogWarning, SENDERNAME, "NULL pointer is passed to addPage");
        return;
    }
    /// FIXME: this might change behaviour and break compatibility - fix in v 1.4
//    else if (page->parent()) {
//        ReportCore::log(CuteReport::LogWarning, SENDERNAME,
//                        "addPage method got a pointer to object that is already assigned to the report");
//        return;
//    }

    page->setParent(this);
    setUniqueName(page);

    connect(page, SIGNAL(destroyed(QObject*)), this, SLOT(childDestroyed(QObject*)));
    connect(page, SIGNAL(afterNewItemAdded(CuteReport::BaseItemInterface*)),
            this, SLOT(slotItemAdded(CuteReport::BaseItemInterface*)));
    connect(page, SIGNAL(afterItemRemoved(CuteReport::BaseItemInterface*,QString,bool)),
            this, SLOT(slotItemRemoved(CuteReport::BaseItemInterface*, QString, bool)));

    if (m_flags.testFlag(DirtynessAutoUpdate))
        connect(page, SIGNAL(changed()), this, SLOT(setDirty()));

    emit pageAdded(page);
    emit changed();
}


void ReportInterface::deletePage(PageInterface * page)
{
    if (!page || page->parent() != this)
        return;

    /** disconnect for prevent triggering destroyed object postprocessing */
    disconnect(page, 0, this, 0);
    page->deleteLater();

    emit pageDeleted(page);
    emit changed();
}


void ReportInterface::deletePage(const QString &pageName)
{
    deletePage(page(pageName));
}


QList<DatasetInterface *> ReportInterface::datasets()
{
#if QT_VERSION < 0x050000
    QList<CuteReport::DatasetInterface*> datasets = findChildren<CuteReport::DatasetInterface*>();
    QList<CuteReport::DatasetInterface*> datasetsFinal;
    foreach (CuteReport::DatasetInterface * dataset, datasets) {
        if (dataset->parent() == this)
            datasetsFinal << dataset;
    }
    return datasetsFinal;
#else
    return findChildren<CuteReport::DatasetInterface*>(QString(), Qt::FindDirectChildrenOnly);
#endif
}


QStringList ReportInterface::datasetNames()
{
    QStringList list;
    foreach (DatasetInterface * ds, datasets())
        list << ds->objectName();
    return list;
}


DatasetInterface * ReportInterface::dataset(const QString & datasetName)
{
    if (datasetName.isEmpty())
        return 0;
#if QT_VERSION < 0x050000
    QList<CuteReport::DatasetInterface*> datasets = findChildren<CuteReport::DatasetInterface*>(datasetName);
    QList<CuteReport::DatasetInterface*> datasetsFinal;
    foreach (CuteReport::DatasetInterface * dataset, datasets) {
        if (dataset->parent() == this)
            datasetsFinal << dataset;
    }
    return datasetsFinal.isEmpty() ? 0 : datasetsFinal.first();
#else
    return findChild<CuteReport::DatasetInterface*>(datasetName, Qt::FindDirectChildrenOnly);
#endif
}


void ReportInterface::addDatasets(QList<DatasetInterface *> datasets)
{
    foreach (DatasetInterface * dataset, datasets) {
        addDataset(dataset);
    }
}


void ReportInterface::addDataset(DatasetInterface* dataset)
{
    if (!dataset) {
        ReportCore::log(CuteReport::LogWarning, SENDERNAME, "NULL pointer is passed to addDataset");
        return;
    }
    /// FIXME: this might change behaviour and break compatibility - fix in v 1.4
//    else if (dataset->parent()) {
//        ReportCore::log(CuteReport::LogWarning, SENDERNAME,
//                        "addDataset method got a pointer to object that is already assigned to the report");
//        return;
//    }

    dataset->setParent(this);
    setUniqueName(dataset, "data");

    connect(dataset, SIGNAL(destroyed(QObject*)), this, SLOT(childDestroyed(QObject*)));
    if (m_flags.testFlag(VariablesAutoUpdate))
        connect(dataset, SIGNAL(renderingStringsChanged()), this, SLOT(slotScriptStringsChanged()), Qt::UniqueConnection);
    if (m_flags.testFlag(DirtynessAutoUpdate))
        connect(dataset, SIGNAL(changed()), this, SLOT(setDirty()));

    emit datasetAdded(dataset);
    emit changed();
}


void ReportInterface::deleteDataset(DatasetInterface* dataset)
{
    if (!dataset)
        return;

    /** disconnect for prevent triggering destroyed object postprocessing */
    disconnect(dataset, 0, this, 0);

    QString name = dataset->objectName();
    delete dataset;
    dataset = 0;

    emit datasetDeleted(dataset);
    emit datasetDeleted(name);
    emit changed();
}


void ReportInterface::deleteDataset(const QString &datasetName)
{
    deleteDataset(dataset(datasetName));
}


QList<FormInterface*> ReportInterface::forms()
{
#if QT_VERSION < 0x050000
    QList<CuteReport::FormInterface*> forms = findChildren<CuteReport::FormInterface*>();
    QList<CuteReport::FormInterface*> formsFinal;
    foreach (CuteReport::FormInterface * form, forms) {
        if (form->parent() == this)
            formsFinal << form;
    }
    return formsFinal;
#else
    return findChildren<CuteReport::FormInterface*>(QString(), Qt::FindDirectChildrenOnly);
#endif
}


FormInterface* ReportInterface::form(const QString & formName)
{
#if QT_VERSION < 0x050000
    QList<CuteReport::FormInterface*> forms = findChildren<CuteReport::FormInterface*>(formName);
    QList<CuteReport::FormInterface*> formsFinal;
    foreach (CuteReport::FormInterface * form, forms) {
        if (form->parent() == this)
            formsFinal << form;
    }
    return formsFinal.isEmpty() ? 0 : formsFinal.first();
#else
    return findChild<CuteReport::FormInterface*>(formName, Qt::FindDirectChildrenOnly);
#endif
}


void ReportInterface::addForm(FormInterface * form)
{
    if (!form) {
        ReportCore::log(CuteReport::LogWarning, SENDERNAME, "NULL pointer is passed to addForm");
        return;
    }
    /// FIXME: this might change behaviour and break compatibility - fix in v 1.4
//    else if (form->parent()) {
//        ReportCore::log(CuteReport::LogWarning, SENDERNAME,
//                        "addForm method got a pointer to object that is already assigned to the report");
//        return;
//    }

    form->setParent(this);

    connect(form, SIGNAL(destroyed(QObject*)), this, SLOT(childDestroyed(QObject*)));
    if (m_flags.testFlag(VariablesAutoUpdate))
        connect(form, SIGNAL(scriptingStringsChanged()), this, SLOT(slotScriptStringsChanged()), Qt::UniqueConnection);

    emit formAdded(form);
    emit changed();
}


void ReportInterface::deleteForm(FormInterface * form )
{
    /** disconnect to prevent triggering destroyed object postprocessing */
    disconnect(form, 0, this, 0);
    delete form;
    form = 0;

    emit formDeleted(form);
    emit changed();
}


QList<RendererInterface *> ReportInterface::renderers()
{
#if QT_VERSION < 0x050000
    QList<CuteReport::RendererInterface*> renderers = findChildren<CuteReport::RendererInterface*>();
    QList<CuteReport::RendererInterface*> renderersFinal;
    foreach (CuteReport::RendererInterface * renderer, renderers) {
        if (renderer->parent() == this)
            renderersFinal << renderer;
    }
    return renderersFinal;
#else
    return findChildren<CuteReport::RendererInterface*>(QString(), Qt::FindDirectChildrenOnly);
#endif
}


QStringList ReportInterface::rendererNames()
{
    QStringList list;
    foreach (RendererInterface * rend, renderers())
        list << rend->objectName();
    return list;
}


RendererInterface * ReportInterface::renderer(const QString & rendererName)
{
    QString name = rendererName.isEmpty() ? m_defaultRendererName : rendererName;
    return findChild<CuteReport::RendererInterface*>(name);
}


void ReportInterface::addRenderer(CuteReport::RendererInterface * renderer)
{
    if (!renderer) {
        ReportCore::log(CuteReport::LogWarning, SENDERNAME, "NULL pointer is passed to addRenderer");
        return;
    }
    /// FIXME: this might change behaviour and break compatibility - fix in v 1.4
//    else if (renderer->parent()) {
//        ReportCore::log(CuteReport::LogWarning, SENDERNAME,
//                        "addRenderer method got a pointer to object that is already assigned to the report");
//        return;
//    }

    renderer->setParent(this);
    setUniqueName(renderer);

    connect(renderer, SIGNAL(destroyed(QObject*)), this, SLOT(childDestroyed(QObject*)));
    if (m_flags.testFlag(DirtynessAutoUpdate))
        connect(renderer, SIGNAL(changed()), this, SLOT(setDirty()));

    emit rendererAdded(renderer);
    emit changed();
}


void ReportInterface::deleteRenderer(RendererInterface *renderer)
{
    if (!renderer || renderer->parent() != this)
        return;

    /** disconnect for prevent triggering destroyed object postprocessing */
    disconnect(renderer, 0, this, 0);

    QString name = renderer->objectName();
    delete renderer;
    renderer = 0;

    emit rendererDeleted(renderer);
    emit rendererDeleted(name);
    emit changed();
}


void ReportInterface::deleteRenderer(const QString &rendererName)
{
    deleteRenderer(renderer(rendererName));
}


QString ReportInterface::defaultRendererName() const
{
    return m_defaultRendererName;
}


void ReportInterface::setDefaultRendererName(const QString &name)
{
    if (m_defaultRendererName == name)
        return;

    m_defaultRendererName = name;

    emit defaultRendererChanged(m_defaultRendererName);
    emit changed();
    emit propertyChanged();
}


QList<PrinterInterface *> ReportInterface::printers()
{
#if QT_VERSION < 0x050000
    QList<CuteReport::PrinterInterface*> printers = findChildren<CuteReport::PrinterInterface*>();
    QList<CuteReport::PrinterInterface*> printersFinal;
    foreach (CuteReport::PrinterInterface * printer, printers) {
        if (printer->parent() == this)
            printersFinal << printer;
    }
    return printersFinal;
#else
    return findChildren<CuteReport::PrinterInterface*>(QString(), Qt::FindDirectChildrenOnly);
#endif
}


QStringList ReportInterface::printerNames()
{
    QStringList list;
    foreach (PrinterInterface * printer, printers())
        list << printer->objectName();
    return list;
}


PrinterInterface * ReportInterface::printer(const QString & printerName)
{
#if QT_VERSION < 0x050000
    QList<CuteReport::PrinterInterface*> printers = findChildren<CuteReport::PrinterInterface*>(printerName);
    QList<CuteReport::PrinterInterface*> printersFinal;
    foreach (CuteReport::PrinterInterface * printer, printers) {
        if (printer->parent() == this)
            printersFinal << printer;
    }
    return printersFinal.isEmpty() ? 0 : printersFinal.first();
#else
    return findChild<CuteReport::PrinterInterface*>(printerName, Qt::FindDirectChildrenOnly);
#endif
}


void ReportInterface::addPrinter(PrinterInterface *printer)
{
    if (!printer) {
        ReportCore::log(CuteReport::LogWarning, SENDERNAME, "NULL pointer is passed to addPrinter");
        return;
    }
    /// FIXME: this might change behaviour and break compatibility - fix in v 1.4
//    else if (printer->parent()) {
//        ReportCore::log(CuteReport::LogWarning, SENDERNAME,
//                        "addPrinter method got a pointer to object that is already assigned to the report");
//        return;
//    }

    printer->setParent(this);
    setUniqueName(printer);

    connect(printer, SIGNAL(destroyed(QObject*)), this, SLOT(childDestroyed(QObject*)));
    if (m_flags.testFlag(DirtynessAutoUpdate))
        connect(printer, SIGNAL(changed()), this, SLOT(setDirty()));

    emit printerAdded(printer);
    emit changed();
}


void ReportInterface::deletePrinter(PrinterInterface *printer)
{
    if (!printer || printer->parent() != this)
        return;

    /** disconnect for prevent triggering destroyed object postprocessing */
    disconnect(printer, 0, this, 0);

    QString name = printer->objectName();
    delete printer;
    printer = 0;

    emit printerDeleted(printer);
    emit printerDeleted(name);
    emit changed();
}


void ReportInterface::deletePrinter(const QString &printerName)
{
    deletePrinter(printer(printerName));
}


QString ReportInterface::defaultPrinterName() const
{
    return m_defaultPrinterName;
}


void ReportInterface::setDefaultPrinterName(const QString &name)
{
    if (m_defaultPrinterName == name)
        return;

    m_defaultPrinterName = name;

    emit defaultPrinterChanged(m_defaultPrinterName);
    emit changed();
    emit propertyChanged();
}


StorageInterface *ReportInterface::storage(const QString &objectName) const
{
    StorageInterface * module = 0;
    foreach (StorageInterface * m, storages()) {
        if (m->objectName() == objectName) {
            module = m;
            break;
        }
    }

    return module;
}


StorageInterface *ReportInterface::storageByUrl(const QString &url) const
{
    QString storageName = url.section(':', 0,0);
    return storage(storageName);
}


QList<StorageInterface *> ReportInterface::storageListByScheme(const QString & scheme) const
{
    QList<StorageInterface *> resultList;
    foreach (StorageInterface * m, storages()) {
        if (m->urlScheme() == scheme)
            resultList << m;
    }
    return resultList;
}


QList<StorageInterface *> ReportInterface::storageListByModuleName(const QString & moduleName) const
{
    QList<StorageInterface *> resultList;
    foreach (StorageInterface * m, storages()) {
        if (m->moduleFullName() == moduleName)
            resultList << m;
    }
    return resultList;
}


QList<StorageInterface *> ReportInterface::storages() const
{
#if QT_VERSION < 0x050000
    QList<CuteReport::StorageInterface*> storages = findChildren<CuteReport::StorageInterface*>();
    QList<CuteReport::StorageInterface*> storagesFinal;
    foreach (CuteReport::StorageInterface * storage, storages) {
        if (storage->parent() == this)
            storagesFinal << storage;
    }
    return storagesFinal;
#else
    return findChildren<CuteReport::StorageInterface*>(QString(), Qt::FindDirectChildrenOnly);
#endif
}


QStringList ReportInterface::storageNames() const
{
    QStringList list;
    foreach (StorageInterface * storage, storages())
        list << storage->objectName();
    return list;
}


void ReportInterface::addStorage(StorageInterface *storage)
{
    if (!storage) {
        ReportCore::log(CuteReport::LogWarning, SENDERNAME, "NULL pointer is passed to addStorage");
        return;
    }
    /// FIXME: this might change behaviour and break compatibility - fix in v 1.4
//    else if (storage->parent()) {
//        ReportCore::log(CuteReport::LogWarning, SENDERNAME,
//                        "addStorage method got a pointer to object that is already assigned to the report");
//        return;
//    }

    storage->setParent(this);
    storage->setObjectName( ReportCore::uniqueName(storage, storage->urlScheme(), this) );

    emit storageAdded(storage);
    emit changed();
}


void ReportInterface::deleteStorage(const QString & storageName)
{
    deleteStorage(storage(storageName));
}


void ReportInterface::deleteStorage(StorageInterface * storage)
{
    if (!storage || storage->parent() != this)
        return;

//    if (defaultStorageName() == storage->objectName())
//        setDefaultStorageName(QString());

    QString name = storage->objectName();
    delete storage;

    emit storageDeleted(storage);
    emit storageDeleted(name);
    emit changed();
}


bool ReportInterface::hasStorageModule(const QString & moduleName)
{
    QList<CuteReport::StorageInterface*> storages = findChildren<CuteReport::StorageInterface*>();
    CuteReport::StorageInterface* existentStorage = 0;
    foreach(CuteReport::StorageInterface* st, storages)
        if (moduleName == st->moduleFullName()) {
            existentStorage = st;
            break;
        }

    return existentStorage;
}


QString ReportInterface::defaultStorageName() const
{
    return m_defaultStorageName;
}


void ReportInterface::setDefaultStorageName(const QString & name)
{
    if (m_defaultStorageName == name)
        return;

    m_defaultStorageName = name;

    emit defaultStorageChanged(m_defaultStorageName);
    emit changed();
    emit propertyChanged();
}



// FIXMI: not imlemented report file version checking
double ReportInterface::version()
{
    return m_version;
}


void ReportInterface::setVersion(double tVersion)
{
    m_version = tVersion;
}


int ReportInterface::suitId()
{
    return 0;
}


QString ReportInterface::description()
{
    return m_description;
}


void ReportInterface::setDescription(const QString & description)
{
    if (m_description == description)
        return;

    m_description = description;

    emit descriptionChanged(m_description);
    emit changed();
    emit propertyChanged();
}


QString ReportInterface::fileUrl()
{
    return m_fileUrl;
}


void ReportInterface::setFileUrl(const QString & filePath)
{
    if (filePath == m_fileUrl)
        return;

    m_fileUrl = filePath;

    emit fileUrlChanged(m_fileUrl);
    emit changed();
    emit propertyChanged();
}


const QVariantHash &ReportInterface::variables()
{
    return m_variables;
}


void ReportInterface::setVariables (const QVariantHash &vars)
{
    if (m_variables == vars)
        return;

    m_variables = vars;

    emit variablesChanged();
    emit changed();
}


void ReportInterface::setVariableValue(const QString & name, const QVariant & value)
{
    if (m_variables.contains(name) && m_variables.value(name) == value)
        return;

    QVariant oldVar = m_variables.value(name);
    QVariant::Type oldVarType = oldVar.type();

    if (oldVarType != QVariant::Invalid && value.canConvert(oldVar.type())) {
        QVariant newVar(oldVar.type());
        newVar.setValue(value);
        newVar.convert(oldVar.type());
        m_variables.insert(name, newVar);
    } else
        m_variables.insert(name, value);

    emit variablesChanged();
    emit changed();
}


QVariant ReportInterface::variableValue(const QString &name)
{
    return m_variables.value(name);
}


bool ReportInterface::variableExists(const QString &name)
{
    return m_variables.contains(name) || m_variableObjects.contains(name);
}


void ReportInterface::renameVariable(const QString &oldName, const QString &newName)
{
    if (m_variables.contains(oldName)) {
        QVariant value = m_variables.value(oldName);
        m_variables.remove(oldName);
        m_variables.insert(newName, value);
    } else if (m_variableObjects.contains(oldName)) {
        ObjectPointer value = m_variableObjects.value(oldName);
        m_variableObjects.remove(oldName);
        m_variableObjects.insert(newName, value);
    }

    emit variablesChanged();
    emit changed();
}


void ReportInterface::removeVariable(const QString &name)
{
    if (m_variables.contains(name))
        m_variables.remove(name);
    else if (m_variableObjects.contains(name))
        m_variableObjects.remove(name);

    emit variablesChanged();
    emit changed();
}


QStringList ReportInterface::variableObjectsNames()
{
    return m_variableObjects.keys();
}


void ReportInterface::setVariableObject(const QString &name, QObject *object)
{
    if (m_variableObjects.contains(name) && m_variableObjects.value(name).data() == object)
        return;

    m_variableObjects.insert(name, ObjectPointer(object));

    emit variablesChanged();
    emit changed();
}


QObject * ReportInterface::variableObject(const QString &name)
{
    return m_variableObjects.value(name).data();
}


void ReportInterface::updateVariables()
{
    CuteReport::ReportCore * core = reinterpret_cast<CuteReport::ReportCore*>(parent());

    QVariantHash oldVariables = m_variables;
    m_variables.clear();

    QSet<QString> set = core->getReportParameters(this);
    set.unite(m_moduleVariables);

    foreach (const QString &key, set)
        m_variables.insert(key, oldVariables.value(key));

    emit variablesChanged();
}


void ReportInterface::childDestroyed(QObject * object)
{
    if (qobject_cast<CuteReport::DatasetInterface*>(object))
        emit datasetDeleted((CuteReport::DatasetInterface*) object);
    else if (qobject_cast<CuteReport::FormInterface*>(object))
        emit formDeleted((CuteReport::FormInterface*) object);
    else if (qobject_cast<CuteReport::PageInterface*>(object))
        emit pageDeleted((CuteReport::PageInterface*) object);
}


void ReportInterface::slotItemAdded(CuteReport::BaseItemInterface*item)
{
    if (m_flags.testFlag(DirtynessAutoUpdate))
        setDirty();
    emit itemAdded(item);
}


void ReportInterface::slotItemRemoved(CuteReport::BaseItemInterface* item, QString, bool directDeletion)
{
    if (m_flags.testFlag(DirtynessAutoUpdate))
        setDirty();
    emit itemDeleted(item, directDeletion);
}


void ReportInterface::setFlag(ReportFlag flag, bool enable)
{
    ReportFlags prev = m_flags;
    if (enable)
        m_flags |= flag;
    else
        m_flags &= ~flag;
    precessFlags(prev);
}


void ReportInterface::setFlags(ReportFlags flags)
{
    ReportFlags prev = m_flags;
    m_flags = flags;
    precessFlags(prev);
}


bool ReportInterface::isFlagSet(ReportFlag flag)
{
    return m_flags.testFlag(flag);
}


ReportInterface::ReportFlags ReportInterface::flags()
{
    return m_flags;
}


void ReportInterface::precessFlags(ReportFlags previousFlags)
{
    if (m_flags.testFlag(VariablesAutoUpdate) && !previousFlags.testFlag(VariablesAutoUpdate)) {

        foreach (PageInterface * page, pages()) {
            foreach (BaseItemInterface * item, page->items())
                connect(item, SIGNAL(renderingStringsChanged()), this, SLOT(slotScriptStringsChanged()), Qt::UniqueConnection);
        }
        foreach (DatasetInterface * dataset, datasets())
            connect(dataset, SIGNAL(renderingStringsChanged()), this, SLOT(slotScriptStringsChanged()), Qt::UniqueConnection);
        foreach (FormInterface * form, forms())
            connect(form, SIGNAL(renderingStringsChanged()), this, SLOT(slotScriptStringsChanged()), Qt::UniqueConnection);

        connect(this, SIGNAL(scriptChanged(QString)), this, SLOT(slotScriptStringsChanged()), Qt::UniqueConnection);

        updateVariables();
    }

    if (!m_flags.testFlag(VariablesAutoUpdate) && previousFlags.testFlag(VariablesAutoUpdate)) {

        foreach (PageInterface * page, pages()) {
            foreach (BaseItemInterface * item, page->items())
                disconnect(item, SIGNAL(scriptingStringsChanged()), this, SLOT(slotScriptStringsChanged()));
        }
        foreach (DatasetInterface * dataset, datasets())
            disconnect(dataset, SIGNAL(scriptingStringsChanged()), this, SLOT(slotScriptStringsChanged()));
        foreach (FormInterface * form, forms())
            disconnect(form, SIGNAL(scriptingStringsChanged()), this, SLOT(slotScriptStringsChanged()));

        disconnect(this, SIGNAL(scriptChanged(QString)), this, SLOT(slotScriptStringsChanged()));
    }

    if (m_flags.testFlag(DirtynessAutoUpdate) && !previousFlags.testFlag(DirtynessAutoUpdate)) {
        foreach (PageInterface * page, pages()) {
            connect(page, SIGNAL(afterNewItemAdded(CuteReport::BaseItemInterface*)),
                    this, SLOT(slotNewItemAdded(CuteReport::BaseItemInterface*)), Qt::UniqueConnection);
            foreach (BaseItemInterface * item, page->items())
                connect(item, SIGNAL(changed()), this, SLOT(setDirty()), Qt::UniqueConnection);
        }
        foreach (DatasetInterface * dataset, datasets())
            connect(dataset, SIGNAL(changed()), this, SLOT(setDirty()), Qt::UniqueConnection);
        foreach (FormInterface * form, forms())
            connect(form, SIGNAL(changed()), this, SLOT(setDirty()), Qt::UniqueConnection);

        connect(this, SIGNAL(changed()), this, SLOT(setDirty()), Qt::UniqueConnection);
    }

    if (!m_flags.testFlag(DirtynessAutoUpdate) && previousFlags.testFlag(DirtynessAutoUpdate)) {
        foreach (PageInterface * page, pages()) {
            disconnect(page, SIGNAL(afterNewItemAdded(CuteReport::BaseItemInterface*)),
                       this, SLOT(slotNewItemAdded(CuteReport::BaseItemInterface*)));
            foreach (BaseItemInterface * item, page->items())
                disconnect(item, SIGNAL(changed()), this, SLOT(setDirty()));
        }
        foreach (DatasetInterface * dataset, datasets())
            disconnect(dataset, SIGNAL(changed()), this, SLOT(setDirty()));
        foreach (FormInterface * form, forms())
            disconnect(form, SIGNAL(changed()), this, SLOT(setDirty()));

        disconnect(this, SIGNAL(changed()), this, SLOT(setDirty()));
    }
}


void ReportInterface::setUniqueName(QObject *object, const QString & proposedName)
{
    if (!proposedName.isEmpty() && ReportCore::isNameUnique(object, object->objectName(), this))
        return;

    if (!proposedName.isEmpty()) {
        object->setObjectName(ReportCore::uniqueName(object, proposedName, this));
    } else if (!object->objectName().isEmpty()) {
        object->setObjectName(ReportCore::uniqueName(object, object->objectName(), this));
    } else {
        ReportPluginInterface * plugin = dynamic_cast<ReportPluginInterface *>(object);
        QString proposedName;
        if (plugin) {
            if (!plugin->objectNameHint().isEmpty())
                proposedName = plugin->objectNameHint().isEmpty() ? plugin->moduleShortName().toLower() : plugin->objectNameHint();
        } else {
            proposedName = QString(object->metaObject()->className()).toLower();
        }
        object->setObjectName(ReportCore::uniqueName(object, proposedName, this));
    }
}


QVariant ReportInterface::customData(const QString & dataName) const
{
    return m_customData.value(dataName);
}


void ReportInterface::setCustomData(const QString &dataName, const QVariant &customData)
{
    m_customData.insert(dataName, customData);

    emit customDataChanged();
    emit customDataByNameChanged(dataName);
    emit changed();
}


void ReportInterface::setCustomData(const QVariantHash &customData)
{
    bool isChanged = false;
    QHashIterator<QString,QVariant> it(customData);
    while (it.hasNext()) {
        it.next();
        if (!m_customData.contains(it.key()) || m_customData.value(it.key()) != it.value()) {
            m_customData.insert(it.key(), it.value());
            isChanged = true;
            emit customDataByNameChanged(it.key());
        }
    }

    if (isChanged) {
        emit customDataChanged();
        emit changed();
    }
}

void ReportInterface::clearCustomData(const QString &dataName)
{
    m_customData.remove(dataName);

    emit customDataChanged();
    emit customDataByNameChanged(QString());
    emit changed();
}


QVariantHash ReportInterface::customDataAll() const
{
    return m_customData;
}


void ReportInterface::setCustomDataAll(const QVariantHash &customData)
{
    m_customData = customData;

    emit customDataChanged();
    emit customDataByNameChanged(QString());
    emit changed();
}


void ReportInterface::slotScriptStringsChanged()
{
    updateVariables();
    
    emit variablesChanged();
    emit changed();
}


bool ReportInterface::isDirty() const
{
    return m_isDirty;
}


void ReportInterface::setDirty(bool b)
{
    if (m_isDirty == b)
        return;

    m_isDirty = b;

    emit dirtynessChanged(m_isDirty);
}


bool ReportInterface::isValid() const
{
    return m_isValid;
}


void ReportInterface::rendererInit(CuteReport::ScriptEngineInterface *scriptEngine)
{
    if (m_isBeingRendered)
        return;
    _m_variables = m_variables;
    _m_variableObjects = m_variableObjects;
    _m_flags = m_flags;
    setFlag(DirtynessAutoUpdate,false);
    m_isBeingRendered = true;
}


void ReportInterface::rendererReset()
{
    if (!m_isBeingRendered)
        return;
    m_variables = _m_variables;
    m_variableObjects = _m_variableObjects;
    setFlags(_m_flags); // restore dirtiness checking
    m_isBeingRendered = false;
}


bool ReportInterface::isBeingRendered()
{
    return m_isBeingRendered;
}


void ReportInterface::setValid(bool b)
{
    if (m_isValid == b)
        return;

    m_isValid = b;

    emit validityChanged(m_isValid);
}


void ReportInterface::setInvalid()
{
    if (!m_isValid)
        return;

    m_isValid = false;

    emit validityChanged(m_isValid);
}


void ReportInterface::slotNewItemAdded(CuteReport::BaseItemInterface *item)
{
    if (m_flags.testFlag(DirtynessAutoUpdate)) {
        connect(item, SIGNAL(changed()), this, SLOT(setDirty()), Qt::UniqueConnection);
    }
}


RenderedReportInterface *ReportInterface::renderedReport() const
{
    return m_renderedReport;
}


void ReportInterface::setRenderedReport(RenderedReportInterface *renderedReport)
{
    delete m_renderedReport;
    m_renderedReport = renderedReport;
}


void ReportInterface::clearRenderedReport()
{
    delete m_renderedReport;
    m_renderedReport = 0;
}


void ReportInterface::setModuleVariable(const QString & var, const QString &fullModuleName)
{
//    TODO: implement fullModuleName for case when some modules use the same variable
//    if one module removes it, it still should be present for another module use
    Q_UNUSED(fullModuleName)

    if (m_moduleVariables.contains(var))
        return;

    m_moduleVariables << var;
    setCustomData("moduleVariables", QVariant(m_moduleVariables.toList()));

    updateVariables();
}


void ReportInterface::removeModuleVariable(const QString &var, const QString &fullModuleName)
{
    //    TODO: implement fullModuleName for case when some modules use the same variable
    //    if one module removes it, it still should be present for another module use
    Q_UNUSED(fullModuleName)

    if (!m_moduleVariables.contains(var))
        return;
    m_moduleVariables.remove(var);
    setCustomData("moduleVariables", QVariant(m_moduleVariables.toList()));

    updateVariables();
}


void ReportInterface::setModuleVariables(const QSet<QString> &vars)
{
    QSet<QString> oldSet = m_moduleVariables;
    m_moduleVariables.unite(vars);
    if (m_moduleVariables != oldSet) {
        setCustomData("moduleVariables", QVariant(m_moduleVariables.toList()));
        updateVariables();
    }
}


void ReportInterface::removeModuleVariables(const QSet<QString> &vars)
{
    QSet<QString> oldSet = m_moduleVariables;
    m_moduleVariables.subtract(vars);
    if (m_moduleVariables != oldSet) {
        setCustomData("moduleVariables", QVariant(m_moduleVariables.toList()));
        updateVariables();
    }
}


void ReportInterface::setLocaleName(const QString &localeName)
{
    m_localeName = localeName;
}


QString ReportInterface::localeName() const
{
    return m_localeName;
}


QVariantHash ReportInterface::inheritanceDataHash() const
{
    return m_inheritanceData->toHash(false);
}


void ReportInterface::setInheritanceDataHash(const QVariantHash &data)
{
    m_inheritanceData->setIsDirty(false);
    m_inheritanceData->fromHash(data);
    if (m_inheritanceData->isDirty()) {
        emit inheritanceDataChanged();
        emit changed();
    }
}


const InheritanceDataContainer &ReportInterface::inheritanceData() const
{
    return *m_inheritanceData;
}


void ReportInterface::setInheritanceData(const InheritanceDataContainer &data)
{
    m_inheritanceData->setIsDirty(false);
    m_inheritanceData->copyFrom(data);
    if (m_inheritanceData->isDirty()) {
        emit inheritanceDataChanged();
        emit changed();
    }
}
