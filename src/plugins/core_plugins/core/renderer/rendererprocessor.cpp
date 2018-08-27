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

#include "rendererprocessor.h"
#include "rendererdata.h"
#include "renderer.h"
#include "datasetinterface.h"
#include "forminterface.h"
#include "iteminterface.h"
#include "renderediteminterface.h"
#include "bandinterface.h"
#include "scriptextensioninterface.h"
#include "reportcore.h"
#include "renderedreport.h"
#include "limits"
#include "aggregatefunctions.h"
#include "scriptengine.h"
#include "preparser.h"
#include "variables.h"

using namespace CuteReport;

USING_SUIT_NAMESPACE

SUIT_BEGIN_NAMESPACE

static bool cmpBands(BandInterface * a, BandInterface * b)
{
    if (a->layoutPriority() == b->layoutPriority())
        return a->order() < b->order();
    else
        return a->layoutPriority() < b->layoutPriority();
}


bool cmpBandsDesc(BandInterface * a, BandInterface * b)
{
    if (a->layoutPriority() == b->layoutPriority())
        return a->order() < b->order();
    else
        return a->layoutPriority() > b->layoutPriority();
}


bool cmpItems(BaseItemInterface * a, BaseItemInterface * b)
{
    return a->order() < b->order();
}


bool cmpPages(PageInterface * a, PageInterface * b)
{
    return a->order() < b->order();
}


Thread::Thread()
{
}

Thread::~Thread() {
    ReportCore::log(LogDebug, "Renderer::Thread", "Rendering Thread successfuly destroyed");
}


RendererProcessor::RendererProcessor(Renderer *renderer, CuteReport::ReportInterface * report)
    :QObject(renderer),
      m_rendererItemInterface(0),
      m_terminate(false),
      m_currentRenderedPage(0),
      m_runs(false),
      m_processEvents(false),
      m_pages(0),
      m_lastItemId(0),
      m_currentColumn(0),
      m_columnableTop(0),
      m_columnableBottom(0)
{
    m_antialiasing = renderer->antialiasing();
    m_textAntialiasing = renderer->textAntialiasing();
    m_smoothPixmapTransform = renderer->smoothPixmapTransform();
    m_dpi = renderer->dpi() ? renderer->dpi() : 300;
    m_delay = renderer->delay();

    m_data = new RendererData();
    m_data->origReport = report;
    m_data->renderer = renderer;
    m_data->processor = this;
    m_data->dpi = renderer->dpi();

//    QByteArray reportData = m_data->renderer->reportCore()->serialize(m_data->origReport);
//    m_data->workingReportCopy = dynamic_cast<CuteReport::ReportInterface*>(m_data->renderer->reportCore()->deserialize(reportData));
    m_data->workingReportCopy = m_data->origReport;

//    Q_ASSERT(m_data->workingReportCopy);
//    if (m_data->workingReportCopy) {
//        m_data->workingReportCopy->setParent(this);
//        m_data->workingReportCopy->init();
//    }

    m_aggregateFunctions = new SUIT_NAMESPACE::AggregateFunctions();
    m_preparser = new SUIT_NAMESPACE::PreParser();
    m_variables = new SUIT_NAMESPACE::Variables();
}


RendererProcessor::~RendererProcessor()
{
    emit log(CuteReport::LogDebug, "RendererProcessor DTOR", "");
    if (m_currentRenderedPage)
        delete m_currentRenderedPage;
    delete m_aggregateFunctions;
    delete m_preparser;
    delete m_variables;
    delete m_data;
}


void RendererProcessor::start()
{
    emit log(CuteReport::LogDebug, QString("start thread id: %1  processor thread id: %2").arg((long)QThread::currentThread()), "");

    m_runs = false;
    m_processEvents = qApp->thread() == this->thread();

    emit started();

    initScriptEngine();

    bool resultOk = evaluateScript();

    if (resultOk && !m_data->workingReportCopy->script().contains("engine.run")) {
        m_data->scriptEngine->evaluate("engine.run();");
        if (m_data->scriptEngine->hasUncaughtException()) {
            QString _message = QString("error evaluating engine.run %1")
                               .arg(m_data->scriptEngine->uncaughtException().toString());
            qWarning() << _message;
            m_data->appendError(_message);
            resultOk = false;
        }
    }

    if (!resultOk && !m_runs) { // sometimes script contains error after renderer started.
        _done(false);
    } else if (!m_runs) {       // could be cancelled by Form logic
        _done(true);
    }
}


void RendererProcessor::terminate()
{
    QMutexLocker locker(&mutex);
    m_terminate = true;
}


void RendererProcessor::_done(bool success)
{
    emit log(CuteReport::LogDebug, QString("_done"), "");
    m_data->dpi = m_dpi;

    RenderedReport * renReport = new RenderedReport();
    renReport->setRenderedPages(m_data->pages);
    renReport->setDpi(m_data->dpi);

    m_data->pages.clear();
    resetScriptEngine();

    emit done(success, renReport);
}

void RendererProcessor::scriptEngineException(QScriptValue value)
{
    emit log(CuteReport::LogError, "Script Exception", value.toString());
}


void RendererProcessor::processEventsIfNeeded()
{
    if (m_processEvents)
        QCoreApplication::processEvents();
}


void RendererProcessor::run()
{
    m_runs = true;

    emit log(CuteReport::LogDebug, QString("run thread id %1").arg((long)QThread::currentThread()), "");

    if (!m_data->workingReportCopy->pages().size())
        emit log(LogError, "report has no pages",  QString("report with name %1 has no pages").arg(m_data->workingReportCopy->objectName()));

    QList<PageInterface *> pages = m_data->workingReportCopy->pages();
    qSort(pages.begin(), pages.end(), cmpPages);

    m_currentPageNumber = 0;
    m_data->reset();

    for (int i=0; i< pages.count() && !terminated(); ++i) {
        CuteReport::PageInterface * page = pages[i];
        emit log(LogDebug, QString("rendering page: %1").arg(page->objectName()), "");
        renderReportPage(page);
    }

    emit m_rendererItemInterface->reportDone();

    resetData();

    m_pages = m_currentPageNumber;

    if (terminated()) {
        emit log(LogDebug, QString("terminated"), "");
        _done(true);
    } else
        _done(true);
}


void RendererProcessor::initScriptEngine()
{
    Q_ASSERT(m_data);
    m_data->scriptEngine = new ScriptEngine(this);
    ScriptEngine * scriptEngine = m_data->scriptEngine;
    connect(scriptEngine, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(scriptEngineException(QScriptValue)));
    scriptEngine->setProcessEventsInterval(50);
    scriptEngine->pushContext();

    ReportInterface * report = m_data->workingReportCopy;

    m_rendererItemInterface = new RendererItemInterface(this);
    scriptEngine->globalObject().setProperty("engine", scriptEngine->newQObject(m_rendererItemInterface), QScriptValue::Undeletable);

    /// registering external application delegates
    foreach (const QString & name, m_data->workingReportCopy->variableObjectsNames()) {
        QObject * obj = m_data->workingReportCopy->variableObject(name);
        if (obj)
            scriptEngine->globalObject().setProperty(name, scriptEngine->newQObject(obj), QScriptValue::Undeletable);
    }

    /// registering script extensions
    foreach (const QString & moduleName, m_data->renderer->reportCore()->moduleNames(CuteReport::ScriptExtensionModule) ) {
        ScriptExtensionInterface * extension = static_cast<ScriptExtensionInterface*>(m_data->renderer->reportCore()->createScriptExtentionObject(moduleName));
        foreach (const QString & key, extension->keys())
            extension->initialize(key, scriptEngine);
    }

    /// registering additional metatypes
    qScriptRegisterQObjectMetaType<DatasetInterface*>(scriptEngine);

    /// adding datasets
    foreach(CuteReport::DatasetInterface * dtst, report->datasets()) {
        emit log(CuteReport::LogDebug, QString("Preparing dataset \'%1\'").arg(dtst->objectName()), "");
        dtst->renderInit(scriptEngine);
        scriptEngine->globalObject().setProperty(dtst->objectName(), scriptEngine->newQObject(dtst), QScriptValue::ReadOnly);
    }

    /// adding forms
    foreach(CuteReport::FormInterface * form, report->forms()){
        form->renderInit(scriptEngine);
        scriptEngine->globalObject().setProperty(form->objectName(), scriptEngine->newQObject(form), QScriptValue::ReadOnly);
    }

    /// adding items
    QSet<QString> initedItems;
    foreach(CuteReport::PageInterface * page, report->pages()) {
        page->renderInit(scriptEngine);
        foreach(CuteReport::BaseItemInterface * item, page->items()) {
            scriptEngine->globalObject().setProperty(item->objectName(), scriptEngine->newQObject(item), QScriptValue::ReadOnly);
            if (!initedItems.contains(item->objectName())) {
                item->renderInit(scriptEngine, CuteReport::InitScript | CuteReport::InitData);
                initedItems.insert(item->moduleFullName());
            }
        }
    }

    m_preparser->registerScriptObjects(scriptEngine);
    m_aggregateFunctions->registerScriptObjects(scriptEngine);
    m_variables->registerScriptObjects(scriptEngine);
}


void RendererProcessor::resetScriptEngine()
{
    ReportInterface * report = m_data->workingReportCopy;
    foreach(CuteReport::FormInterface * form, report->forms()){
        form->renderReset();
    }
    foreach(CuteReport::DatasetInterface * dtst, report->datasets()) {
        dtst->renderReset();
    }
}


bool RendererProcessor::evaluateScript()
{
    QString script = m_data->origReport->script();
    if (!preprocessScript(script))
        return false;
    m_data->scriptEngine->evaluate( script );

    if (m_data->scriptEngine->hasUncaughtException()) {
        QString _message = QString("script error at line %1 \n%2")
                           .arg(m_data->scriptEngine->uncaughtExceptionLineNumber())
                           .arg(m_data->scriptEngine->uncaughtException().toString());
        m_data->appendError(_message);
        emit log(LogError, QString("Report's script evaluation error"), QString("error message: %1").arg(_message));
        return false;
    }
    return true;
}


void RendererProcessor::resetData()
{
    foreach (DatasetInterface * ds, m_data->workingReportCopy->datasets())
        ds->resetCursor();

    foreach (PageInterface * page, m_data->workingReportCopy->pages()) {
        page->renderReset();
        foreach (BaseItemInterface * item, page->items())
            item->renderReset();
    }

    m_aggregateFunctions->reset();
    m_bandsDone.clear();
    m_datasetRegister.clear();
    m_currentDatasetLine = 0;
    m_data->scriptEngine->popContext();
    m_data->scriptEngine->pushContext();
}


void RendererProcessor::renderReportPage(CuteReport::PageInterface * page)
{
    emit log(CuteReport::LogDebug, QString("renderReportPage: %1").arg(page->objectName()), "");
    emit m_rendererItemInterface->templatePageBefore(page);

    if (m_dpi > 0)
        page->setDpi(m_dpi);

    topBands.clear();
    bottomBands.clear();
    freeBands.clear();

    m_currentPage = page;
    m_currentDatasetLine = 0;
    m_processingBand = 0;
    m_lastProcessedBand = 0;
    /// do not reset page number here - it is controlled by page object itself

//    page->renderInit(m_rendererItemInterface);
    page->renderStart();

    QList <BaseItemInterface*> items = page->items();
    for (int i=0; i<items.count() && !terminated(); ++i) {
        CuteReport::BaseItemInterface * item = items.at(i);
        CuteReport::BandInterface* band = qobject_cast<CuteReport::BandInterface*>(item);
        if (!band)
            continue;

        if (band->layoutType() == CuteReport::BandInterface::LayoutTop)
            topBands.append(band);
        if (band->layoutType() == CuteReport::BandInterface::LayoutBottom)
            bottomBands.append(band);
        if (band->layoutType() == CuteReport::BandInterface::LayoutFree)
            freeBands.append(band);
    }

    if (terminated())
        return;

    qSort(topBands.begin(), topBands.end(), cmpBandsDesc);
    qSort(bottomBands.begin(), bottomBands.end(), cmpBandsDesc);

//    initBands(topBands);
//    initBands(bottomBands);
//    initBands(freeBands);

    if (!m_data->pages.count() || !page->printOnPreviousPage())
        createNewRenderingPage();

    for (int i=0; i<topBands.count() && !terminated(); ++i) {
        BandInterface * band = topBands.at(i);
        emit log(CuteReport::LogDebug, QString("checking band: %1").arg(band->objectName()), "");
        if (terminated())
            return;

        if (m_bandsDone.contains(band)) //already processed in group iterations
            continue;

        DatasetInterface * dataset = datasetRegisteredTo(band);
        if (dataset)
            processDataset(dataset);
        else
            processBand(band, CuteReport::RenderingNormal);
    }

    m_state = RendererPublicInterface::ContentDone;

    if (m_currentRenderedPage) {
        completePage(m_currentRenderedPage);
    }

    page->renderReset();

    emit m_rendererItemInterface->templatePageAfter(page);
}


void RendererProcessor::initBands(QList<CuteReport::BandInterface*> bands)
{
    foreach (CuteReport::BandInterface * band, bands) {
        band->renderBegin();
        foreach (CuteReport::BaseItemInterface * const child, band->findChildren<CuteReport::BaseItemInterface *>())
            child->renderBegin();
    }
}


void RendererProcessor::newColumnOrPage()
{
    if (m_currentPage->columns() <= 1 || !m_processingBand->respectPageColumns() || (m_currentColumn+1) >= m_currentPage->columns())  {
        createNewRenderingPage();
    } else {
        shiftColumn();
    }
}


void RendererProcessor::firstColumn()
{
    m_currentColumn = 0;

    if (m_columnableBottom == 0)  // first band processing
        return;

    m_freeSpace.setTop(m_columnableBottom);
    m_freeSpace.moveLeft(m_currentPage->pageRect().left() + m_freeSpace.width() * qMax(0, m_currentColumn));
}


void RendererProcessor::shiftColumn()
{
    m_currentColumn++;

    if (m_currentColumn >= m_currentPage->columns())
        m_currentColumn = 0;

    m_freeSpace.setTop(m_columnableTop);
    m_freeSpace.moveLeft(m_currentPage->pageRect().left() + m_freeSpace.width() * qMax(0, m_currentColumn));
}


void RendererProcessor::createNewRenderingPage()
{
    if (m_currentRenderedPage) {
        completePage(m_currentRenderedPage);
    }

    if (terminated())
        return;

    m_state = RendererPublicInterface::EmptyPage;

    ++m_currentPageNumber;
    m_currentColumn = 0;

    m_currentRenderedPage = m_currentPage->render();

    emit m_rendererItemInterface->pageBefore(m_currentRenderedPage);


    emit processingPage(m_currentPageNumber, 0, 0, 0);

    m_freeSpace = m_currentPage->pageRect();
    if (m_currentPage->columns() > 1)
        m_freeSpace.setWidth(m_currentPage->pageRect().width() / m_currentPage->columns());
    m_columnableTop = m_freeSpace.top();
    m_columnableBottom = 0;

    emit m_rendererItemInterface->pageDataAfter(m_currentRenderedPage);

    foreach(BandInterface * band, freeBands)   //process listFree first if it want paint on background
        if (band != m_processingBand)
            processBand(band, RenderingNewPage);

    m_state = RendererPublicInterface::BackgroundDone;

    foreach(BandInterface * band, topBands)
        if (band != m_processingBand)
            processBand(band, RenderingNewPage);

    m_state = RendererPublicInterface::HeadersDone;

    for (int i = bottomBands.count()-1; i>=0 ;--i)
        if (bottomBands.at(i) != m_processingBand)
            processBand(bottomBands.at(i), RenderingNewPage);

    m_state = RendererPublicInterface::FootersDone;
}


void RendererProcessor::completePage(RenderedPageInterface* page)
{
    foreach(BandInterface * band, freeBands) {   //process listFree first if it want paint on foreground
        if (band != m_processingBand)
            processBand(band, RenderingNormal);
    }

    m_data->appendPage(page);

    emit m_rendererItemInterface->pageAfter(m_currentRenderedPage);

    m_currentRenderedPage = 0;

    if (m_delay > 0) {
        QWaitCondition waitCondition;
        QMutex mutex;
        waitCondition.wait(&mutex,m_delay);
    }
}


void RendererProcessor::processBand(CuteReport::BandInterface * band, RenderingStage stage)
{
    Q_ASSERT(band);

    emit m_rendererItemInterface->bandBefore(band);

    CuteReport::BandInterface * saveBand = m_processingBand;
    m_processingBand = band;

    bool active = false;
    switch (stage) {
        case RenderingNewPage:  active = band->renderNewPage(); break;
        case RenderingNormal:   active = band->renderBegin(); break;
    }

    emit m_rendererItemInterface->bandDataAfter(band);

    if (active) {

        emit log(CuteReport::LogDebug, QString("process band: %1").arg(band->objectName()), "");

        if (!band->respectPageColumns())
            firstColumn();

        emit m_rendererItemInterface->bandBefore(band);

        QRectF geometry = band->absoluteGeometry(Millimeter);
        if (band->layoutType()== BandInterface::LayoutTop)
            geometry.moveTo(m_freeSpace.topLeft());
        else if (band->layoutType()== BandInterface::LayoutBottom)
            geometry.moveBottomLeft(m_freeSpace.bottomLeft());
        band->setAbsoluteGeometry(geometry, Millimeter);

        QList<BaseItemInterface*> children;
        foreach (BaseItemInterface* child, band->findChildren<CuteReport::BaseItemInterface*>())
            if (child->parent() == band)
                children.append(child);
        qSort(children.begin(), children.end(), cmpItems);

        QList<BaseItemInterface*> activeItems ;

        foreach (BaseItemInterface * const item, children)
            item->renderSiblingsBefore(children);

        foreach (BaseItemInterface * const item, children)
            prepareItemData(item, activeItems, true);

        if (band->stretchable()) {
            foreach (BaseItemInterface * child, children) {
                if (!activeItems.contains(child))
                    continue;
                QRectF childRect = child->absoluteBoundingRect(Millimeter);
                if ( childRect.bottom() > geometry.bottom())
                    geometry.setBottom(childRect.bottom());
            }
            band->setAbsoluteGeometry(geometry);
        }

        foreach (CuteReport::BaseItemInterface * const item, children)
            item->renderSiblingsAfter(children);

        if (band->layoutType() != BandInterface::LayoutFree && !canFitBandToPage(band) ) {
            newColumnOrPage();
            if (!canFitBandToPage(band))
                createNewRenderingPage();
            geometry = band->absoluteGeometry(Millimeter);
            if (band->layoutType()== BandInterface::LayoutTop)
                geometry.moveTo(m_freeSpace.topLeft());
            else if (band->layoutType()== BandInterface::LayoutBottom)
                geometry.moveBottomLeft(m_freeSpace.bottomLeft());
            band->setAbsoluteGeometry(geometry);
        }

        RenderedItemInterface * renderedBand = band->renderView(++m_lastItemId);
        Q_ASSERT_X(renderedBand, "Item rendering object is NULL", QString("object name: \'%1\', needrendering: %2").arg(band->objectName()).arg(active).toLatin1());

        renderedBand->setId(m_lastItemId);
        m_lastIdForItem.insert(band->objectName(), m_lastItemId);
        if (band->layoutType() == BandInterface::LayoutFree)
            renderedBand->setZValue(band->order()>=0 ? (100 + band->order()) : (-100 - band->order()) ); // 100 reserverd for top and bottom
        renderedBand->setParentItem(m_currentRenderedPage);
        renderedBand->setAbsoluteGeometry(geometry, Millimeter);
        renderedBand->redraw();

        foreach (CuteReport::BaseItemInterface * const item, children) {
            if (activeItems.contains(item)) {
                placeItem(item, renderedBand, activeItems, true);
            }
        }

        foreach (CuteReport::BaseItemInterface * const item, children) {
            if (activeItems.contains(item)) {
                completePlacedItem(item, activeItems, true);
            }
        }

        if (band->layoutType() == BandInterface::LayoutTop) {
            m_freeSpace.setTop(qMax(m_freeSpace.top(), geometry.bottom()));  // band can be placed in its own logic to any position
            m_columnableBottom = qMax(m_columnableBottom, band->absoluteGeometry(Millimeter).bottom());
            if (!band->respectPageColumns() || (m_currentColumn >= m_currentPage->columns() -1) ) {
                m_columnableTop = m_freeSpace.top();
            }
        } else if (band->layoutType() == BandInterface::LayoutBottom)
            m_freeSpace.setBottom(qMin(m_freeSpace.bottom(), geometry.top()));

        if (band->respectPageColumns() && m_currentPage->columns() > 1 && m_currentPage->fillDirection() == PageInterface::Horizontal) {
            shiftColumn();
        }


        switch (stage) {
            case RenderingNewPage:  band->renderNewPageComplete(); break;
            case RenderingNormal:   band->renderEnd(); break;
        }
    }


    emit m_rendererItemInterface->bandAfter(band);

    m_state = RendererPublicInterface::DrawingContent;
    m_lastProcessedBand = band;
    m_processingBand = saveBand;
}


bool RendererProcessor::prepareItemData(CuteReport::BaseItemInterface * item, QList<CuteReport::BaseItemInterface*> & activeItems, bool withChildren)
{
    Q_ASSERT(item);
    emit log(CuteReport::LogDebug, QString("prepare item data: %1").arg(item->objectName()), "");
    processEventsIfNeeded();

    emit m_rendererItemInterface->itemBefore(item);

    bool isItemActive = item->renderBegin();

    if (isItemActive) {
        activeItems.append(item);

        if (withChildren) {
            QList<CuteReport::BaseItemInterface *> list;
            foreach (CuteReport::BaseItemInterface * child, item->findChildren<CuteReport::BaseItemInterface *>())
                if (child->parent() == item)
                    list.append(child);
            if (list.count()) {
                qSort(list.begin(), list.end(), cmpItems);
                foreach (CuteReport::BaseItemInterface * const child, list)
                    child->renderSiblingsBefore(list);
                foreach (CuteReport::BaseItemInterface * const child, list)
                    prepareItemData(child, activeItems, true);
                foreach (CuteReport::BaseItemInterface * const child, list)
                    child->renderSiblingsAfter(list);
            }
        }

        emit m_rendererItemInterface->itemDataAfter(item);

    } else {
        emit m_rendererItemInterface->itemDataAfter(item);
        emit m_rendererItemInterface->itemRenderingEndBefore(item);
        item->renderEnd();
        emit m_rendererItemInterface->itemAfter(item);
    }


    return isItemActive;
}


void RendererProcessor::placeItem(BaseItemInterface *item, RenderedItemInterface *parent, QList<BaseItemInterface*> & processedItems, bool withChildren)
{
    emit log(CuteReport::LogDebug, QString("deploy item: %1").arg(item->objectName()), "");

    emit m_rendererItemInterface->itemLayoutBefore(item);

    QRectF geometry = item->absoluteGeometry(Millimeter);

    RenderedItemInterface * renderedItem = item->renderView(++m_lastItemId);

    if (renderedItem) {

        renderedItem->setId(m_lastItemId);
        m_lastIdForItem.insert(item->objectName(), m_lastItemId);
        renderedItem->setParentItem(parent ? (QGraphicsItem *)parent : (QGraphicsItem *)m_currentRenderedPage);
        renderedItem->setAbsoluteGeometry(geometry, Millimeter);
        renderedItem->redraw();


        if (withChildren) {
            QList<CuteReport::BaseItemInterface *> list;
            foreach (CuteReport::BaseItemInterface * child, item->findChildren<CuteReport::BaseItemInterface *>())
                if (child->parent() == item)
                    list.append(child);
            qSort(list.begin(), list.end(), cmpItems);
            foreach (CuteReport::BaseItemInterface * const child, list) {
                if (processedItems.contains(child))
                    placeItem(child, renderedItem, processedItems, true);
            }
        }
    } else {
        --m_lastItemId;
    }

    emit m_rendererItemInterface->itemLayoutAfter(item);
}


void RendererProcessor::completePlacedItem(BaseItemInterface *item, QList<BaseItemInterface *> &processedItems, bool withChildren)
{
    emit log(CuteReport::LogDebug, QString("renderingEndItem item: %1").arg(item->objectName()), "");

    if (withChildren) {
        QList<CuteReport::BaseItemInterface *> list;
        foreach (CuteReport::BaseItemInterface * child, item->findChildren<CuteReport::BaseItemInterface *>())
            if (child->parent() == item)
                list.append(child);
        qSort(list.begin(), list.end(), cmpItems);
        foreach (CuteReport::BaseItemInterface * const child, list) {
            if (processedItems.contains(child))
                completePlacedItem(child, processedItems, withChildren);
        }
    }

    emit m_rendererItemInterface->itemRenderingEndBefore(item);
    item->renderEnd();
    emit m_rendererItemInterface->itemAfter(item);
}


void RendererProcessor::processDataset(DatasetInterface * dtst)
{
    emit log(LogDebug, QString("rendering dataset: %1").arg(dtst->objectName()), "");

    emit m_rendererItemInterface->datasetBefore(dtst);

    /// store dynamic data
    int currentDatasetLine = m_currentDatasetLine;
    DatasetInterface * currentDataset = m_currentDataset;

    if (!dtst->isPopulated()) {
        if (!dtst->populate()) {
            emit log(CuteReport::LogError, QString("dataset \"%1\" error").arg(dtst->objectName()),
                     QString("%1: %2").arg(dtst->objectName()).arg(dtst->getLastError()));
            terminate();
            return;
        }
    } else
        dtst->setCurrentRowNumber(0);

    m_currentDataset = dtst;
    m_currentDatasetLine = 1;

    BandsList currentGroup = bandRegisteredToDataset(dtst->objectName());
    qSort(currentGroup.begin(), currentGroup.end(), cmpBands);

    do {
        if (terminated())
            return;

        m_aggregateFunctions->processDatasetIteration(m_currentDataset);

        foreach(BandInterface * band, currentGroup)
            processBand(band, RenderingNormal);

        m_currentDatasetLine++;
    } while (dtst->setNextRow());

    foreach (BandInterface * band, currentGroup)
        if (!m_bandsDone.contains(band))
            m_bandsDone.append(band);

    /// restore dynamic data
    m_currentDatasetLine = currentDatasetLine;
    m_currentDataset = currentDataset;

    emit m_rendererItemInterface->datasetAfter(dtst);
}


bool RendererProcessor::canFitBandToPage(BandInterface * band)
{
    return (m_freeSpace.top() + band->geometry().height() <= m_freeSpace.bottom());
}


bool RendererProcessor::terminated()
{
    QMutexLocker locker(&mutex);
    return m_terminate;
}


void RendererProcessor::setValue(const QString & valueName, const QVariant &value, QScriptValue::PropertyFlags flag)
{
    m_data->scriptEngine->globalObject().setProperty(valueName, m_data->scriptEngine->newVariant(value), flag);
}


QVariant RendererProcessor::getValue(const QString & valueName)
{
    return m_data->scriptEngine->globalObject().property(valueName).toVariant();
}


CuteReport::DatasetInterface * RendererProcessor::datasetRegisteredTo(BandInterface * band)
{
    CuteReport::DatasetInterface * resultDataset = 0;
    foreach (QString dataset, m_datasetRegister.keys()) {
        BandsList list = m_datasetRegister.values(dataset);
        if (list.contains(band))
            resultDataset = m_data->renderer->reportCore()->datasetByName(dataset, m_data->workingReportCopy);
    }
    return resultDataset;
}


void RendererProcessor::registerBandToDatasetIteration(const QString & datasetName, const QString & objectName)
{
    BaseItemInterface * item = m_data->renderer->reportCore()->itemByName(objectName, m_currentPage);
    BandInterface * band = qobject_cast<BandInterface*>(item);
    if (band)
        registerBandToDatasetIteration(datasetName, band);
}


void RendererProcessor::registerBandToDatasetIteration(const QString & datasetName, CuteReport::BandInterface * band)
{
    if (band && !m_datasetRegister.values(datasetName).contains(band) )
        m_datasetRegister.insertMulti(datasetName, band);
}


void RendererProcessor::unregisterBandFromDatasetIteration(const QString & datasetName, const QString & objectName)
{
    BaseItemInterface * item = m_data->renderer->reportCore()->itemByName(objectName, m_currentPage);
    BandInterface * band = qobject_cast<BandInterface*>(item);

    if (band)
        unregisterBandFromDatasetIteration(datasetName, band);
}


void RendererProcessor::unregisterBandFromDatasetIteration(const QString & datasetName, CuteReport::BandInterface * band)
{
    if (band && !m_datasetRegister.values(datasetName).contains(band) ) {
        QMutableHashIterator<QString, BandInterface*> i(m_datasetRegister);
        while (i.hasNext()) {
            i.next();
            if  (i.value() == band)
                i.remove();
        }
    }
}


BandsList RendererProcessor::bandRegisteredToDataset(const QString & datasetName)
{
    return m_datasetRegister.values(datasetName);
}


const CuteReport::BandInterface * RendererProcessor::getBandForItem(const QObject * object)
{
    if (!object)
        return 0;

    CuteReport::BandInterface* band = qobject_cast<CuteReport::BandInterface*> (object);
    if (band)
        return 0;

    const CuteReport::ItemInterface * item = dynamic_cast<const CuteReport::ItemInterface*>(object);
    if (!item)
        return 0;

    BaseItemInterface * parent = item->parentItem();

    if (!parent)
        return 0;

    CuteReport::BandInterface* parentBand = qobject_cast<CuteReport::BandInterface*> (parent);

    if (parentBand)
        return parentBand;

    return getBandForItem(parent);
}


void RendererProcessor::processString(ScriptString &scriptString)
{
    if (scriptString.expressionType == StringHasNoExpression) {

    } else  if (scriptString.expressionType == StringHasExpression) {
        // escaping all delimiter characters to prevent problems
        QString regexp;
        QString delimiterBegin = scriptString.expDelimiter.isEmpty() ? "[" : scriptString.expDelimiter.section(",", 0, 0).trimmed();
        QString delimiterEnd = scriptString.expDelimiter.isEmpty() ? "]" : scriptString.expDelimiter.section(",", 1, 1).trimmed();
        for (int i=0; i<delimiterBegin.length(); ++i)
            regexp+=QString("\\") + delimiterBegin.at(i);
        regexp+="(.*)";
        for (int i=0; i<delimiterEnd.length(); ++i)
            regexp+= QString("\\") + delimiterEnd.at(i);

        QRegExp rx(regexp);
        rx.setMinimal(true);

        int pos = 0;
        while ((pos = rx.indexIn(scriptString.resultStr, pos)) != -1) {
            int length = rx.matchedLength();

            if (length == 0) // something wrong, exit
                break;

            QString expression = rx.cap(1);
            QString evaluationResult = _processString(expression, scriptString);

            scriptString.resultStr.replace(pos, length, evaluationResult);
            pos += evaluationResult.length();
        }
    } else if (scriptString.expressionType == StringIsExpression) {
        scriptString.resultStr = _processString(scriptString.origStr, scriptString);
    }
}


QString RendererProcessor::_processString(const QString & string, ScriptString &scriptString)
{
    QString expression = preprocessEvaluateString(string, scriptString.object);
    QString evaluationResult;

    if (m_data->scriptEngine->canEvaluate(expression)) {
        QScriptValue evaluateValue = m_data->scriptEngine->evaluate(expression);
        if (m_data->scriptEngine->hasUncaughtException()) {
            QString _message = QString("script error at line %1 \n%2")
                               .arg(m_data->scriptEngine->uncaughtExceptionLineNumber())
                               .arg(m_data->scriptEngine->uncaughtException().toString());
            if (scriptString.object) {
                emit log(LogError, QString("Script evaluation error for item \'%1\'").arg(scriptString.object->objectName()),
                         QString("Script evaluation error for item \'%1\': %2").arg(scriptString.object->objectName()).arg(_message));
            } else {
                emit log(LogError, QString("Script evaluation error"),
                         QString("script:\'%1\' error: \'%2\'").arg(string).arg(_message));
            }
            scriptString.result = false;
            scriptString.error = _message;
        } else {
            evaluationResult = evaluateValue.toString();
        }
    }

    return evaluationResult;
}


RendererItemInterface *RendererProcessor::rendererItemInterface() const
{
    return m_rendererItemInterface;
}


void RendererProcessor::sendLog(LogLevel level, const QString &shortMessage, const QString &fullMessage)
{
    emit log(level, shortMessage, fullMessage);
}


AggregateFunctions *RendererProcessor::aggregateFunctions() const
{
    return m_aggregateFunctions;
}


void RendererProcessor::registerRenderingStrings(const QList<InternalStringData> &strDatas, BaseItemInterface *item)
{
    foreach (const InternalStringData & strData, strDatas) {
        switch (strData.expressionType) {
            case StringHasNoExpression:
                break;
            case StringHasExpression:
                registerEvaluationString(strData.str, item);
                break;
            case StringIsExpression:
                QString expressionStart = strData.expDelimiter.isEmpty() ? "[" : strData.expDelimiter.section(",",0,0).trimmed();
                QString expressionEnd = strData.expDelimiter.isEmpty() ? "]" : strData.expDelimiter.section(",",1,1).trimmed();
                registerEvaluationString(strData.str, expressionStart, expressionEnd, item);
                break;
        }
    }
}



void RendererProcessor::registerEvaluationString(const QString & string, const QString & delimiterBegin, const QString & delimiterEnd, CuteReport::BaseItemInterface *item)
{
    // escaping all delimiter characters for prevent problems
    QString regexp;
    for (int i=0; i<delimiterBegin.length(); ++i)
        regexp+=QString("\\") + delimiterBegin.at(i);
    regexp+="(.*)";
    for (int i=0; i<delimiterEnd.length(); ++i)
        regexp+= QString("\\") + delimiterEnd.at(i);


    QRegExp rx(regexp);
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(string, pos)) != -1) {
        int length = rx.matchedLength();

        if (length == 0) // something wrong, exit
            break;

        QString expression = rx.cap(1);
        registerEvaluationString(expression, item);

        pos += length;
    }
}


void RendererProcessor::registerEvaluationString(const QString & string, BaseItemInterface *item)
{
    if (string.isEmpty())
        return;

    QString str(string);

    const BandInterface * band = getBandForItem(item);
    QStringList errors;
    QStringList moduleErrors;

    bool result = true;
    result &= m_preparser->initialItemScriptPreprocess(str, item, &moduleErrors);
    errors << moduleErrors;
    result &= m_variables->initialItemScriptPreprocess(str, item, &moduleErrors);
    errors << moduleErrors;
    result &= m_aggregateFunctions->initialItemScriptPreprocess(str, band ? band->objectName() : QString(), m_data->workingReportCopy, &moduleErrors);
    errors << moduleErrors;

    foreach (const QString & error, errors) {
        emit log(LogError, QString("Script preprocess error for report:\'%1\' item:\'%2\'")
                 .arg(m_data->workingReportCopy->objectName())
                 .arg(item ? item->objectName() : "Undefined"), error);
    }
}


QString RendererProcessor::preprocessEvaluateString(const QString &str, const QObject *object)
{
    QString resultStr = str;

    const BandInterface * band = getBandForItem(object);
    m_preparser->itemScriptPreprocess(resultStr, object, 0);
    m_aggregateFunctions->itemScriptPreprocess(resultStr, band ? band->objectName() : QString(), m_data->workingReportCopy, 0);

    return resultStr;
}


bool RendererProcessor::preprocessScript(QString &str)
{
    QStringList errors;
    QStringList moduleErrors;
    bool result = true;
    result &= m_preparser->mainScriptPreprocess(str, &moduleErrors);
    errors << moduleErrors;
    result &= m_aggregateFunctions->mainScriptPreprocess(str, m_data->workingReportCopy, &moduleErrors);
    errors << moduleErrors;

    foreach (const QString & error, errors) {
        emit log(LogError, QString("Script preprocess error for report \'%1\'").arg(m_data->workingReportCopy->objectName()), error);
    }

    return result;
}


QSet<QString> RendererProcessor::reportParameters(ReportInterface *report)
{
    QSet<QString> set = m_preparser->findParameterVariables(report->script());

    foreach (PageInterface * page, report->pages()) {
        foreach (BaseItemInterface * item, page->items()) {
            QList<CuteReport::InternalStringData>  scriptingStrings = item->renderingStrings();
            foreach (const CuteReport::InternalStringData & strData, scriptingStrings) {
                if (strData.expressionType == StringHasNoExpression)
                    continue;
                set.unite( m_preparser->findParameterVariables(strData.str) );
            }
        }
    }

    foreach (DatasetInterface * ds, report->datasets()) {
        QList<CuteReport::InternalStringData> renderingStrings = ds->renderingStrings();
        foreach (const CuteReport::InternalStringData & strData, renderingStrings) {
            set.unite( m_preparser->findParameterVariables(strData.str) );
        }
    }

    foreach (FormInterface * form, report->forms()) {
        QStringList renderingStrings = form->renderingStrings();
        foreach (const QString & str, renderingStrings) {
            set.unite( m_preparser->findParameterVariables(str) );
        }
    }
    return set;
}


SUIT_END_NAMESPACE
