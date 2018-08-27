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
#include <QtCore>
#include <QBrush>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "pageinterface.h"
#include "rendererpublicinterface.h"
#include "pageheader.h"
#include "pageheaderscripting.h"
#include "item_common/simplerendereditem.h"
#include "scriptengineinterface.h"

using namespace CuteReport;

inline void initMyResource() { Q_INIT_RESOURCE(pageHeader); }

PageHeader::PageHeader(QObject * parent):
    CuteReport::BandInterface(new PageHeaderPrivate, parent)
   ,m_renderer(0)
{
    Q_D(PageHeader);
    d->geometry = QRectF(0,0,50,20);
    setResizeFlags(FixedPos | ResizeBottom);
}


PageHeader::PageHeader(PageHeaderPrivate *dd, QObject * parent)
    :CuteReport::BandInterface(dd, parent)
    ,m_renderer(0)
{
}


PageHeader::~PageHeader()
{
}


bool PageHeader::moduleInit()
{
    initMyResource();
    return true;
}


CuteReport::BaseItemInterface * PageHeader::itemClone() const
{
    Q_D(const PageHeader);
    return new PageHeader(new PageHeaderPrivate(*d), parent());
}


QByteArray PageHeader::serialize()
{
    Q_D(PageHeader);
    QByteArray ba;
    QDataStream s( &ba, QIODevice::ReadWrite );
    s << *d;
    return ba;
}


void PageHeader::deserialize(QByteArray &data)
{
    Q_D(PageHeader);
    QDataStream s( &data, QIODevice::ReadWrite );
    s >> *d;
}


QDataStream &operator<<(QDataStream &s, const PageHeaderPrivate &p) {
    s << static_cast<const BandInterfacePrivate&>(p);
    s << p.onFirstPage << p.once;
    return s;
}


QDataStream &operator>>(QDataStream &s, PageHeaderPrivate &p) {
    s >> static_cast<BandInterfacePrivate&>(p);
    s >> p.onFirstPage; s >> p.once;
    return s;
}


BaseItemInterface * PageHeader::createInstance(QObject * parent) const
{
    return new PageHeader(parent);
}


bool PageHeader::canContain(QObject * object)
{
    return (!qobject_cast<BandInterface*>(object) && qobject_cast<BaseItemInterface*>(object));
}


QIcon PageHeader::itemIcon() const
{
	return QIcon(":/pageHeader.png");
}


QString PageHeader::moduleShortName() const
{
    return tr("Page Header");
}


QString PageHeader::itemGroup() const
{
    return QString("Bands");
}


void PageHeader::renderInit(ScriptEngineInterface *scriptEngine, InitFlags flags)
{
    renderInitBase(scriptEngine, flags);
    m_renderer = scriptEngine->rendererItemInterface();
}


void PageHeader::renderReset()
{
    m_renderer = 0;
    renderResetBase();
}


bool PageHeader::renderBegin()
{
    return false;
}


bool PageHeader::renderNewPage()
{
    rendererBeginBaseStart(new PageHeaderPrivate(*(reinterpret_cast<PageHeaderPrivate*>(d_ptr))));

    Q_D(PageHeader);

    bool result = false;

    if (m_renderer->currentPageNumber() == 1) {
        if (d->onFirstPage)
            result = true;
    } else if (m_renderer->currentPageNumber() == 2){
        if (d->once) {
            if (!d->onFirstPage)
                result = true;
        } else
            result = true;
    } else
        result = !d->once;

    rendererBeginBaseEnd();

    return result && d->enabled;

    bool needProcessing = result && d->enabled;
    if (!needProcessing)
        renderNewPageComplete();

    return needProcessing;
}


RenderedItemInterface *PageHeader::renderView(quint32 id)
{
    Q_D(PageHeader);
    RenderedItemInterface * view = new SimpleRenderedItem(this, new PageHeaderPrivate(*d));
    return view;
}



bool PageHeader::printOnFirstPage() const
{
    Q_D(const PageHeader);
    return d->onFirstPage;
}


void PageHeader::setPrintOnFirstPage(bool b)
{
    Q_D(PageHeader);
    if (b == d->onFirstPage)
        return;

    d->onFirstPage = b;
    emit printOnFirstPageChanged();
    emit changed();
}


bool PageHeader::printOnce() const
{
    Q_D(const PageHeader);
    return d->once;
}


void PageHeader::setPrintOnce(bool b)
{
    Q_D(PageHeader);
    if (b == d->once)
        return;
    d->once = b;
    emit printOnceChanged();
    emit changed();
}


QPair<QString, int> PageHeader::sortingOrder() const
{
    return QPair<QString, int>("page", 300);
}


QString PageHeader::_current_property_description() const
{
    QString propertyName = metaObject()->property(m_currentProperty).name();

    if (propertyName == "firstPage")
        return tr("If True, band is drawn on first page");
    else if (propertyName == "once")
        return tr("If True, band appears only once");
    else
        return BandInterface::_current_property_description();
}

//}//namespace

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(PageHeader, PageHeader)
#endif
