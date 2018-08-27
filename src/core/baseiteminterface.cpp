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

#include "baseiteminterface.h"
#include "baseiteminterface_p.h"
#include "iteminterfaceview.h"
#include "renderediteminterface.h"
#include "pageinterface.h"
#include "reportinterface.h"
#include "bandinterface.h"
#include "reportcore.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

namespace CuteReport
{

static const char * BorderTypeStr[] = {"Middle", "Inner", "Outer"};
static const int BOrderTypeNum = 3;

TemplateItemDetailLevel BaseItemInterface::g_templateDetailLevel = TemplateDetailLevel_Full;

BaseItemInterface::BaseItemInterface(QObject * parent) :
    ReportPluginInterface(parent),
    d_ptr(new BaseItemInterfacePrivate),
    orig_ptr(0)
{
    initMe();
}


BaseItemInterface::BaseItemInterface(BaseItemInterfacePrivate *dd, QObject * parent)
    :ReportPluginInterface(parent),
      d_ptr(dd),
      orig_ptr(0)
{
    initMe();
}


void BaseItemInterface::pushContext(BaseItemInterfacePrivate *r)
{
    if (!orig_ptr) {
        orig_ptr = d_ptr;   // a pointer in dataStack so we will be able to recognize rendering stack bottom
    }

    dataStack.push_back(d_ptr);
    d_ptr = r;
    d_ptr->renderingType = RenderingReportBefore;

//    Q_ASSERT(!orig_ptr);
//    //delete orig_ptr;s
//    orig_ptr = d_ptr;
//    d_ptr = r;
//    d_ptr->renderingType = RenderingReportBefore;
}


void BaseItemInterface::popContext()
{
    if (dataStack.isEmpty())
        return;

//    Q_ASSERT_X(orig_ptr, "popContext", "Error in Rendered while trying to pop context on an empty stack.");
    if (!orig_ptr) {
        ReportCore::log(CuteReport::LogCritical, objectName(), "Renderer critical issue", "Error in Rendered while trying to pop context on an empty stack. Report the bug to developers.");
    }


    delete d_ptr;
    d_ptr = dataStack.takeLast();

    if (d_ptr == orig_ptr) {// we have reach bottom of the rendering stack
        orig_ptr = 0;
    }

//    if (!orig_ptr)
//        return;
//    delete d_ptr;
//    d_ptr = orig_ptr;
//    orig_ptr = 0;
}


BaseItemInterface::~BaseItemInterface()
{
    delete d_ptr;
    delete orig_ptr;
    qDeleteAll(dataStack);
}


bool BaseItemInterface::moduleInit()
{
    return true;
}


void BaseItemInterface::initMe()
{
    Q_D(BaseItemInterface);
    d->state = NotInitedState;
    m_gui = 0;
    d->resizeFlags = ResizeTop | ResizeBottom | ResizeLeft | ResizeRight;
    m_currentProperty = -1;
    m_page = 0;
    m_parentItem = 0;
    m_templateDetailLevel = g_templateDetailLevel;
}


void BaseItemInterface::init()
{
    Q_D(BaseItemInterface);
    if (d->state != NotInitedState)
        return;

    m_parentItem = qobject_cast<CuteReport::BaseItemInterface*>(parent());
    m_page = m_parentItem ? BaseItemInterface::itemPage(m_parentItem) : qobject_cast<CuteReport::PageInterface*>(parent());

    updateMeassure();

    d->state = NormalState;
}


void BaseItemInterface::check_gui()
{
    if (!m_gui)
        init_gui();
}


void BaseItemInterface::update_gui()
{
    if (!m_gui || d_ptr->state == DeleteState)
        return;

    QRectF itemPixelRect = convertUnit(absoluteGeometry(), d_ptr->unit, Pixel, d_ptr->dpi);

#if QT_VERSION >= 0x050000
    itemPixelRect.setBottom(itemPixelRect.bottom() -1);
    itemPixelRect.setRight(itemPixelRect.right() -1);
#endif

    QGraphicsItem * pageItem = page() ? page()->pageItem() : 0;
    QGraphicsItem * parentGuiItem = parentItem() ? (QGraphicsItem*)parentItem()->view() : pageItem;
    m_gui->setParentItem( parentGuiItem );

    QPointF itemPixelpos = itemPixelRect.topLeft();
    QPointF absPos = pageItem ? pageItem->mapToScene(itemPixelpos) : itemPixelpos;
    QPointF pos = parentGuiItem ? parentGuiItem->mapFromScene(absPos) : absPos;

    m_gui->setRect(0,0, itemPixelRect.width(), itemPixelRect.height());
    m_gui->setZValue(order());

    m_gui->setRotation(-d_ptr->rotation);

    QPointF transPos = BaseItemInterface::transformedPos(d_ptr, QRectF(pos, itemPixelRect.size()));
    m_gui->setPos(transPos);

    m_gui->update();
}


BaseItemInterface *BaseItemInterface::clone(bool withChildren, bool init) const
{
    BaseItemInterface * newItem = this->itemClone();
    newItem->setReportCore(this->reportCore());
    newItem->setObjectName(this->objectName());

    if (withChildren) {
        foreach (CuteReport::BaseItemInterface* child, findChildren<BaseItemInterface * >()) {
            if (child->parent() != this)     // only direct children
                continue;
            CuteReport::BaseItemInterface * childCopy = child->clone(withChildren, init);
            childCopy->setParent(newItem);
        }
    }
    if  (init)
        newItem->init();

    return newItem;
}


PageInterface * BaseItemInterface::page() const
{
    return m_page;
}


void BaseItemInterface::setPage(PageInterface * page)
{
    Q_D(BaseItemInterface);
    if (!QObject::parent())
        QObject::setParent(page);
    m_page = page;
    if (m_page) {
        d->unit = m_page->unit();
        d->dpi = m_page->dpi();
        update_gui();
    }
}


BaseItemInterface * BaseItemInterface::parentItem() const
{
    return m_parentItem;
}


void BaseItemInterface::setParentItem(BaseItemInterface *parentItem)
{
    Q_D(BaseItemInterface);
    if (parentItem == m_parentItem)
        return;

    if (m_parentItem)
        m_parentItem->childRemoved(this);

    QObject::setParent(parentItem);

    m_parentItem = parentItem;
    adjustOrder();

    if (m_parentItem)
        m_parentItem->childAdded(this);

    NOTPROPERTY_CHANGE_SIGNALS(parentItemChanged, m_parentItem)
}


CuteReport::ReportInterface * BaseItemInterface::reportObject()
{
    QObject * object = this->parent();
    while (!dynamic_cast<CuteReport::ReportInterface*> (object) && object)
        object = object->parent();

    return dynamic_cast<CuteReport::ReportInterface*> (object);
}


int BaseItemInterface::childLevel()
{
    return m_parentItem ? m_parentItem->childLevel() + 1 : 0;
}


BandInterface *BaseItemInterface::carrierBand()
{
    if (qobject_cast<CuteReport::BandInterface*> (this))
        return static_cast<CuteReport::BandInterface*>(this);

    if (parentItem())
        return parentItem()->carrierBand();
    else
        return 0;
}


BaseItemInterface * BaseItemInterface::topmostChildAt(const QPointF & localPos)
{
    QList<BaseItemInterface *>  list = childrenAt(localPos);

    while (true) {
        QList<BaseItemInterface *>  nextLevelList;
        foreach (BaseItemInterface * child, list)
            nextLevelList.append(child->childrenAt(child->mapFromParent(localPos)));
        if (nextLevelList.size())
            list = nextLevelList;
        else
            break;
    }

    if (list.size() == 1)
        return list[0];
    else if (list.size() > 1) {
        BaseItemInterface * item = list[0];
        for (int i = 1; i<list.size(); ++i) {
            if (list[i]->order() > item->order())
                item = list[i];
        }
        return item;
    }
    return 0;
}


QList<BaseItemInterface *> BaseItemInterface::childrenAt(const QPointF & localPos)
{
    QList<BaseItemInterface *>  list;

    foreach (BaseItemInterface * const item, findChildren<BaseItemInterface *>()){
        if (item->parent() != this)
            continue;
        QRectF childGeometry = item->geometry();
        if (childGeometry.contains(localPos)) {
            list.append(item);
        }
    }
    return list;
}


QList<BaseItemInterface *> BaseItemInterface::allChildrenAt(const QPointF & localPos)
{
    QList<BaseItemInterface *>  list;
    foreach (BaseItemInterface * const item, childrenAt(localPos)){
        if (item->parent() != this) // only direct children checking
            continue;
        QList<BaseItemInterface *> nextLevelList = item->allChildrenAt(item->mapFromParent(localPos));
        if (!nextLevelList.isEmpty())
            list.append(nextLevelList);
    }
    return list;
}


bool BaseItemInterface::canContain(QObject * object)
{
    return qobject_cast<BaseItemInterface*>(object);
}


ItemInterfaceView *BaseItemInterface::view()
{
    check_gui();
    return m_gui;
}


void BaseItemInterface::saveState()
{
    BaseItemInterfacePrivate * state = new BaseItemInterfacePrivate(*d_ptr);
    dataStack.append(state);
}


void BaseItemInterface::restoreState()
{
    if (!dataStack.isEmpty()) {
        if (d_ptr)
            delete (d_ptr);
        d_ptr = dataStack.takeLast();
    }
}


void BaseItemInterface::renderInitBase(CuteReport::ScriptEngineInterface *scriptEngine, CuteReport::InitFlags flags)
{
    Q_UNUSED(scriptEngine);
    Q_D(BaseItemInterface);
    d->state = RenderingReadyState;

    delete orig_ptr;
    orig_ptr = 0;

    emit printInit();
}


void BaseItemInterface::rendererBeginBaseStart(BaseItemInterfacePrivate *p)
{
    Q_D(BaseItemInterface);
    d->state = RenderingState;
    emit printCreateBefore();
    pushContext(p);
    emit printDataBefore();
}


void BaseItemInterface::rendererBeginBaseEnd()
{
    emit printBefore();
}


void BaseItemInterface::renderReset()
{
    renderResetBase();
}


bool BaseItemInterface::renderEnd()
{
    popContext();
    Q_D(BaseItemInterface);
    d->state = RenderingReadyState;
    emit printAfter();
    return true;
}


void BaseItemInterface::renderResetBase()
{
    Q_D(BaseItemInterface);
    emit printReset();
    d->state = NormalState;
    update_gui();
}



bool BaseItemInterface::selfRendering()
{
    return d_ptr->selfRendering;
}


bool BaseItemInterface::childrenSelfRendering()
{
    return d_ptr->childrenSelfRendering;
}


int BaseItemInterface::resizeFlags() const
{
    Q_D(const BaseItemInterface);
    return d->resizeFlags;
}

void BaseItemInterface::setResizeFlags(int resizeFlags)
{
    Q_D(BaseItemInterface);
    d->resizeFlags = resizeFlags;
}


int BaseItemInterface::baseItemFlags() const
{
    Q_D(const BaseItemInterface);
    return d->baseItemFlags;
}

void BaseItemInterface::setBaseItemFlags(int flags, bool value)
{
    Q_D(BaseItemInterface);
    int newValue = d->baseItemFlags;
    if (value)
        newValue |= flags;
    else
        newValue &= ~flags;
    PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(baseItemFlags, newValue)
}


//void BaseItemInterface::setBaseItemFlag(int flag, bool value)
//{
//    Q_D(BaseItemInterface);
//    BaseItemInterface::BaseItemFlags baseItemFlags = d->baseItemFlags;
//    if (value)
//        baseItemFlags |= flag;
//    else
//        baseItemFlags &= ~flag;
//    PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(baseItemFlags, baseItemFlags)
//}


bool BaseItemInterface::isBaseItemFlagSet(int flag)
{
    Q_D(const BaseItemInterface);
    return d->baseItemFlags & flag;
}




bool BaseItemInterface::enabled() const
{
    Q_D(const BaseItemInterface);
    return d->enabled;
}


void BaseItemInterface::setEnabled(bool b)
{
    Q_D(BaseItemInterface);
    PROCESS_NEW_PROPERTY_VALUE(enabled, b);
}


int BaseItemInterface::order() const
{
    Q_D(const BaseItemInterface);
    return d->order;
}


void BaseItemInterface::setOrder(int order)
{
    Q_D(BaseItemInterface);
    int newValue = order < 0 ? 0 : order;
    PROCESS_NEW_PROPERTY_VALUE(order, newValue)
}


void BaseItemInterface::setDpi(int dpi)
{
    Q_D(BaseItemInterface);
    SAVEIFCHANGED(dpi, dpi)

    update_gui();

    emit dpiChanged(d->dpi);
    emit boundingRectChanged();
    /// we dont need changed() signal since it causes dirty state and we don't need dirtyness for dpi
}


int BaseItemInterface::dpi() const
{
    Q_D(const BaseItemInterface);
    return d->dpi;
}


QByteArray BaseItemInterface::serialize()
{
    QByteArray ba;
    QDataStream s( &ba, QIODevice::ReadWrite );
    s << *d_ptr;
    return ba;
}


void BaseItemInterface::deserialize(QByteArray & data)
{
    QDataStream s( &data, QIODevice::ReadWrite );
    s >> *d_ptr;
}


void BaseItemInterface::paintBegin(QPainter * painter, const QStyleOptionGraphicsItem *option, BaseItemInterfacePrivate *data,
                                   const QRectF &boundingRect, RenderingType type )
{
    Q_UNUSED(type);

    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing | QPainter::NonCosmeticDefaultPen);
#if QT_VERSION >= 0x050000
    painter->setRenderHint(QPainter::Qt4CompatiblePainting);
#endif

    const BaseItemInterfacePrivate * const d = data /*? data : d_func()*/;

    painter->setPen(Qt::NoPen);
    painter->setOpacity(d->opacity);
    painter->setBrush(d->backgroundBrush);

    QRectF rect = boundingRect;
    QPen borderPen = penPrepared(d->borderPen, d->dpi);
    qreal penWidth = borderPen.widthF();
    qreal delta = 0;

    switch(d->borderType) {
        case Middle: delta = 0; break;
        case Inner: delta = penWidth / 2; break;
        case Outer: delta = -penWidth / 2; break;
    }

#if QT_VERSION >= 0x050000
//    if (penWidth&1)
//        rect = rect.adjusted( delta, delta, -delta, -delta );
//    else
        rect = rect.adjusted( delta, delta, -delta+1, -delta+1 );
#else
//    if (penWidth&1)
//        rect = rect.adjusted( delta, delta, -delta, -delta );
//    else
        rect = rect.adjusted( delta, delta, -delta, -delta );
#endif

    painter->drawRect(rect);

    painter->save();
}


void BaseItemInterface::paintEnd(QPainter * painter, const QStyleOptionGraphicsItem *option, BaseItemInterfacePrivate * data, const QRectF &boundingRect, RenderingType type )
{
    Q_UNUSED(type);

    BaseItemInterfacePrivate * const d = data /*? data : d_func()*/;

    painter->restore();

    /// border pen
    QPen borderPen = penPrepared(d->borderPen, d->dpi);
    painter->setPen(borderPen);
    painter->setOpacity(d->opacity);

    /// background brush
    painter->setBrush(Qt::NoBrush);

//    QRect area = paintArea(data, boundingRect);

//    QRectF rect = (option->type == QStyleOption::SO_GraphicsItem) ? boundingRect : option->exposedRect;
    QRectF rect = boundingRect;

    qreal penWidth = borderPen.widthF();
    qreal delta = 0;

    switch(d->borderType) {
        case Middle: delta = 0; break;
        case Inner: delta = penWidth / 2; break;
        case Outer: delta = -penWidth / 2; break;
    }

#if QT_VERSION >= 0x050000
//    if (penWidth&1)
//        rect = rect.adjusted( delta, delta, -delta, -delta );
//    else
        rect = rect.adjusted( delta, delta, -delta+1, -delta+1 );
#else
//    if (penWidth&1)
//        rect = rect.adjusted( delta, delta, -delta, -delta );
//    else
        rect = rect.adjusted( delta, delta, -delta, -delta );
#endif

    painter->setRenderHint (QPainter::Antialiasing, false);

    int _frame = d->frame;

    if(_frame & DrawLeft && _frame & DrawRight && _frame & DrawTop && _frame & DrawBottom) {
        painter->drawRect(rect);
    } else {
        painter->setPen( QPen(Qt::NoPen));
        painter->drawRect(rect);

        if (type == RenderingTemplate) {
            painter->save();
            QPen pen(QColor(128,128,128));
            pen.setStyle(Qt::DashDotLine);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->setOpacity(0.5);
            painter->drawRect(rect);
            painter->restore();
        }

        painter->setPen(borderPen);
        painter->setOpacity(d->opacity);

        if (_frame&DrawLeft)
            painter->drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());

        if (_frame&DrawRight)
            painter->drawLine(rect.right(), rect.top(), rect.right(), rect.bottom());

        if (_frame&DrawTop)
            painter->drawLine(rect.left(), rect.top(), rect.right(), rect.top());

        if (_frame&DrawBottom)
            painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    }

    if (type == RenderingTemplate && d->baseItemFlags & BaseReportItem) {
        // TODO: optimize by using static QPixmap pointer
        static QImage lockImageOrig(":images/lock.png");
        if (!d->lockImage){
            d->lockImage = new QPixmap();
        }
        QSize pixmapSize = convertUnit(QSize(2.5,2.5), CuteReport::Millimeter, CuteReport::Pixel, d->dpi).toSize();
        if (d->lockImage->size() != pixmapSize)
            d->lockImage->convertFromImage(lockImageOrig.scaled(pixmapSize));
        painter->setOpacity(0.7);
        painter->drawPixmap(rect.topLeft(), *d->lockImage);
    } else {
        if (d->lockImage) {
            delete d->lockImage;
            d->lockImage = 0;
        }
    }
}


QRectF BaseItemInterface::paintArea(const BaseItemInterfacePrivate *data, const QRectF &boundingRect)
{
//    const BaseItemInterfacePrivate * const d = data;

    //QRect rect = (option->type == QStyleOption::SO_GraphicsItem) ? boundingRect.toRect() : option->exposedRect.toRect();
    QRectF rect = boundingRect;
    qreal value = 0;
    if (data->frame != 0) {
        borderWidth(data, Pixel, &value);
        qreal delta = value;
        if (delta != 0)
            rect = rect.adjusted( delta, delta, -delta, -delta );

        #if QT_VERSION >= 0x050000
            rect = rect.adjusted( 0, 0, 1, 1 );
        #endif
    }

//    int penWidth = d->borderPen.widthF()/72*d->dpi;
//    int delta = floor(penWidth/2+0.5);

//    switch(d->borderType) {
//        case Middle: rect = rect.adjusted( delta, delta, -delta, -delta ); break;
//        case Inner: rect = rect.adjusted( penWidth, penWidth, -penWidth, -penWidth ); break;
//        case Outer: rect = rect.adjusted( -penWidth, -penWidth, +penWidth, +penWidth ); break;
//    }

//    if (penWidth&1)
//        rect = rect.adjusted( delta, delta, -delta+1, -delta+1 );
//    else
//        rect = rect.adjusted( delta, delta, -delta, -delta );

    //#if QT_VERSION >= 0x050000
    //    if (penWidth&1)
    //        rect = rect.adjusted( delta, delta, -delta, -delta );
    //    else
    //        rect = rect.adjusted( delta, delta, -delta+1, -delta+1 );
    //#else
    //    if (penWidth&1)
    //        rect = rect.adjusted( delta, delta, -delta, -delta );
    //    else
    //        rect = rect.adjusted( delta, delta, -delta, -delta );
    //#endif

#if QT_VERSION >= 0x050000
//    rect = rect.adjusted( 0, 0, +1, +1);
#endif

    return rect;
}


QRectF BaseItemInterface::mapFromPage(const QRectF & rect, Unit inputUnit, Unit outputUnit) const
{
    Unit unitIn = (inputUnit == UnitNotDefined) ? d_ptr->unit : inputUnit;
    Unit unitOut = (outputUnit == UnitNotDefined) ? d_ptr->unit : outputUnit;
    QRectF inputRectMM =  convertUnit(rect, unitIn, Millimeter, d_ptr->dpi);
    QRectF resultRect = m_parentItem ? m_parentItem->mapFromPage(inputRectMM.translated(-d_ptr->geometry.topLeft()), Millimeter, Millimeter) : inputRectMM.translated(-d_ptr->geometry.topLeft());
    return convertUnit(resultRect, Millimeter, unitOut, d_ptr->dpi);
}


QPointF BaseItemInterface::mapFromPage(const QPointF & point, Unit inputMeassure, Unit outputMeassure) const
{
    Unit unitIn = (inputMeassure == UnitNotDefined) ? d_ptr->unit : inputMeassure;
    Unit unitOut = (outputMeassure == UnitNotDefined) ? d_ptr->unit : outputMeassure;
    QPointF inputPointMM =  convertUnit(point, unitIn, Millimeter, d_ptr->dpi);
    QPointF resultPoint = m_parentItem ? m_parentItem->mapFromPage(inputPointMM - d_ptr->geometry.topLeft(), Millimeter, Millimeter) : inputPointMM - d_ptr->geometry.topLeft();
    return convertUnit(resultPoint, Millimeter, unitOut, d_ptr->dpi);
}


QRectF BaseItemInterface::mapToPage(const QRectF & rect, Unit inputMeassure, Unit outputMeassure) const
{
    Unit unitIn = (inputMeassure == UnitNotDefined) ? d_ptr->unit : inputMeassure;
    Unit unitOut = (outputMeassure == UnitNotDefined) ? d_ptr->unit : outputMeassure;
    QRectF inputRectMM =  convertUnit(rect, unitIn, Millimeter, d_ptr->dpi);
    QRectF resultRect = m_parentItem ? m_parentItem->mapToPage(inputRectMM.translated(d_ptr->geometry.topLeft()), Millimeter, Millimeter) : inputRectMM.translated(d_ptr->geometry.topLeft());
    return convertUnit(resultRect, Millimeter, unitOut, d_ptr->dpi);
}


QPointF BaseItemInterface::mapToPage(const QPointF & point, Unit inputMeassure, Unit outputMeassure) const
{
    Unit unitIn = (inputMeassure == UnitNotDefined) ? d_ptr->unit : inputMeassure;
    Unit unitOut = (outputMeassure == UnitNotDefined) ? d_ptr->unit : outputMeassure;
    QPointF inputPointMM =  convertUnit(point, unitIn, Millimeter, d_ptr->dpi);
    QPointF resultPoint = m_parentItem ?  m_parentItem->mapFromPage(inputPointMM + d_ptr->geometry.topLeft(), Millimeter, Millimeter) : inputPointMM + d_ptr->geometry.topLeft();
    return convertUnit(resultPoint, Millimeter, unitOut, d_ptr->dpi);
}


QRectF BaseItemInterface::mapToParent(const QRectF & rect, Unit inputMeassure, Unit outputMeassure) const
{
    Unit unitIn = (inputMeassure == UnitNotDefined) ? d_ptr->unit : inputMeassure;
    Unit unitOut = (outputMeassure == UnitNotDefined) ? d_ptr->unit : outputMeassure;
    QRectF inputRectMM =  convertUnit(rect, unitIn, Millimeter, d_ptr->dpi);
    QRectF resultRect = inputRectMM.translated(d_ptr->geometry.topLeft());
    return convertUnit(resultRect, Millimeter, unitOut, d_ptr->dpi);
}


QPointF BaseItemInterface::mapToParent(const QPointF & point, Unit inputMeassure, Unit outputMeassure) const
{
    Unit unitIn = (inputMeassure == UnitNotDefined) ? d_ptr->unit : inputMeassure;
    Unit unitOut = (outputMeassure == UnitNotDefined) ? d_ptr->unit : outputMeassure;
    QPointF inputPointMM =  convertUnit(point, unitIn, Millimeter, d_ptr->dpi);
    QPointF resultPoint = inputPointMM + d_ptr->geometry.topLeft();
    return convertUnit(resultPoint, Millimeter, unitOut, d_ptr->dpi);
}


QRectF BaseItemInterface::mapFromParent(const QRectF & rect, Unit inputMeassure, Unit outputMeassure) const
{
    Unit unitIn = (inputMeassure == UnitNotDefined) ? d_ptr->unit : inputMeassure;
    Unit unitOut = (outputMeassure == UnitNotDefined) ? d_ptr->unit : outputMeassure;
    QRectF inputRectMM =  convertUnit(rect, unitIn, Millimeter, d_ptr->dpi);
    QRectF resultRect = inputRectMM.translated(-d_ptr->geometry.topLeft());
    return convertUnit(resultRect, Millimeter, unitOut, d_ptr->dpi);
}


QPointF BaseItemInterface::mapFromParent(const QPointF & point, Unit inputUnit, Unit outputUnit) const
{
    Unit unitIn = (inputUnit == UnitNotDefined) ? d_ptr->unit : inputUnit;
    Unit unitOut = (outputUnit == UnitNotDefined) ? d_ptr->unit : outputUnit;
    QPointF inputPointMM =  convertUnit(point, unitIn, Millimeter, d_ptr->dpi);
    QPointF resultPoint = inputPointMM - d_ptr->geometry.topLeft();
    return convertUnit(resultPoint, Millimeter, unitOut, d_ptr->dpi);
}


QRectF BaseItemInterface::absoluteGeometry(Unit outputUnit) const
{
    return mapToPage(QRectF(QPointF(0,0), QSizeF(d_ptr->geometry.size())), Millimeter, outputUnit);
}


void BaseItemInterface::setAbsoluteGeometry(const QRectF & rect, Unit inputUnit)
{
    setGeometry(m_parentItem ? m_parentItem->mapFromPage(rect, inputUnit, Millimeter) : rect, Millimeter);
}


QRectF BaseItemInterface::absoluteBoundingRect(Unit unit, bool withTransformation, bool withBorder) const
{
    QRectF rect = boundingRect(Millimeter, withTransformation, withBorder);
//    QRectF rect = QRectF(QPointF(0,0), QSizeF(d_ptr->geometry.size()));
//    QRectF resRect = QRectF(QPointF(0,0), transform().mapRect(rect).size());
    return mapToPage(rect, Millimeter, unit);
}


QPolygonF BaseItemInterface::absolutePolygon(Unit unit) const
{
    return transform().map(QPolygonF(absoluteGeometry(unit)));
}


CuteReport::PageInterface * BaseItemInterface::itemPage(CuteReport::BaseItemInterface * item)
{
    if (item->page())
        return item->page();
    else {
        CuteReport::BaseItemInterface * parentItem = item->parentItem();
        if (parentItem)
            return itemPage(parentItem);
    }
    return 0;
}


QFont BaseItemInterface::fontPrepared(const QFont & font, qint16 dpi)
{
    QFont f(font);
    qreal pixels = f.pointSizeF()/72*dpi;
    f.setPixelSize(pixels);
    return f;
}


QPen BaseItemInterface::penPrepared(const QPen &pen, qint16 dpi)
{
    QPen p(pen);
    qreal pixels = (p.widthF() * dpi) / 25.4; //widthF is always in mm
    p.setWidthF(pixels);
    p.setCosmetic(false);
    return p;
}


const QRectF BaseItemInterface::adjustRect(QRectF & rect, const QPen & pen)
{
    Q_UNUSED(pen);
    return rect;
}


void BaseItemInterface::adjustOrder()
{
    if (!parent())
        return;

    int maxOrder = 0;
    foreach (BaseItemInterface * item, parent()->findChildren<CuteReport::BaseItemInterface*>()) {
        if (item == this || item->parent() != parent())
            continue;
        if (item->order() > maxOrder)
            maxOrder = item->order();
    }

    setOrder(maxOrder+1);
}


QString BaseItemInterface::_current_property_description() const
{
    QString propertyName = metaObject()->property(m_currentProperty).name();

    if (propertyName == "objectName")
        return tr("Item name");
    else if (propertyName == "geometry")
        return tr("Item geomery");
    else if (propertyName == "frame")
        return tr("Item frame drawing sides");
    else if (propertyName == "opacity")
        return tr("Item opacity");
    else if (propertyName == "rotation")
        return tr("Rotation angle");
    else if (propertyName == "borderPen")
        return tr("Border pen properties");
    else if (propertyName == "backgroundBrush")
        return tr("Border pen background properies");
    else if (propertyName == "enabled")
        return tr("Defines if item enabled or not. Disabled items will not be processed.");
    else if (propertyName == "order")
        return tr("Position order");

    return QString();
}


int BaseItemInterface::_current_property_precision() const
{
    return 1;
}

////////////////////////////////////////// shared data

Unit BaseItemInterface::unit() const
{
    Q_D(const BaseItemInterface);
    return d->unit;
}


void BaseItemInterface::setUnit(Unit unit)
{
    Q_D(BaseItemInterface);
    PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(unit,unit)
}


QRectF BaseItemInterface::geometry(Unit unit) const
{
    Q_D(const BaseItemInterface);
    if (d->state == NotInitedState)
        return d->geometry;

    Unit u = (unit == UnitNotDefined) ? d->unit : unit;
    return convertUnit(d->geometry, Millimeter, u, d->dpi);
}


QRectF BaseItemInterface::boundingRect(Unit unit, bool withTransformation, bool withBorder) const
{
    QRectF rect = QRectF(QPointF(0,0), QSizeF(d_ptr->geometry.size()));
    if (withBorder) {
        qreal value = 0;
        borderWidth(d_ptr, Millimeter, 0, &value);
        int delta = ceil(value);
        if (delta != 0)
            rect = rect.adjusted( -delta, -delta, delta, delta );
    }
    if (withTransformation)
        rect = QRectF(rect.topLeft(), transform().mapRect(rect).size());
    return convertUnit(rect, Millimeter, unit, d_ptr->dpi);
}


QPolygonF BaseItemInterface::polygon(Unit unit) const
{
    return transform().map(QPolygonF(geometry(unit)));
}


void BaseItemInterface::setGeometry(const QRectF &rect, Unit unit)
{
    Q_D(BaseItemInterface);
    if (d->state == NotInitedState) {
        d->geometry = rect;
        return;
    }

    Unit u = (unit == UnitNotDefined) ? d->unit : unit;
    QRectF newRect = convertUnit(rect, u, Millimeter, d->dpi);

    if (newRect.width() < d->minRectSize.width())
        newRect.setWidth(d->minRectSize.width());
    if (newRect.height() < d->minRectSize.height())
        newRect.setHeight(d->minRectSize.height());

    SAVEIFCHANGED(geometry, newRect)

    update_gui();

    if (d->state == NormalState) {
        emit geometryChanged(geometry());
        emit changed();
        emit boundingRectChanged();
    }
}



qreal BaseItemInterface::height(Unit unit) const
{
    Q_D(const BaseItemInterface);
    if (d->state == NotInitedState)
        return d->geometry.height();

    Unit u = (unit == UnitNotDefined) ? d->unit : unit;
    return convertUnit(d->geometry, Millimeter, u, d->dpi).height();
}


void BaseItemInterface::setHeight(qreal height, Unit unit)
{
    Q_D(BaseItemInterface);

    if (d->state == NotInitedState) {
        d->geometry.setHeight(height);
        return;
    }

    Unit u = (unit == UnitNotDefined) ? d->unit : unit;
    qreal newValue = convertUnit(height, u, Millimeter, d->dpi);

    if (newValue < d->minRectSize.height())
        newValue = d->minRectSize.height();

    if (d->geometry.height() == newValue)
        return;

    d->geometry.setHeight(newValue);

    update_gui();

    if (d->state == NormalState) {
        emit geometryChanged(geometry());
        emit boundingRectChanged();
        emit changed();
    }
}


qreal BaseItemInterface::width(Unit unit) const
{
    Q_D(const BaseItemInterface);
    if (d->state == NotInitedState) {
        return d->geometry.width();
    }

    Unit u = (unit == UnitNotDefined) ? d->unit : unit;
    return convertUnit(d->geometry, Millimeter, u, d->dpi).width();
}


void BaseItemInterface::setWidth(qreal width, Unit unit)
{
    Q_D(BaseItemInterface);

    if (d->state == NotInitedState) {
        d->geometry.setWidth(width);
        return;
    }

    Unit u = (unit == UnitNotDefined) ? d->unit : unit;
    qreal newValue = convertUnit(width, u, Millimeter, d->dpi);

    if (newValue < d->minRectSize.width())
        newValue = d->minRectSize.width();

    if (d->geometry.width() == newValue)
        return;

    d->geometry.setWidth(newValue);

    update_gui();

    if (d->state == NormalState) {
        emit geometryChanged(geometry());
        emit boundingRectChanged();
        emit changed();
    }
}


QSizeF BaseItemInterface::minSize(Unit unit) const
{
    Q_D(const BaseItemInterface);
    if (d->state == NotInitedState)
        return d->minRectSize;

    Unit u = (unit == UnitNotDefined) ? d->unit : unit;
    return convertUnit(d->minRectSize, Millimeter, u, d->dpi);
}


qint8 BaseItemInterface::frame() const
{
    Q_D(const BaseItemInterface);
    return d->frame;
}


void BaseItemInterface::setFrame(Frames frame)
{
    Q_D(BaseItemInterface);
    PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(frame, frame)
}


qreal BaseItemInterface::opacity() const
{
    Q_D(const BaseItemInterface);
    return d->opacity;
}


void  BaseItemInterface::setOpacity(qreal opacity)
{
    Q_D(BaseItemInterface);
    qreal newValue = qBound(0.0, opacity, 1.0);
    PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(opacity, newValue)
}


qreal BaseItemInterface::rotation() const
{
    Q_D(const BaseItemInterface);
    return d->rotation;
}


void BaseItemInterface::setRotation(qreal angle)
{
    Q_D(BaseItemInterface);
    qreal newValue = qBound(0.0, angle, 360.0);
    PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(rotation, newValue);
    if (d->state == NormalState) {
        emit boundingRectChanged();
    }
}


QPen BaseItemInterface::borderPen() const
{
    Q_D(const BaseItemInterface);
    return d->borderPen;
}


void BaseItemInterface::setBorderPen(const QPen & pen)
{
    Q_D(BaseItemInterface);
    PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(borderPen, pen)
}


QBrush BaseItemInterface::backgroundBrush() const
{
    Q_D(const BaseItemInterface);
    return d->backgroundBrush;
}


void BaseItemInterface::setBackgroundBrush(const QBrush & brush)
{
    Q_D(BaseItemInterface);
    PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(backgroundBrush, brush)
}


BaseItemInterface::BorderType BaseItemInterface::borderType() const
{
    Q_D(const BaseItemInterface);
    return d->borderType;
}


void BaseItemInterface::setBorderType(BaseItemInterface::BorderType borderType)
{
    Q_D(BaseItemInterface);
    PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(borderType, borderType);
    if (d->state == NormalState) {
        emit borderTypeChanged(borderTypeToString(d->borderType));
    }
}


QString BaseItemInterface::borderTypeStr() const
{
    return borderTypeToString(borderType());
}


void BaseItemInterface::setBorderTypeStr(const QString &borderType)
{
    setBorderType(borderTypeFromString(borderType));
}


void BaseItemInterface::updateMeassure()
{
    Q_D(BaseItemInterface);
    d->geometry = convertUnit(d->geometry, d->unit, Millimeter, d->dpi);
}


TemplateItemDetailLevel BaseItemInterface::templateDetailLevel()
{
    return m_templateDetailLevel;
}


void BaseItemInterface::setTemplateDetailLevel(const TemplateItemDetailLevel &value)
{
    m_templateDetailLevel = value;
}


TemplateItemDetailLevel BaseItemInterface::defaultTemplateDetailLevel()
{
    return g_templateDetailLevel;
}


void BaseItemInterface::setDefaultTemplateDetailLevel(const TemplateItemDetailLevel &value)
{
    g_templateDetailLevel = value;
}

void BaseItemInterface::aboutToBeDeleted()
{
    Q_D(BaseItemInterface);
    d->state = DeleteState;
}


QTransform BaseItemInterface::transform() const
{
    Q_D(const BaseItemInterface);
    return transform(d);
}


qreal BaseItemInterface::borderWidth(const BaseItemInterfacePrivate *data, Unit unit, qreal *insideWidth, qreal *outsideWidth)
{
    qreal penWidth = convertUnit(data->borderPen.widthF(), Millimeter, unit, data->dpi);

    if (insideWidth || outsideWidth) {
        qreal half = penWidth/2.0;
        if (insideWidth) {
            switch(data->borderType) {
                case Middle: *insideWidth = half; break;
                case Inner: *insideWidth = penWidth; break;
                case Outer: *insideWidth = 0; break;
            }
        }
        if (outsideWidth) {
            switch(data->borderType) {
                case Middle: *outsideWidth = half; break;
                case Inner: *outsideWidth = 0; break;
                case Outer: *outsideWidth = penWidth; break;
            }
        }
    }

    return penWidth;
}


QTransform BaseItemInterface::transform(const BaseItemInterfacePrivate *d)
{
    QTransform trans;
    trans.rotate(-d->rotation);
    return trans;
}


QPointF BaseItemInterface::transformedPos(const BaseItemInterfacePrivate *d, const QRectF &rect)
{
    QTransform trans = transform(d);
    QRectF rotatedRect = trans.mapRect(QRectF(QPointF(0,0), rect.size()));
    QPointF pos = rect.topLeft() - rotatedRect.topLeft();
    return pos;
}


BaseItemInterface::BorderType BaseItemInterface::borderTypeFromString(const QString &bType)
{
    QString t = bType.toLower();
    for (int i=0; i<BOrderTypeNum; ++i) {
        if (t == QString(BorderTypeStr[i]).toLower()) {
            return BorderType(i);
        }
    }
    return Inner;
}


QString BaseItemInterface::borderTypeToString(BaseItemInterface::BorderType bType)
{
    if (bType >= Middle && bType <= Outer)
        return BorderTypeStr[bType];
    return BorderTypeStr[2];
}


StdEditorPropertyList BaseItemInterface::stdEditorList() const
{
    StdEditorPropertyList list;
    list << StdEditorProperty(EDFrame, "frame");
    list << StdEditorProperty(EDFramePen, "borderPen");
    return list;
}


QStringList BaseItemInterface::_borderType_variants()
{
    QStringList list;
    for (int i=0; i<BOrderTypeNum; ++i) {
        list << borderTypeToString(BorderType(i));
    }
    return list;
}


/*********************************************************************************************
 *
 *          ItemInterfacePrivate
 *
 *********************************************************************************************/

QDataStream &operator<<(QDataStream &s, const BaseItemInterfacePrivate &p) {
    s << p.borderPen << p.backgroundBrush << p.geometry << (qint8)p.frame << p.opacity << p.rotation   << p.enabled << p.order
      << p.dpi << (qint8)p.unit << p.selfRendering << p.childrenSelfRendering << p.minRectSize << (qint16) p.resizeFlags
      << (qint8)p.renderingType << (qint8)p.borderType << (qint8)p.state << p.baseItemFlags << p.allowSplit;

    return s;
}


QDataStream &operator>>(QDataStream &s, BaseItemInterfacePrivate &p) {
    qint8 unit;
    qint16 resizeFlags;
    qint8 renderingType;
    qint8 borderType;
    qint8 state;
    qint8 frame;

    s >> p.borderPen; s >> p.backgroundBrush; s >> p.geometry; s >> frame; s >> p.opacity; s >> p.rotation;   s >> p.enabled; s >> p.order;
    s >> p.dpi; s >> unit; s >> p.selfRendering; s >> p.childrenSelfRendering; s >> p.minRectSize; s >>resizeFlags;
    s >> renderingType; s >> borderType; s >> state; s >> p.baseItemFlags; s >> p.allowSplit;

    p.unit = (Unit)unit;
    p.resizeFlags = resizeFlags;
    p.renderingType = (RenderingType)renderingType;
    p.borderType = (BaseItemInterface::BorderType)borderType;
    p.state = (ItemState)state;
    p.frame = (BaseItemInterface::Frames)frame;

    return s;
}


} //namespace
