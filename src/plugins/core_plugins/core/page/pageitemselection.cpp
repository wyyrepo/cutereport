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
#include "pageitemselection.h"
#include "page.h"
#include "pagegui.h"
#include "iteminterfaceview.h"
#include "bandinterface.h"
#include "reportcore.h"
#include "cutereport_functions.h"
#include "magnets.h"

#include <QtGui>
#include <QGraphicsSceneMouseEvent>

#define BOUND 0
#define SIZE 8

//#define DEBUG

using namespace CuteReport;


SUIT_BEGIN_NAMESPACE


ItemHandle::ItemHandle(Type t, ItemSelection *s) :
    QGraphicsItem (s),
    m_itemView(0),
    m_type(t),
    m_sel(s),
    m_active(true)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    updateCursor();
}


void ItemHandle::updateCursor()
{
    if (!m_active) {
        setCursor(Qt::ArrowCursor);
        return;
    }

    switch (m_type) {
        case LeftTop:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case Top:
            setCursor(Qt::SizeVerCursor);
            break;
        case RightTop:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case Right:
            setCursor(Qt::SizeHorCursor);
            break;
        case RightBottom:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case Bottom:
            setCursor(Qt::SizeVerCursor);
            break;
        case LeftBottom:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case Left:
            setCursor(Qt::SizeHorCursor);
            break;
        default:
            Q_ASSERT(0);
    }
}


void ItemHandle::setActive(bool a)
{
    m_active = a;

    if (a)
        setFlags(/*QGraphicsItem::ItemIsMovable |*/ QGraphicsItem::ItemIsSelectable);
    else
        setFlags(0);

    updateCursor();
}


void ItemHandle::setItem(CuteReport::ItemInterfaceView *i)
{
    m_itemView = i;
}


void ItemHandle::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    //    painter->setPen( Qt::black);
    //    painter->drawRect(0, 0, SIZE - 1, SIZE - 1);
    if (m_active)
        painter->fillRect( 0, 0, SIZE - 1, SIZE - 1, QBrush(QColor("#096BFF")) );
    else
        painter->fillRect( 0, 0, SIZE - 1, SIZE - 1, QBrush(Qt::black) );
}


QRectF ItemHandle::boundingRect () const
{
    return QRectF(0,0,SIZE,SIZE);
}


void ItemHandle::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
//    qDebug() << "ItemHandle::mousePressEvent";

    if (!(m_itemView && e->button() == Qt::LeftButton))
        return;

    if (!(m_active))
        return;

    e->accept();

    m_origPressPos = e->scenePos();
    //m_geom = m_origGeom = QRectF (m_itemView->pos(), m_itemView->boundingRect().size());
    m_origGeom = m_itemView->coreItem()->absoluteGeometry(Millimeter);
    m_geom = m_origGeom;

    // do not use magnets for bands
    //m_useMagnets = !qobject_cast<CuteReport::BandInterface*>(m_itemView->coreItem());

    m_checkingItems.clear();
    foreach (BaseItemInterface * item, m_sel->pageGUI()->page()->items()) {
        if (item == m_itemView->coreItem())
            continue;
        if (item->parentItem() == m_itemView->coreItem()->parentItem() || m_itemView->coreItem()->parentItem() == item)
            m_checkingItems.insert(0,  item);
        else
            m_checkingItems.append(item);
    }
}


void ItemHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    if (!(m_itemView && m_active && e->buttons() & Qt::LeftButton))
        return;

    e->accept();

    const QPointF deltaPix = e->scenePos() - m_origPressPos;
    const QPointF deltaMM = convertUnit(deltaPix, Pixel, Millimeter, m_itemView->coreItem()->dpi());

    m_geom = m_origGeom;

    switch (m_type) {

        case LeftTop: {
                QPointF point = m_sel->alignedPoint(deltaMM, m_origGeom.topLeft(), m_checkingItems);
                m_geom.setTopLeft(point);
                m_itemView->coreItem()->setAbsoluteGeometry(m_geom, Millimeter);
            } break;

        case Top: {
                QPointF point = m_sel->alignedPoint(QPointF(0, deltaMM.y()), m_origGeom.topLeft(), m_checkingItems);
                m_geom.setTop(point.y());
                m_itemView->coreItem()->setAbsoluteGeometry(m_geom, Millimeter);
            } break;

        case RightTop: {
                QPointF point = m_sel->alignedPoint(deltaMM, m_origGeom.topRight(), m_checkingItems);
                m_geom.setTopRight(point);
                m_itemView->coreItem()->setAbsoluteGeometry(m_geom, Millimeter);
            } break;

        case Right: {
                QPointF point = m_sel->alignedPoint(QPointF(deltaMM.x(), 0), m_origGeom.topRight(), m_checkingItems);
                m_geom.setRight(point.x());
                m_itemView->coreItem()->setAbsoluteGeometry(m_geom, Millimeter);
            } break;

        case RightBottom: {
                QPointF point = m_sel->alignedPoint(deltaMM, m_origGeom.bottomRight(), m_checkingItems);
                m_geom.setBottomRight(point);
                m_itemView->coreItem()->setAbsoluteGeometry(m_geom, Millimeter);
            } break;

        case Bottom: {
                QPointF point = m_sel->alignedPoint(QPointF(0, deltaMM.y()), m_origGeom.bottomLeft(), m_checkingItems);
                m_geom.setBottom(point.y());
                m_itemView->coreItem()->setAbsoluteGeometry(m_geom, Millimeter);
            } break;

        case LeftBottom: {
                QPointF point = m_sel->alignedPoint(deltaMM, m_origGeom.bottomLeft(), m_checkingItems);
                m_geom.setBottomLeft(point);
                m_itemView->coreItem()->setAbsoluteGeometry(m_geom, Millimeter);
            } break;

        case Left: {
                QPointF point = m_sel->alignedPoint(QPointF(deltaMM.x(), 0), m_origGeom.bottomLeft(), m_checkingItems);
                m_geom.setLeft(point.x());
                m_itemView->coreItem()->setAbsoluteGeometry(m_geom, Millimeter);
            } break;

        default: break;

    } // end switch

}


void ItemHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
//    qDebug() << "ItemHandle::mouseReleaseEvent";
    e->accept();
    //m_sel->pageGUI()->setIgnoreObjectSelection(true);
    if (e->button() != Qt::LeftButton || !m_active)
        return;
}



// ------------------ ItemSelection

ItemSelection::ItemSelection(PageGUI * pageGUI, BaseItemInterface *item)
    :m_item(item),
      m_pageGui(pageGUI)

{
    //    setAcceptedMouseButtons(0);
    for (int i = ItemHandle::LeftTop; i < ItemHandle::TypeCount; ++i)
        m_handles[i] = new ItemHandle(static_cast<ItemHandle::Type>(i), this);

    setFlags(QGraphicsItem::ItemHasNoContents /*| QGraphicsItem::ItemIsFocusable*/ | QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemClipsToShape, false);

    updateActive();
    updateGeometry();

    setParent(item);
    setZValue(100);
    setParentItem(m_pageGui->pageItem());

    connect(item, SIGNAL(boundingRectChanged()), this, SLOT(updateGeometry()));
}


ItemSelection::~ItemSelection()
{
}


void ItemSelection::updateActive()
{
    int flag = m_item->resizeFlags();
    bool locked = m_item->isBaseItemFlagSet(BaseItemInterface::LockedItem);

    bool active[ItemHandle::TypeCount];
    qFill(active, active + ItemHandle::TypeCount, false);

    //    qDebug("fixed = %i", flag&CuteReport::ItemInterface::Fixed);
    //    qDebug("top = %i", flag&CuteReport::ItemInterface::ResizeBottom);
    //    qDebug("bottom = %i", flag&CuteReport::ItemInterface::ResizeTop);

    active[ItemHandle::Left] = !locked && flag&CuteReport::BaseItemInterface::ResizeLeft;
    active[ItemHandle::Right] = !locked && flag&CuteReport::BaseItemInterface::ResizeRight;
    active[ItemHandle::Top] = !locked && flag&CuteReport::BaseItemInterface::ResizeTop;
    active[ItemHandle::Bottom] = !locked && flag&CuteReport::BaseItemInterface::ResizeBottom;

    active[ItemHandle::LeftTop] = !locked && flag&CuteReport::BaseItemInterface::ResizeTop && flag&CuteReport::BaseItemInterface::ResizeLeft;
    active[ItemHandle::RightTop] = !locked && flag&CuteReport::BaseItemInterface::ResizeTop && flag&CuteReport::BaseItemInterface::ResizeRight;
    active[ItemHandle::RightBottom] = !locked && flag&CuteReport::BaseItemInterface::ResizeBottom && flag&CuteReport::BaseItemInterface::ResizeRight;
    active[ItemHandle::LeftBottom] = !locked && flag&CuteReport::BaseItemInterface::ResizeBottom && flag&CuteReport::BaseItemInterface::ResizeLeft;

    /*
    qFill(active, active + ItemHandle::TypeCount, false);
    // Determine active handles
    switch (is) {
    case UnlaidOut:
    qFill(active, active + ItemHandle::TypeCount, true);
        break;
    case ManagedGridLayout: // Grid: Allow changing span
    active[ItemHandle::Left] = active[ItemHandle::Top] = active[ItemHandle::Right] = active[ItemHandle::Bottom] = true;
        break;
    case ManagedFormLayout:  // Form: Allow changing column span
        if (const unsigned operation = ChangeFormLayoutItemRoleCommand::possibleOperations(m_formWindow->core(), m_widget)) {
        active[ItemHandle::Left]  = operation & (ChangeFormLayoutItemRoleCommand::SpanningToField|ChangeFormLayoutItemRoleCommand::FieldToSpanning);
        active[ItemHandle::Right] = operation & (ChangeFormLayoutItemRoleCommand::SpanningToLabel|ChangeFormLayoutItemRoleCommand::LabelToSpanning);
        }
        break;
    default:
        break;
    }
*/
    for (int i = ItemHandle::LeftTop; i < ItemHandle::TypeCount; ++i)
        if (ItemHandle *h = m_handles[i]) {
            h->setItem(m_item->view());
            h->setActive(active[i]);
        }
}


void ItemSelection::updateGeometry()
{
    if (!m_item->view())
        return;

    prepareGeometryChange();

    m_boundingRect = m_item->absoluteBoundingRect(Pixel);
    setPos(QPointF ( m_boundingRect.left() - BOUND, m_boundingRect.top() - BOUND ) );
    const QRectF r(QPointF(BOUND,BOUND), m_boundingRect.size() + QSize(1,1));

    m_boundingRect = QRectF(0 ,0 , m_boundingRect.width() + 2*BOUND, m_boundingRect.height() + 2*BOUND);

    update();

    const int w = SIZE;
    const int h = SIZE;

    for (int i = ItemHandle::LeftTop; i < ItemHandle::TypeCount; ++i) {
        ItemHandle *hndl = m_handles[ i ];
        if (!hndl)
            continue;
        switch (i) {
            case ItemHandle::LeftTop:
                hndl->setPos(r.x() - w / 2, r.y() - h / 2);
                break;
            case ItemHandle::Top:
                hndl->setPos(r.x() + r.width() / 2 - w / 2, r.y() - h / 2);
                break;
            case ItemHandle::RightTop:
                hndl->setPos(r.x() + r.width() - w / 2, r.y() - h / 2);
                break;
            case ItemHandle::Right:
                hndl->setPos(r.x() + r.width() - w / 2, r.y() + r.height() / 2 - h / 2);
                break;
            case ItemHandle::RightBottom:
                hndl->setPos(r.x() + r.width() - w / 2, r.y() + r.height() - h / 2);
                break;
            case ItemHandle::Bottom:
                hndl->setPos(r.x() + r.width() / 2 - w / 2, r.y() + r.height() - h / 2);
                break;
            case ItemHandle::LeftBottom:
                hndl->setPos(r.x() - w / 2, r.y() + r.height() - h / 2);
                break;
            case ItemHandle::Left:
                hndl->setPos(r.x() - w / 2, r.y() + r.height() / 2 - h / 2);
                break;
            default:
                break;
        }
    }
}

QRectF ItemSelection::boundingRect () const
{
    return m_boundingRect;
}


void ItemSelection::update()
{
    for (int i = ItemHandle::LeftTop; i < ItemHandle::TypeCount; ++i) {
        ItemHandle *h = m_handles[ i ];
        if (h)
            h->update();
    }
}


CuteReport::ItemInterfaceView *ItemSelection::item() const
{
    return m_item->view();
}


void ItemSelection::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

#ifndef DEBUG
    Q_UNUSED(painter)

    //    painter->save();

    //    painter->translate( boundingRect().center());
    //    painter->rotate( m_itemView->rotate() );
    //    painter->translate( -m_itemView->boundingRect().center());

    //    m_itemView->paint(painter, option, widget);
    //    foreach (QGraphicsItem * item, m_itemView->childItems())
    //    {
    //	painter->save();
    ////	painter->translate( item->boundingRect().center());
    ////	painter->rotate( item->rotate() );
    ////	painter->translate( -item->boundingRect().center());
    //	item->paint(painter, option, widget);
    //	painter->restore();
    //    }
    //    painter->restore();

#else
    painter->save();
    painter->setOpacity( 0.3 );
    painter->fillRect( this->boundingRect(), QBrush(Qt::red));
    painter->restore();
#endif
}


void ItemSelection::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
//    qDebug() << "ItemSelection::mousePressEvent";

    //    if  (e->button() != Qt::LeftButton)
    //        return;

    e->accept();

    QPointF pagePos =  convertUnit(e->scenePos(), Pixel, m_pageGui->page()->unit(), m_pageGui->page()->dpi()) -
                       convertUnit(QPointF(PAGE_BORDER, PAGE_BORDER), CuteReport::Millimeter, m_pageGui->page()->unit(), m_pageGui->page()->dpi());
    BaseItemInterface * item = m_pageGui->page()->itemAt(pagePos);

    if (item != m_item)
        e->ignore();

    m_origPressPos = e->scenePos();
    m_origGeom = m_item->absoluteGeometry(Millimeter);
    m_geom = m_origGeom;
    m_moved = false;

    QList<CuteReport::BaseItemInterface*> children = m_item->findChildren<CuteReport::BaseItemInterface*>();
    QList<CuteReport::BaseItemInterface*> allItems = m_pageGui->page()->items();
    m_checkingItems.clear();

    foreach (BaseItemInterface * checkItem, allItems) {
        if (checkItem == m_item || children.contains(checkItem))
            continue;
        if (checkItem->parentItem() == m_item->parentItem() || checkItem->parentItem() == m_item)
            m_checkingItems.insert(0,  checkItem);
        else
            m_checkingItems.append(checkItem);
    }

}


void ItemSelection::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    if (m_item->resizeFlags() & BaseItemInterface::FixedPos || m_item->isBaseItemFlagSet(BaseItemInterface::LockedItem))
        return;

    e->accept();

    m_moved = true;
    const QPointF deltaPix = e->scenePos() - m_origPressPos;
    const QPointF deltaMM = convertUnit(deltaPix, Pixel, Millimeter, m_item->dpi());
    m_geom = m_origGeom;

    m_geom.translate(deltaMM);
    QList<QPointF> pointsList;
    pointsList << m_geom.bottomLeft() << m_geom.bottomRight() << m_geom.topLeft() << m_geom.topRight();

    QPointF point = alignedPoint(deltaMM, m_origGeom.topLeft(), m_checkingItems, pointsList);
    m_geom.moveTopLeft(point);
    m_item->setAbsoluteGeometry(m_geom, Millimeter);
}


void ItemSelection::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    Q_UNUSED(e)
    if (m_moved) {
        if (e->button() == Qt::RightButton) {
            m_checkingItems.clear();
            QPointF controlPoint = convertUnit(e->scenePos(), Pixel, m_pageGui->page()->unit(), m_pageGui->page()->dpi()) -
                                   convertUnit(QPointF(PAGE_BORDER, PAGE_BORDER), Millimeter, m_pageGui->page()->unit(), m_pageGui->page()->dpi());

            BaseItemInterface * parentItem = 0;

            QList<BaseItemInterface *> items = m_item->page()->itemsAt(controlPoint);
            for (int i=items.size()-1; i>=0; --i) {
                if (items[i] != m_item && items[i]->canContain(m_item)) {
                    parentItem = items[i];
                    break;
                }
            }


            if (parentItem  && !(m_item->findChildren<CuteReport::BaseItemInterface*>().contains(parentItem))) {
                QRectF absoluteGeometry = m_item->absoluteGeometry();
                m_item->setParentItem(parentItem);
                m_item->setAbsoluteGeometry(absoluteGeometry);
            }
            //m_pageGui->setIgnoreObjectSelection(true);
        }
    }  else
        emit itemClicked(m_item, e);

    e->ignore();
    //QGraphicsObject::mouseReleaseEvent(e);
}


void ItemSelection::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
    e->ignore();
}


QPointF ItemSelection::alignedPoint(const QPointF delta, const QPointF origPoint, const QList<CuteReport::BaseItemInterface*> & checkingItems, QList<QPointF> pointsList)
{
    QPointF pointResult = origPoint + delta;

    bool needCorrection = true;
    Page * page = reinterpret_cast<Page*>(m_item->page());
    // do not use magnets for bands
    bool useMagnets = !qobject_cast<CuteReport::BandInterface*>(m_item);

    if (useMagnets) {
        if (pointsList.isEmpty())
            pointsList << pointResult;
        Magnets::MagnetDirection direction;
        if (delta.x() != 0 && delta.y() != 0)
            direction = Magnets::HVDirection;
        else if (delta.x() != 0)
            direction = Magnets::HDirection;
        else
            direction = Magnets::VDirection;

        QPointF magnetDelta = m_pageGui->magnets()->delta(pointsList, checkingItems, direction);

        if (!magnetDelta.isNull()) {
            needCorrection = false;
            pointResult += magnetDelta;
        }
    }

    if (needCorrection && page->useGrid()) {
        qreal pageStepMM =  convertUnit(page->gridStep(), page->unit(), Millimeter, page->dpi());
        qreal corrX = (delta.x() >= 0.0) ? 0 : pageStepMM/2;
        qreal corrY = (delta.y() >= 0.0) ? 0 : pageStepMM/2;
        qreal x = int((pointResult.x() + corrX) / pageStepMM) * pageStepMM;
        qreal y = int((pointResult.y() + corrY) / pageStepMM) * pageStepMM;
        pointResult = QPointF(x,y);
    }

    if ((delta.y() > 0 && pointResult.y() < origPoint.y()) || (delta.y() < 0 && pointResult.y() > origPoint.y()))
        pointResult.setY(origPoint.y());
    if ((delta.x() > 0 && pointResult.x() < origPoint.x()) || (delta.x() < 0 && pointResult.x() > origPoint.x()))
        pointResult.setX(origPoint.x());

    return pointResult;
}


SUIT_END_NAMESPACE
