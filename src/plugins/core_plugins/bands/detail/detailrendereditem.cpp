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
#include "detailrendereditem.h"
#include "pageinterface.h"
#include "cutereport_functions.h"

#include <QPainter>

using namespace CuteReport;

DetailRenderedItem::DetailRenderedItem(Detail *item, CuteReport::BaseItemInterfacePrivate *itemPrivateData)
    :RenderedItemInterface(item, itemPrivateData)
    ,m_item(item)
{
    QRectF pixelRect = convertUnit(itemPrivateData->geometry, itemPrivateData->unit, Pixel, itemPrivateData->dpi);
    QRectF rect = QRectF(0,0, pixelRect.width(), pixelRect.height());
    setRect(rect);
    setBrush(itemPrivateData->backgroundBrush);
    setZValue(itemPrivateData->order);
}


RenderedItemInterface *DetailRenderedItem::split(qreal heightMM, quint32 id)
{
    DetailPrivate * d = reinterpret_cast<DetailPrivate*>(d_ptr);
    DetailPrivate * new_d = new DetailPrivate(*d);

    new_d->geometry.setHeight(qMax(0.0, d->geometry.height() - heightMM));
    d->geometry.setHeight(heightMM);

    DetailRenderedItem * newView = new DetailRenderedItem(0, new_d);

    return newView;
}

