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
#include "pageeditor.h"
#include "pageeditorcontainer.h"
#include "propertyeditor.h"
#include "pageinterface.h"
#include "reportinterface.h"
#include "reportplugininterface.h"
#include "iteminterface.h"
#include "bandinterface.h"
#include "objectinspector.h"
#include "reportcore.h"
#include "renamedialog.h"
#include "mainwindow.h"
#include "fonteditor.h"
#include "alignmenteditor.h"
#include "frameeditor.h"
#include "baseiteminterface.h"
#include "emptydialog.h"
#include "dummyitem.h"

#include <QLabel>
#include <QMenu>
#include <QMessageBox>

const char * MODULENAME = "PageEditor";

using namespace CuteDesigner;

inline void initMyResource() { Q_INIT_RESOURCE(page_editor); }


bool pageOrderLessThan(CuteReport::PageInterface * p1, CuteReport::PageInterface * p2)
{
    return p1->order() < p2->order();
}

bool bandOrderLessThan(const CuteReport::BandInterface* b1, const CuteReport::BandInterface* b2)
{
    QPair<QString,int> sort1 = b1->sortingOrder();
    QPair<QString,int> sort2 = b2->sortingOrder();
    int compare = sort1.first.compare(sort2.first, Qt::CaseSensitive);
    if (compare == 0) {
        return sort1.second < sort2.second;
    }
    return compare > 0;
}

bool itemOrderLessThan(const CuteReport::BaseItemInterface* b1, const CuteReport::BaseItemInterface* b2)
{
    return b1->moduleShortName().compare(b2->moduleShortName()) < 0;
}


PageEditor::PageEditor(QObject *parent) :
    ModuleInterface(parent),
    m_currentPage(0),
    m_activeObject(0),
    m_currentManipulator(0),
    m_isActive(false),
    m_blockPageOrders(false),
    m_copyPaste(0),
    m_deleteAction(0),
    m_objectsSaveAction(0),
    m_objectsLoadAction(0)
{
}


PageEditor::~PageEditor()
{
    delete ui;
    delete m_copyPaste;
}


void PageEditor::init(Core *core)
{
    initMyResource();
    ModuleInterface::init(core);

    if (core->getSettingValue("CuteReport_PageEditor/tabMode").isNull())
        core->setSettingValue("CuteReport_PageEditor/tabMode", 2);

    ui = new PageEditorContainer(this);
    ui->init();
    ui->addPagePlugins(core->reportCore()->modules(CuteReport::PageModule));

    m_propertyEditor = core->createPropertyEditor(ui);
    ui->addPropertyEditor(m_propertyEditor);
    m_objectInspector = new ObjectInspector(ui);
    ui->addObjectInspector(m_objectInspector);

    m_stdActions = Core::StdActions(Core::ActionCopy | Core::ActionPaste);

    QList<CuteReport::BandInterface*> bandPlugins;
    QList<CuteReport::BaseItemInterface*> itemPlugins;

    foreach (CuteReport::ReportPluginInterface* plugin, core->reportCore()->modules(CuteReport::ItemModule)) {
        CuteReport::BaseItemInterface* itemPlugin = reinterpret_cast<CuteReport::BaseItemInterface*>(plugin);
        if (qobject_cast<CuteReport::BandInterface*>(itemPlugin))
            bandPlugins << reinterpret_cast<CuteReport::BandInterface*>(itemPlugin);
        else
            itemPlugins << itemPlugin;
    }
    qSort(bandPlugins.begin(), bandPlugins.end(), bandOrderLessThan);
    qSort(itemPlugins.begin(), itemPlugins.end(), itemOrderLessThan);

    foreach (CuteReport::BandInterface* plugin, bandPlugins) {
        ui->addItem(plugin->itemIcon(), plugin->moduleShortName(), plugin->suitName(),  plugin->itemGroup());
    }

    foreach (CuteReport::BaseItemInterface* plugin, itemPlugins) {
        ui->addItem(plugin->itemIcon(), plugin->moduleShortName(), plugin->suitName(),  plugin->itemGroup());
    }


//    foreach (CuteReport::ReportPluginInterface* plugin, core->reportCore()->modules(CuteReport::ItemModule)) {
//        CuteReport::BaseItemInterface* itemPlugin = reinterpret_cast<CuteReport::BaseItemInterface*>(plugin);
//        if (!dynamic_cast<CuteReport::BandInterface*>(itemPlugin)) {
//            bandPlugins << itemPlugin;
////            ui->addItem(itemPlugin->itemIcon(), itemPlugin->moduleShortName(), itemPlugin->suitName(),  itemPlugin->itemGroup());
//        }
//    }

    //connect(core, SIGNAL(loadReport_after(CuteReport::ReportInterface*)), this, SLOT(slotReportCreated(CuteReport::ReportInterface*)));
    //connect(core, SIGNAL(newReport_after(CuteReport::ReportInterface*)), this, SLOT(slotReportCreated(CuteReport::ReportInterface*)));
    connect(core, SIGNAL(currentReportChanged(CuteReport::ReportInterface*)), this, SLOT(slotReportChanged(CuteReport::ReportInterface*)));
    connect(m_objectInspector, SIGNAL(objectChanged(QObject*)), this, SLOT(slotActiveObjectChanged(QObject*)));
    connect(m_objectInspector, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
    connect(m_objectInspector, SIGNAL(deletePressed()), this, SLOT(slotObjectInspectorDeletePressed()));

    connect(ui.data(), SIGNAL(requestForCreatePage(QString)), this, SLOT(slotRequestForCreatePage(QString)));
    connect(ui.data(), SIGNAL(requestForClonePage(QString)), this, SLOT(slotRequestForClonePage(QString)));
    connect(ui.data(), SIGNAL(requestForDeletePage(QString)), this, SLOT(slotRequestForDeletePage(QString)));
    connect(ui.data(), SIGNAL(currentTabChanged(QString)), this, SLOT(slotCurrentPageChangedByGUI(QString)));
    connect(ui.data(), SIGNAL(requestForRenamePage(QString)), this, SLOT(slotRequestForRenamePage(QString)));

    ui->setEnabled(core->currentReport());
}


void PageEditor::reloadSettings()
{
    if (ui)
        ui->reloadSettings();
}


void PageEditor::saveSettings()
{
    if (ui)
        ui->saveSettings();
}


void PageEditor::activate()
{
    core()->reportCore()->log(CuteReport::LogDebug, MODULENAME, "activate");
    if (m_currentPage && m_currentManipulator)
        foreach (QLabel * label, m_currentManipulator->statusBarLabels()) {
            m_core->addToStatusBar(label);
            label->show();
        }

    m_isActive = true;
}


void PageEditor::deactivate()
{
    core()->reportCore()->log(CuteReport::LogDebug, MODULENAME, "deactivate");
    if (m_currentPage && m_currentManipulator) {
        foreach (QLabel * label, m_currentManipulator->statusBarLabels())
            label->hide();
    }
    m_isActive = false;
}


void PageEditor::beforeDelete()
{
    m_propertyEditor->setObject(0);
    m_objectInspector->setRootObject(0);
}


void PageEditor::sync()
{
    // no need to do something
    // changes commit immediately
}


QWidget * PageEditor::view()
{
    //    if (!ui)
    //        ui = new PageEditorContainer;
    return ui;
}


QIcon PageEditor::icon()
{
    return QIcon(":images/editor_48.png");
}


QString PageEditor::name()
{
    return QString("Pages");
}


QList<DesignerMenu *> PageEditor::mainMenu()
{
    QWidget * parent = core()->mainWindow();

    QList<CuteDesigner::DesignerMenu*> menus;

    CuteDesigner::DesignerMenu * reportMenu = new CuteDesigner::DesignerMenu(parent, "Page", 400, 400); // very high priority;  very high priority;
    menus.append(reportMenu);

    reportMenu->menu->addAction(createAction("peActionNewPage", "New Page", ":/images/document-new.png", "Alt+P, Alt+N", SLOT(slotNewPage())));
    reportMenu->menu->addAction(m_deleteAction = createAction("peActionDeletePage", "Delete Page", ":/images/document-close.png", "Alt+P, Alt+D", SLOT(slotDeletePage())));
    reportMenu->menu->addAction(m_objectsLoadAction = createAction("peActionLoadPageItems", "Load Objects", ":/images/document-open.png", "Alt+P, Alt+L", SLOT(slotObjectsLoad())));
    reportMenu->menu->addAction(m_objectsSaveAction = createAction("peActionSavePageItems", "Save Objects as...", ":/images/document-save-as.png", "Alt+P, Alt+S", SLOT(slotObjectsSave())));

    return menus;
}


Core::StdActions PageEditor::stdActions()
{
    return Core::StdActions(Core::ActionCopy | Core::ActionPaste);
}


void PageEditor::stdActionTriggered(Core::StdAction actionType, QAction *action)
{
    Q_UNUSED(action);
//    qDebug() << actionType;

    switch (actionType) {
        case Core::ActionCopy: {
                delete m_copyPaste;
                m_copyPaste = new CopyPasteStruct;
                CuteReport::BaseItemInterface * item = dynamic_cast<CuteReport::BaseItemInterface *> (m_activeObject.data());
                m_copyPaste->page = m_currentPage ? m_currentPage->objectName() : QString();
                m_copyPaste->item = item ? item->objectName() : QString();
                m_copyPaste->cut = false;
                if (item)
                    m_copyPaste->data = core()->reportCore()->serialize(item);
                else {      // only for items implemented
                    delete m_copyPaste;
                    m_copyPaste = 0;
                }
            }
            break;
        case Core::ActionPaste: {
                if (!m_copyPaste || !m_currentReport)
                    return;
                QObject * object = core()->reportCore()->deserialize(m_copyPaste->data);
                CuteReport::BaseItemInterface * item = dynamic_cast<CuteReport::BaseItemInterface *> (object);
                if (item) {
                    CuteReport::BaseItemInterface * currentItem = dynamic_cast<CuteReport::BaseItemInterface *> (m_activeObject.data());
                    if (currentItem && currentItem->objectName() == item->objectName())
                        currentItem = currentItem->parentItem();
                    CuteReport::PageInterface * currentPage = currentItem ? currentItem->page() : dynamic_cast<CuteReport::PageInterface *> (m_activeObject.data());

                    if (currentItem) {
                        item->setParentItem(currentItem);
                        item->setObjectName( core()->reportCore()->uniqueName(object, object->objectName(), m_currentReport));
                        QList<CuteReport::BaseItemInterface *> children = item->findChildren<CuteReport::BaseItemInterface *>();
                        foreach (CuteReport::BaseItemInterface * child, children) {
                            child->setObjectName( core()->reportCore()->uniqueName(child, child->objectName(), m_currentReport));
                        }
                        item->init();
                        QRectF geom = item->absoluteGeometry(CuteReport::Millimeter);
                        geom.moveTopLeft(QPointF(qrand()% 10, qrand()% 10));
                        item->setAbsoluteGeometry(geom);
                        currentPage->addItem(item);
                    } else {
                        item->setObjectName( core()->reportCore()->uniqueName(object, object->objectName(), m_currentReport));
                        QList<CuteReport::BaseItemInterface *> children = item->findChildren<CuteReport::BaseItemInterface *>();
                        foreach (CuteReport::BaseItemInterface * child, children) {
                            qDebug() << child->objectName();
                            child->setObjectName( core()->reportCore()->uniqueName(child, child->objectName(), m_currentReport));
                            child->setBaseItemFlags(CuteReport::BaseItemInterface::BaseReportItem, false );
                            child->setBaseItemFlags(CuteReport::BaseItemInterface::LockedItem, false );
                        }
                        item->setBaseItemFlags(CuteReport::BaseItemInterface::BaseReportItem, false );
                        item->setBaseItemFlags(CuteReport::BaseItemInterface::LockedItem, false );
                        item->init();
                        QRectF geom = item->absoluteGeometry(CuteReport::Millimeter);
                        geom.moveTopLeft(QPointF(qrand()% 10, qrand()% 10));
                        item->setAbsoluteGeometry(geom);
                        currentPage->addItem(item);
                    }


                } else {
                    delete object;
                    return;
                }




            }
        default: break;
    }

}


void PageEditor::slotItemHelperRequest(CuteReport::BaseItemInterface *item)
{
    if (!item || item->isBaseItemFlagSet(CuteReport::BaseItemInterface::LockedItem))
        return;

    CuteReport::BaseItemHelperInterface * helper = item->createHelper(m_core->designerItemInterface());

    if (!helper)
        return;

    EmptyDialog d(m_core->mainWindow());
    d.setWindowTitle(QString("%1 (%2)").arg(item->moduleShortName(), item->suitName()));
    d.setHelperWidget(helper);
    if (d.exec() == QDialog::Accepted)
        helper->sync();
}


void PageEditor::slotPageHelperRequest()
{
    CuteReport::PageInterface * page = dynamic_cast<CuteReport::PageInterface *>(sender());
    if (!page)
        return;

    CuteReport::PageHelperInterface * helper = page->createHelper(m_core->designerItemInterface());

    if (!helper)
        return;

    EmptyDialog d(m_core->mainWindow());
    d.setWindowTitle(QString("%1 (%2)").arg(page->moduleShortName(), page->suitName()));
    d.setHelperWidget(helper);
    if (d.exec() == QDialog::Accepted)
        helper->sync();
}


void PageEditor::slotActiveObjectChanged(QObject * object)
{
    if (object && object == m_activeObject)
        return;

    if (m_activeObject && dynamic_cast<CuteReport::BaseItemInterface *> (m_activeObject.data())) {
        disconnect(m_activeObject, SIGNAL(parentItemChanged(CuteReport::BaseItemInterface*)), this,  SLOT(slotUpdateObjectInspector()));
    } /*else if (m_activeObject && dynamic_cast<CuteReport::PageInterface *> (m_activeObject.data())) {
        disconnect(m_activeObject, SIGNAL(afterItemRemoved(CuteReport::BaseItemInterface*,QString,bool)), this, SLOT(slotUpdateObjectInspector()));
    }*/

    m_activeObject = object;

//    if (object)
//        qDebug() << object->objectName();

    m_propertyEditor->setObject(object);

    if (!qobject_cast<ObjectInspector *> (sender())) {
        m_objectInspector->blockSignals(true);
        m_objectInspector->setRootObject(object ? m_currentPage : 0);
        if (m_currentPage) {
            foreach(CuteReport::BaseItemInterface * item, m_currentPage->selectedItems())
                m_objectInspector->selectObject(item, QItemSelectionModel::Select);
        }
        m_objectInspector->blockSignals(false);
    }

    CuteReport::BaseItemInterface * item = qobject_cast<CuteReport::BaseItemInterface *> (object);
    if (m_currentPage && item) {
        m_currentPage->setCurrentItem(item);
        m_propertyEditor->setReadOnlyMode(item->isBaseItemFlagSet(CuteReport::BaseItemInterface::LockedItem));
        connect(item, SIGNAL(parentItemChanged(CuteReport::BaseItemInterface*)), this, SLOT(slotUpdateObjectInspector()));
        if (m_objectsSaveAction) m_objectsSaveAction->setEnabled(true);
    } else {
        if (m_currentPage) m_propertyEditor->setReadOnlyMode(false);
        if (m_objectsSaveAction) m_objectsSaveAction->setEnabled(false);
    }

    /*else {
        CuteReport::PageInterface * page = qobject_cast<CuteReport::PageInterface *> (object);
        if (page) {
            connect(page, SIGNAL(afterItemRemoved(CuteReport::BaseItemInterface*,QString,bool)), this, SLOT(slotUpdateObjectInspector()));
        }
    }*/

    updateStdEditors();
}


void PageEditor::slotUpdateObjectInspector()
{
    m_objectInspector->setRootObject(m_currentPage);
    m_objectInspector->selectObject(m_activeObject);
}


void PageEditor::slotSelectionChanged()
{
    QList<CuteReport::BaseItemInterface *> list;
    foreach (QObject * object, m_objectInspector->selectedObjects()) {
        CuteReport::BaseItemInterface * item = qobject_cast<CuteReport::BaseItemInterface *>(object);
        if (item)
            list.append(item);
    }
    if (m_currentPage)
        m_currentPage->setSelectedItems(list);
}


void PageEditor::slotReportChanged(CuteReport::ReportInterface * report)
{
    ui->setEnabled(report);
    ui->removeAllTabs();
    foreach (CuteReport::PageInterface * page, m_pages) {
        if (page)
            page->disconnect(this, 0);
    }
    m_pages.clear();
    m_pageNames.clear();
    if (m_currentReport) {
        m_currentReport->disconnect(this, 0);
    }

    m_currentPage = 0;
    m_activeObject = 0;
    m_currentManipulator = 0;

    if (report) {
        QList<CuteReport::PageInterface *> pages = report->pages();
        qSort(pages.begin(), pages.end(), pageOrderLessThan);
        foreach (CuteReport::PageInterface * page, pages) {
            _processNewPage(page);
        }
        slotChangeCurrentPage( pages.count() ? pages.first() : 0);
        if (m_currentPage)
            ui->setCurrentTab(m_currentPage->objectName());
        connect(report, SIGNAL(pageAdded(CuteReport::PageInterface*)), this, SLOT(slotPageCreatedOutside(CuteReport::PageInterface*)));
        connect(report, SIGNAL(pageDeleted(CuteReport::PageInterface*)), this, SLOT(slotPageDeletedOutside(CuteReport::PageInterface*)));
    } else {
        m_objectInspector->setRootObject(0);
        m_propertyEditor->setObject(0);
    }

    if (m_deleteAction) m_deleteAction->setEnabled(m_pages.count());
    if (m_objectsLoadAction) m_objectsLoadAction->setEnabled(m_pages.count());

    m_currentReport = report;
}


void PageEditor::slotRequestForCreatePage(QString moduleName)
{
    CuteReport::PageInterface * page = core()->reportCore()->createPageObject(moduleName, core()->currentReport());
    if (!page)
        return;
    page->init();
    core()->currentReport()->addPage(page);

    if (m_deleteAction) m_deleteAction->setEnabled(m_pages.count());
    if (m_objectsLoadAction) m_objectsLoadAction->setEnabled(m_pages.count());
}


void PageEditor::slotPageCreatedOutside(CuteReport::PageInterface *page)
{
    if (!page || m_pages.contains(page))
        return;
    ui->setCurrentTab(page->objectName());
    _processNewPage(page);
    slotChangeCurrentPage(page);

    if (m_deleteAction) m_deleteAction->setEnabled(m_pages.count());
    if (m_objectsLoadAction) m_objectsLoadAction->setEnabled(m_pages.count());
}


void PageEditor::slotRequestForDeletePage(QString pageName)
{
    CuteReport::PageInterface * page = core()->reportCore()->pageByName(pageName, core()->currentReport());
    if (!page)
        return;

    if (page->items().count()) {
        int ret = QMessageBox::warning(core()->reportCore()->rootWidget(), tr("CuteReport"),
                                       tr("The Page you want to delete is not empty.\n"
                                          "Do you still want to delete it?"),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);
        if (ret == QMessageBox::Cancel)
            return;
    }


    QList<CuteReport::PageInterface*> pages = core()->currentReport()->pages();
    int index = -1;
    for (int i=0; i<pages.count(); i++)
        if (pages[i]->objectName() == pageName) {
            index = i;
            break;
        }

    ui->removeTab(pageName);
    //    int index = m_pages.indexOf(page);
    m_pages.removeAt(index);
    m_pageNames.removeAt(index);
    m_currentReport->deletePage(page);

    if (m_deleteAction) m_deleteAction->setEnabled(m_pages.count());
    if (m_objectsLoadAction) m_objectsLoadAction->setEnabled(m_pages.count());
}


void PageEditor::slotPageDeletedOutside(CuteReport::PageInterface *page)
{
    int index = m_pages.indexOf(page);
    m_pages.removeAt(index);
    m_pageNames.removeAt(index);
    ui->removeTab(page->objectName());
    if (m_currentPage == page) {
        m_currentPage = 0;
        int index = -1;
        if (index > m_pages.count() -1)
            index = m_pages.count() -1;
        if (index < 0)
            index = 0;
        slotChangeCurrentPage( m_pages.count() ? m_pages[index] : 0);
        if (m_currentPage)
            ui->setCurrentTab(m_currentPage->objectName());
    }

    if (m_deleteAction) m_deleteAction->setEnabled(m_pages.count());
    if (m_objectsLoadAction) m_objectsLoadAction->setEnabled(m_pages.count());
}


void PageEditor::slotRequestForClonePage(QString pageName)
{
    Q_UNUSED(pageName)
}


void PageEditor::slotCurrentPageChangedByGUI(QString pageName)
{
    CuteReport::PageInterface * newPage = core()->reportCore()->pageByName(pageName, core()->currentReport());
    if (newPage != m_currentPage) {
        slotChangeCurrentPage(newPage);
        core()->setCurrentPage(newPage);
    }
}


void PageEditor::slotCurrentPageChangedByCore(CuteReport::PageInterface* page)
{
    if (page == m_currentPage)
        return;

    slotChangeCurrentPage(page);
    ui->setCurrentTab(page->objectName());
}


void PageEditor::slotRequestForRenamePage(QString pageName)
{
    CuteReport::PageInterface * page = core()->reportCore()->pageByName(pageName, core()->currentReport());

    if (page) {

        if (page->isPageFlagSet(CuteReport::PageInterface::BaseReportPage)) {
            QMessageBox::warning(core()->mainWindow(), tr("CuteReport page renaming"),
                                           tr("This page is inherited from another report page\n"
                                              "and cannot be renamed."),
                                           QMessageBox::Ok);
        } else {
            RenameDialog d(page, core()->currentReport(), core()->mainWindow());
            d.setWindowTitle("Page renaming");
            if (d.exec() == QDialog::Accepted) {
                page->setObjectName(d.newName());
                //            ui->setNewPageName( pageName, d.newName());
            }
        }
    }
}


void PageEditor::slotPageNameChangedOutside(const QString & name)
{
    Q_UNUSED(name);
    CuteReport::PageInterface * page = qobject_cast<CuteReport::PageInterface*>(sender());
    Q_ASSERT(page);
    int index = m_pages.indexOf(page);
    QString newName = page->objectName();
    QString oldName = m_pageNames.at(index);
    ui->setNewPageName(oldName, newName);
    m_pageNames[index] = newName;
}


void PageEditor::slotNewPage()
{
    QStringList modules = core()->reportCore()->moduleNames(CuteReport::PageModule);

    if (modules.size() == 0) {
        CuteReport::ReportCore::log(CuteReport::LogWarning, MODULENAME, "There is over no page modules");
        return;
    }

    if (modules.size() > 1)
        CuteReport::ReportCore::log(CuteReport::LogWarning, MODULENAME, "There are over 1 page modules.");

    slotRequestForCreatePage(modules.at(0));
}


void PageEditor::slotDeletePage()
{
    if (!m_currentPage)
        return;

    slotRequestForDeletePage(m_currentPage->objectName());
}


void PageEditor::slotPageMoveFront()
{
    if (!m_currentReport)
        return;
    QList<CuteReport::PageInterface *> pages = m_currentReport->pages();
    if (pages.size() < 1)
        return;
    qSort(pages.begin(), pages.end(), pageOrderLessThan);
    int curIndex = pages.indexOf(m_currentPage);
    if (curIndex == 0)
        return;
    CuteReport::PageInterface * otherPage = pages.at(curIndex-1);
    m_blockPageOrders = true;
    int otherPageOrder = otherPage->order();
    otherPage->setOrder(m_currentPage->order());
    m_currentPage->setOrder(otherPageOrder);

    pages.swap(otherPageOrder, curIndex);
    ui->removeAllTabs();
    foreach (CuteReport::PageInterface * page, pages)
        ui->addTab(page->createView(), page->icon(), page->objectName());
    ui->setCurrentTab(m_currentPage->objectName());

    m_blockPageOrders = false;
}


void PageEditor::slotPageMoveBack()
{
    if (!m_currentReport)
        return;
    QList<CuteReport::PageInterface *> pages = m_currentReport->pages();
    if (pages.size() < 1)
        return;
    qSort(pages.begin(), pages.end(), pageOrderLessThan);
    int curIndex = pages.indexOf(m_currentPage);
    if (curIndex == pages.size()-1)
        return;
    CuteReport::PageInterface * otherPage = pages.at(pages.size()-1);
    m_blockPageOrders = true;
    int otherPageOrder = otherPage->order();
    otherPage->setOrder(m_currentPage->order());
    m_currentPage->setOrder(otherPageOrder);

    pages.swap(otherPageOrder, curIndex);
    ui->removeAllTabs();
    foreach (CuteReport::PageInterface * page, pages)
        ui->addTab(page->createView(), page->icon(), page->objectName());
    ui->setCurrentTab(m_currentPage->objectName());

    m_blockPageOrders = false;
}


void PageEditor::slotObjectsLoad()
{
    if (!m_currentPage)
        return;

    QString fileName = core()->loadObjectDialog(QStringList() << "qtrpo","Save Selected Objects");

    if (!fileName.isEmpty()) {
        QByteArray objectsData = core()->reportCore()->loadObject(fileName, core()->currentReport());
        bool ok = true;
        QObject * objectGroup = core()->reportCore()->deserialize(objectsData, &ok);

        if (objectGroup) {
            QList<CuteReport::BaseItemInterface *> items = objectGroup->findChildren<CuteReport::BaseItemInterface *>();

            QList<CuteReport::BaseItemInterface *> currentSelectedItems = m_currentPage->selectedItems();
            CuteReport::BaseItemInterface * itemToPlaceOn = currentSelectedItems.size() ? currentSelectedItems.first() : 0;
            if (!itemToPlaceOn) {
                /// place of the first band found
                QList<CuteReport::BaseItemInterface *> pageItems = m_currentPage->items();
                foreach (CuteReport::BaseItemInterface * item, pageItems) {
                    CuteReport::BandInterface * band = dynamic_cast<CuteReport::BandInterface*>(item);
                    if (band) {
                        itemToPlaceOn = band;
                        break;
                    }
                }
            }

            if (itemToPlaceOn) {

                foreach (CuteReport::BaseItemInterface * item, items) {
                    if (item->parent() != objectGroup)
                        continue;
                    CuteReport::PageInterface * page = itemToPlaceOn->page();
                    item->setParentItem(itemToPlaceOn);
                    item->init();
                    page->addItem(item);
                }
            }
        }

        delete objectGroup;
    }
}


void PageEditor::slotObjectsSave()
{
    if (!m_currentPage)
        return;
    QList<CuteReport::BaseItemInterface *> items = m_currentPage->selectedItems();
    QObject * rootObject = new CuteReport::DummyItem();
    rootObject->setObjectName("Selection");

    /// check if selected items have no parent-child relation and remove child from saving list
    QList<CuteReport::BaseItemInterface *> allChildrenItems;
    foreach (CuteReport::BaseItemInterface * item, items) {
        allChildrenItems << item->findChildren<CuteReport::BaseItemInterface *>();
    }
    foreach (CuteReport::BaseItemInterface * item, items) {
        if (!allChildrenItems.contains(item)) {
            CuteReport::BaseItemInterface * clonedItem = item->clone(true, false);
            clonedItem->setParentItem(0);
            clonedItem->setPage(0);
            clonedItem->setParent(rootObject);
        }
    }

    bool ok = true;
    QByteArray objectsData = core()->reportCore()->serialize(rootObject, &ok);
    if (ok) {
         QString fileName = core()->saveObjectDialog(QStringList() << "qtrpo","Save Selected Objects");

         if (!fileName.isEmpty()) {
             core()->reportCore()->saveObject(fileName, core()->currentReport(), objectsData);
         }
    }

    delete rootObject;
}


void PageEditor::slotObjectInspectorDeletePressed()
{
    if (!m_currentPage)
        return;

    /// we delete by name for safety, since Page can delete items that are still in our list
    QStringList list;
    foreach (QObject * object, m_objectInspector->selectedObjects()) {
        CuteReport::BaseItemInterface * item = qobject_cast<CuteReport::BaseItemInterface *>(object);
        if (item)
            list << item->objectName();
    }

    foreach (const QString & itemName, list) {
        m_currentPage->deleteItem(itemName);
    }
}


//void PageEditor::slotReportCreated(CuteReport::ReportInterface * report)
//{
//    connect(report, SIGNAL(pageAdded(CuteReport::PageInterface*)), this
//}


void PageEditor::_processNewPage(CuteReport::PageInterface *page)
{
    if (!page)
        return;
    m_pages.append(page);
    m_pageNames.append(page->objectName());
    ui->addTab(page->createView(ui.data()), page->icon(), page->objectName());
    connect(page, SIGNAL(objectNameChanged(QString)), this, SLOT(slotPageNameChangedOutside(QString)));
    connect(page, SIGNAL(itemHelperRequest(CuteReport::BaseItemInterface*)), this, SLOT(slotItemHelperRequest(CuteReport::BaseItemInterface*)));
    connect(page, SIGNAL(pageHelperRequest()), this, SLOT(slotPageHelperRequest()));
}


void PageEditor::setStdActions(Core::StdActions actions)
{
    m_stdActions = actions;
    stdActionsChanged(m_stdActions);
}


void PageEditor::slotChangeCurrentPage(CuteReport::PageInterface* page)
{
    if (m_currentPage) {
        disconnect(m_currentPage, SIGNAL(activeObjectChanged(QObject*)), this, SLOT(slotActiveObjectChanged(QObject*)));
        if (m_currentManipulator)
            foreach (QLabel * label, m_currentManipulator->statusBarLabels())
                label->hide();
    }

    m_currentPage = page;

    if (!m_currentPage) {
        slotActiveObjectChanged(0);
        if (m_currentManipulator)
            m_currentManipulator->setActivePage(0);
        return;
    }

    connect(m_currentPage, SIGNAL(activeObjectChanged(QObject*)), this, SLOT(slotActiveObjectChanged(QObject*)));
    connect(m_currentPage, SIGNAL(afterItemRemoved(CuteReport::BaseItemInterface*, QString, bool)), this, SLOT(slotUpdateObjectInspector()));

    /// manage manipulator
    CuteReport::PageManipulatorInterface * oldManipulator = m_currentManipulator;
    if (!m_currentManipulator) {
        m_currentManipulator = m_currentPage->createManupulator(this);
        m_pageManipulators.insert(m_currentManipulator->pageManupilatorId(), m_currentManipulator);
    } else
        if (m_currentManipulator && m_currentPage->pageManupilatorID() != m_currentManipulator->pageManupilatorId()) {
            if (m_pageManipulators.contains(m_currentPage->pageManupilatorID())) {
                m_currentManipulator = m_pageManipulators.value(m_currentPage->pageManupilatorID());
            } else {
                m_currentManipulator = m_currentPage->createManupulator(this);
                m_pageManipulators.insert(m_currentManipulator->pageManupilatorId(), m_currentManipulator);
            }
        }

    if (m_currentManipulator) {
        m_currentManipulator->setActivePage(m_currentPage);

        if (m_isActive)
            foreach (QLabel * label, m_currentManipulator->statusBarLabels()) {
                m_core->addToStatusBar(label);
                label->show();
            }

        if (oldManipulator != m_currentManipulator)
            ui->setPageActions(m_currentManipulator->actions());
    }



    slotActiveObjectChanged(m_currentPage->currentItem() ? (QObject*)m_currentPage->currentItem() : (QObject*)m_currentPage);
}


void PageEditor::updateStdEditors()
{
    CuteReport::StdEditorPropertyList list;
    //QObject * object = (m_currentPage && m_currentPage->currentItem()) ? (QObject*)m_currentPage->currentItem() : (QObject*)m_currentPage;
    bool locked = false;
    if (CuteReport::BaseItemInterface * bItem = qobject_cast<CuteReport::BaseItemInterface*>(m_activeObject)) {
        list = bItem->stdEditorList();
        locked = bItem->isBaseItemFlagSet(CuteReport::BaseItemInterface::LockedItem);
    } else if (CuteReport::PageInterface * page = qobject_cast<CuteReport::PageInterface*>(m_activeObject))
        list = page->stdEditorList();

    bool fontEditor = false;
    bool alignmentEditor = false;
    bool frameEditor = false;

    foreach (const CuteReport::StdEditorProperty & ed, list) {
        switch (ed.first) {
            case CuteReport::EDFont: ui->fontEditor()->setFontPropertyName(ed.second); fontEditor = true; break;
            case CuteReport::EDFontColor: ui->fontEditor()->setColorPropertyName(ed.second); fontEditor = true; break;
            case CuteReport::EDTextAlignment: ui->alignmentEditor()->setAlignPropertyName(ed.second); alignmentEditor = true; break;
            case CuteReport::EDFrame: ui->frameEditor()->setFramePropertyName(ed.second); frameEditor = true; break;
            case CuteReport::EDFramePen: ui->frameEditor()->setPenPropertyName(ed.second); frameEditor = true; break;
        }
    }

    QObjectList selection;

    if (m_currentPage && (fontEditor || alignmentEditor || frameEditor) ) {
        foreach (CuteReport::BaseItemInterface * item, m_currentPage->selectedItems()) {
            selection << static_cast<QObject*>(item);
        }
    }

    if (fontEditor) {
        if (!m_currentPage || selection.size() < 2)
            ui->fontEditor()->setObject(m_activeObject);
        else
            ui->fontEditor()->setObjectList(selection);
        ui->fontEditor()->update();
    } else {
        ui->fontEditor()->clear();
    }
    ui->fontEditor()->setEnabled(fontEditor && !locked);

    if (alignmentEditor) {
        if (!m_currentPage || selection.size() < 2)
            ui->alignmentEditor()->setObject(m_activeObject);
        else
            ui->alignmentEditor()->setObjectList(selection);
        ui->alignmentEditor()->update();
    } else {
        ui->alignmentEditor()->clear();
    }
    ui->alignmentEditor()->setEnabled(alignmentEditor && !locked);

    if (frameEditor) {
        if (!m_currentPage || selection.size() < 2)
            ui->frameEditor()->setObject(m_activeObject);
        else
            ui->frameEditor()->setObjectList(selection);
        ui->frameEditor()->update();
    } else {
        ui->frameEditor()->clear();
    }
    ui->frameEditor()->setEnabled(frameEditor && !locked);
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(PageEditor, PageEditor)
#endif
