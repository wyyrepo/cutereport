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
#ifndef RENDEREDITEMINTERFACE_H
#define RENDEREDITEMINTERFACE_H

#include <QGraphicsRectItem>
#include "cutereport_globals.h"
#include "cutereport_types.h"

//static int RenderedItemInterfaceType = QGraphicsItem::UserType + 17778;

namespace CuteReport {

class BaseItemInterface;
class BaseItemInterfacePrivate;
class RenderedPageInterface;

class CUTEREPORT_EXPORTS RenderedItemInterface : public QGraphicsRectItem
{

public:
    enum { Type = QGraphicsItem::UserType + 17778 };
    explicit RenderedItemInterface(BaseItemInterface * item, CuteReport::BaseItemInterfacePrivate *itemPrivateData);
    virtual ~RenderedItemInterface();

    //BaseItemInterface * coreItem(){return m_item;}

    quint32 id() const;
    void setId(quint32 id);

    //void setParentRenderedItem(RenderedItemInterface * parentItem);

    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    virtual void redraw(bool withChildren = true);

    virtual QRectF absoluteGeometry(Unit unit = UnitNotDefined) const;
    virtual void setAbsoluteGeometry(const QRectF & geometry, CuteReport::Unit unit = CuteReport::UnitNotDefined);

    virtual QPointF absolutePixelPos() const;
    virtual QRectF absoluteBoundingRect(Unit unit = UnitNotDefined) const;

    virtual int dpi();
    virtual void setDpi(int dpi, bool withChildren = true);

    virtual QRectF boundingRect() const;

    virtual bool setEditMode(QWidget * topWidget) = 0;
    virtual bool isEditMode() {return false;}
    virtual bool hasEditMode() {return false;}
    virtual QPolygonF editShape() {return rect();}

    int type() const { return Type; }

    /// required height to fit first piece in MM
    virtual bool canSplit(qreal heightMM) {return false;}
    /// returns second piece if can be splitted, or 0 if no
    virtual CuteReport::RenderedItemInterface * split(qreal heightMM, quint32 id) {return 0;}

protected:
    BaseItemInterfacePrivate * const d_ptr;
    virtual void paintBegin(QPainter * painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    virtual void paintEnd(QPainter * painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
    Q_DECLARE_PRIVATE(BaseItemInterface)

    quint32 m_id;
//    BaseItemInterface * m_item;
};

}

Q_DECLARE_INTERFACE(CuteReport::RenderedItemInterface, "CuteReport.RenderedItemInterface/1.0")
Q_DECLARE_METATYPE(CuteReport::RenderedItemInterface*)

#endif // RENDEREDITEMINTERFACE_H
