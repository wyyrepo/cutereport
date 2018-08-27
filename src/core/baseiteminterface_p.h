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
#ifndef BASEITEMINTERFACE_P_H
#define BASEITEMINTERFACE_P_H
#include <QtCore>
#include <QtGui>
#include "baseiteminterface.h"
#include "cutereport_globals.h"

namespace CuteReport
{

class CUTEREPORT_EXPORTS  PrivateDynamicData
{
public:
   explicit PrivateDynamicData() {}
   virtual ~PrivateDynamicData() {}
};

class CUTEREPORT_EXPORTS  PrivateRenderingData: public PrivateDynamicData
{
public:
    explicit PrivateRenderingData(){}
    explicit PrivateRenderingData(const PrivateRenderingData & p) {Q_UNUSED(p);}

//    virtual qint8 type() const = 0;

    virtual ~PrivateRenderingData(){}
};


class CUTEREPORT_EXPORTS  PrivateTemplateData: public PrivateDynamicData
{
public:
    explicit PrivateTemplateData(){}
    explicit PrivateTemplateData(const PrivateTemplateData & p) {Q_UNUSED(p);}
    virtual ~PrivateTemplateData(){}

//    virtual qint8 type() const = 0;

};


class CUTEREPORT_EXPORTS BaseItemInterfacePrivate
{
public:
    BaseItemInterfacePrivate():
        opacity(1),
        rotation(0),
        enabled(true),
        order(0),
        dpi(92),
        unit(Millimeter),
        selfRendering(false),
        childrenSelfRendering(false),
        minRectSize(0,0),
        borderType(BaseItemInterface::Middle),
        state(NormalState),
        baseItemFlags(0),
        allowSplit(false),
        frame(BaseItemInterface::DrawLeft | BaseItemInterface::DrawRight | BaseItemInterface::DrawTop | BaseItemInterface::DrawBottom),
        renderingType(RenderingTemplate),
        r(0),
        lockImage(0)
    {
        borderPen.setWidth(0.3); // always in mm
        borderPen.setJoinStyle(Qt::MiterJoin);
    }

    BaseItemInterfacePrivate(const BaseItemInterfacePrivate & p):
        borderPen(p.borderPen),
        backgroundBrush(p.backgroundBrush),
        geometry(p.geometry),
        frame(p.frame),
        opacity(p.opacity),
        rotation(p.rotation),
        enabled(p.enabled),
        order(p.order),
        dpi(p.dpi),
        unit(p.unit),
        selfRendering(p.selfRendering),
        childrenSelfRendering(p.childrenSelfRendering),
        minRectSize(p.minRectSize),
        resizeFlags(p.resizeFlags),
        borderType(p.borderType),
        state(p.state),
        baseItemFlags(p.baseItemFlags),
        allowSplit(p.allowSplit),
        renderingType(p.renderingType),
        r(0),
        lockImage(0)
    {}

    virtual ~BaseItemInterfacePrivate() {
        delete r;
        delete lockImage;
    }

    virtual void initRenderingData() { }


    QPen borderPen;
    QBrush backgroundBrush;
    QRectF geometry; // in mm
    BaseItemInterface::Frames frame;
    qreal opacity;
    qreal rotation;
    bool enabled;
    qint16 order;
    qint16 dpi;
    Unit unit;
    bool selfRendering;
    bool childrenSelfRendering;
    QSizeF minRectSize; // mm
    int resizeFlags;
    BaseItemInterface::BorderType borderType;
    ItemState state;
    qint32 baseItemFlags;
    bool allowSplit;
    RenderingType renderingType;
    PrivateDynamicData * r;

    /// rendering data
    QPixmap * lockImage; // TODO v2 move to a separated block that will be used only for template
};


CUTEREPORT_EXPORTS QDataStream &operator<<(QDataStream &s, const BaseItemInterfacePrivate &p);
CUTEREPORT_EXPORTS QDataStream &operator>>(QDataStream &s, BaseItemInterfacePrivate &p);

}


#endif // BASEITEMINTERFACE_P_H
