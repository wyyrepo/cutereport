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
#include <QtGui>
#include <QtScript>
#include "pageinterface.h"
#include "rendererpublicinterface.h"
#include "datasetinterface.h"

#include "detailfooter.h"
#include "detailfooter_p.h"
#include "detailfooterscripting.h"
#include "item_common/simplerendereditem.h"
#include "scriptengineinterface.h"

using namespace CuteReport;

inline void initMyResource() { Q_INIT_RESOURCE(detailFooter); }

DetailFooter::DetailFooter(QObject * parent):
    CuteReport::BandInterface(new DetailFooterPrivate, parent)
  ,m_renderer(0)
{
    Q_D(DetailFooter);
    d->geometry = QRectF(0,0,50,20);
    setResizeFlags(FixedPos | ResizeBottom);
}


DetailFooter::DetailFooter(DetailFooterPrivate *dd, QObject * parent)
    :CuteReport::BandInterface(dd, parent)
    ,m_renderer(0)
{
}


DetailFooter::~DetailFooter()
{
}


bool DetailFooter::moduleInit()
{
    initMyResource();
    return true;
}


BaseItemInterface *DetailFooter::itemClone() const
{
    Q_D(const DetailFooter);
    return new DetailFooter(new DetailFooterPrivate(*d), parent());
}


QByteArray DetailFooter::serialize()
{
    Q_D(DetailFooter);
    QByteArray ba;
    QDataStream s( &ba, QIODevice::ReadWrite );
    s << *d;
    return ba;
}


void DetailFooter::deserialize(QByteArray &data)
{
    Q_D(DetailFooter);
    QDataStream s( &data, QIODevice::ReadWrite );
    s >> *d;
}


QDataStream &operator<<(QDataStream &s, const DetailFooterPrivate &p) {
    s << static_cast<const BandInterfacePrivate&>(p);
    s << p.dataset << p.condition << p.lastConditionResult ;

    return s;
}


QDataStream &operator>>(QDataStream &s, DetailFooterPrivate &p) {
    s >> static_cast<BandInterfacePrivate&>(p);
    s >> p.dataset; s >> p.condition; s>> p.lastConditionResult;
    return s;
}


BaseItemInterface *DetailFooter::createInstance(QObject * parent) const
{
    return new DetailFooter(parent);
}


bool DetailFooter::canContain(QObject * object)
{
    return (!qobject_cast<BandInterface*>(object) && qobject_cast<BaseItemInterface*>(object));
}


QIcon DetailFooter::itemIcon() const
{
    return QIcon(":/detailFooter.png");
}


QString DetailFooter::moduleShortName() const
{
    return tr("Detail Footer");
}


QString DetailFooter::itemGroup() const
{
    return QString("Bands");
}


QString DetailFooter::dataset() const
{
    Q_D(const DetailFooter);
    return d->dataset;
}


void DetailFooter::setDataset(const QString & dataset)
{
    Q_D(DetailFooter);
    if (d->dataset == dataset)
        return;

    d->dataset = dataset;
    emit datasetChanged(d->dataset);
    emit changed();
}


QString DetailFooter::condition() const
{
    Q_D(const DetailFooter);
    return d->condition;
}


void DetailFooter::setCondition(const QString & condition)
{
    Q_D(DetailFooter);
    if (d->condition == condition)
        return;

    d->condition = condition;
    emit conditionChanged(d->condition);
    emit changed();
}


void DetailFooter::renderInit(ScriptEngineInterface *scriptEngine, InitFlags flags)
{
    renderInitBase(scriptEngine, flags);
    Q_D(DetailFooter);

    m_renderer = scriptEngine->rendererItemInterface();

    if (flags.testFlag(CuteReport::InitData)) {
        d->lastConditionResult = QString();
        if (!d->dataset.isEmpty()) {
            m_renderer->registerBandToDatasetIteration(d->dataset, this);
        } else {
            m_renderer->error(this->objectName(), "\'dataset\' field is empty");
        }
    }
}


void DetailFooter::renderReset()
{
    Q_D(DetailFooter);
    m_renderer = 0;
    d->lastConditionResult = QString();
    renderResetBase();
}


bool DetailFooter::renderBegin()
{
    rendererBeginBaseStart(new DetailFooterPrivate(*(reinterpret_cast<DetailFooterPrivate*>(d_ptr))));
    Q_D(DetailFooter);

    DetailFooterPrivate * d_orig = reinterpret_cast<DetailFooterPrivate*>(orig_ptr);
    bool needRendering = false;

    if (!d->dataset.isEmpty()) {
        if (!d->condition.isEmpty()) {
            QString currentCondition = d->lastConditionResult.isEmpty() ? m_renderer->processString(CuteReport::ScriptString(reportObject(), this, d->condition)) : d->lastConditionResult;
            CuteReport::DatasetInterface * dataset = m_renderer->dataset(d->dataset);
            bool hasNextRow = false;
            if (dataset)
                hasNextRow = dataset->setNextRow();     // looking ahead
            if (hasNextRow) {
                QString nextCondition = m_renderer->processString(CuteReport::ScriptString(reportObject(), this, d->condition));
                if (dataset)
                    dataset->setPreviousRow(); // setting row back
                if (currentCondition != nextCondition) {
                    needRendering = true;
                    d_orig->lastConditionResult = "";
                } else
                    d_orig->lastConditionResult = nextCondition;
            } else {
                dataset->setPreviousRow();
                needRendering = true;
            }
        } else { // print after last row only
            // TODO:
//            m_renderer->currentPageNumber()
//            needRendering = true;
        }
    }

    if (needRendering)
        m_renderer->resetAggregateFunctions(this);

    rendererBeginBaseEnd();

    bool needProcessing = needRendering && d->enabled;
    if (!needProcessing)
        renderEnd();

    return needProcessing;
}


void DetailFooter::renderPassEnd()
{
    Q_D(DetailFooter);
    d->lastConditionResult = QString();
}


RenderedItemInterface *DetailFooter::renderView(quint32 id)
{
    Q_D(DetailFooter);
    CuteReport::RenderedItemInterface * view = new SimpleRenderedItem(this, new DetailFooterPrivate(*d));
    return view;
}


QPair<QString, int> DetailFooter::sortingOrder() const
{
    return QPair<QString, int>("detail", 700);
}


QString DetailFooter::_current_property_description() const
{
    QString propertyName = metaObject()->property(m_currentProperty).name();

    if (propertyName == "dataset")
        return tr("Name of dataset which band is joined to");
    else if (propertyName == "condition")
        return tr("Band is drawn if condition has changed");
    else
        return BandInterface::_current_property_description();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(DetailFooter, DetailFooter)
#endif
