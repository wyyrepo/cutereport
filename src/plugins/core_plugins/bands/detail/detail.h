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

#ifndef DETAIL_H
#define DETAIL_H

#include <bandinterface.h>
#include "bandinterface_p.h"
#include "cutereport_globals.h"

namespace CuteReport
{
class ReportPaintEngineInterface;
class RendererPublicInterface;
}


class DetailPrivate;

class Detail : public CuteReport::BandInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "CuteReport.BandInterface/1.0")
#endif
    Q_INTERFACES(CuteReport::BandInterface)

    Q_PROPERTY(QString dataset READ dataset WRITE setDataset NOTIFY datasetChanged)
    Q_PROPERTY(bool zebra READ zebra WRITE setZebra NOTIFY zebraChanged)
    Q_PROPERTY(QBrush alternateBrush READ alternateBrush WRITE setAlternateBrush NOTIFY alternateBrushChanged)
    Q_PROPERTY(bool forceNewPage READ forceNewPage WRITE setForceNewPage NOTIFY forceNewPageChanged)
    Q_PROPERTY(bool allowSplit READ allowSplit WRITE setAllowSplit NOTIFY allowSplitChanged)

public:
    Detail(QObject * parent = 0);
    ~Detail();

    virtual bool moduleInit();

    virtual CuteReport::BaseItemInterface *createInstance(QObject * parent) const;
    virtual QByteArray serialize();
    virtual void deserialize(QByteArray &data);
    virtual bool canContain(QObject * object);

    virtual void renderInit(CuteReport::ScriptEngineInterface *scriptEngine, CuteReport::InitFlags flags);
    virtual bool renderBegin();
    virtual void renderReset();
    virtual CuteReport::RenderedItemInterface * renderView(quint32 id);

    virtual QIcon itemIcon() const;
    virtual QString moduleShortName() const;
    virtual QString suitName() const { return "Standard"; }
    virtual QString itemGroup() const;

    int layoutPriority() const { return 50;}
    virtual bool respectPageColumns() {return true;}

    QString dataset() const;
    void setDataset(const QString & dataset);

    bool zebra() const;			    //light and dark rows
    void setZebra(bool b);

    QBrush alternateBrush() const;
    void setAlternateBrush(const QBrush & brush);

    bool forceNewPage() const;
    void setForceNewPage(bool b);

    bool allowSplit() const;
    void setAllowSplit(bool value);
    virtual bool isSplitAllowed() const;

    virtual QPair<QString, int> sortingOrder() const; // first - sorting group, second - sorting order

    virtual QString _current_property_description() const;

signals:
    void groupChanged(QString);
    void datasetChanged(QString);
    void zebraChanged(bool);
    void alternateBrushChanged(QBrush);
    void forceNewPageChanged(bool);
    void allowSplitChanged(bool);

private:
    Q_DECLARE_PRIVATE(Detail)

    Detail(DetailPrivate *dd, QObject * parent);
    virtual BaseItemInterface * itemClone() const;

    CuteReport::RendererPublicInterface * m_renderer;
};


class DetailPrivate : public CuteReport::BandInterfacePrivate
{
public:
    DetailPrivate():
        zebra(false)
      ,alternateRow(false)
      ,forceNewPage(false)
      ,allowSplit(false)
    {}
    virtual ~DetailPrivate(){}
    QString dataset;
    bool zebra;
    QBrush alternateBrush;
    bool alternateRow;
    bool forceNewPage;
    bool allowSplit;
};

QDataStream &operator<<(QDataStream &s, const DetailPrivate &p);
QDataStream &operator>>(QDataStream &s, DetailPrivate &p);

#endif

