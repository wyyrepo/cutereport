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

#ifndef RENDERERITEMINTERFACE_H
#define RENDERERITEMINTERFACE_H

#include "rendererpublicinterface.h"
#include "plugins_common.h"

namespace CuteReport {
class BandInterface;
class DatasetInterface;
}

SUIT_BEGIN_NAMESPACE
class RendererProcessor;
class RendererItemInterface;
SUIT_END_NAMESPACE

USING_SUIT_NAMESPACE

SUIT_BEGIN_NAMESPACE
class RendererItemInterface : public CuteReport::RendererPublicInterface
{
    Q_OBJECT
public:
    explicit RendererItemInterface(SUIT_NAMESPACE::RendererProcessor *parent = 0);
    
    virtual void setValue(const QString & valueName, const QVariant &value);
    virtual QVariant getValue(const QString & valueName);

    virtual QString processString(const CuteReport::ScriptString & scriptString);
    virtual QString processString(CuteReport::ScriptString & scriptString);

    virtual int currentPageNumber();
    virtual void setCurrentPageNumber(int value);
    virtual PageDrawState currentState();

    virtual QRectF pageFreeSpace(const QString &pageName);
    virtual void setPageFreeSpace(const QRectF &rect, const QString & pageName = QString());
    virtual QPointF currentBandDelta();
    virtual void setCurrentBandDelta(const QPointF &point); // mm

    virtual void registerBandToDatasetIteration(const QString &datasetName, const QString & objectName);
    virtual void registerBandToDatasetIteration(const QString & datasetName, CuteReport::BandInterface * band);
    virtual void unregisterBandFromDatasetIteration(const QString & datasetName, const QString & objectName);
    virtual void unregisterBandFromDatasetIteration(const QString &datasetName, CuteReport::BandInterface * band);
    virtual CuteReport::BandsList bandRegisteredToDataset(const QString & datasetName);

    virtual void registerStrings(const QList<CuteReport::InternalStringData> &strData, CuteReport::BaseItemInterface *item);
    void resetAggregateFunctions(CuteReport::BandInterface * band);

    virtual CuteReport::DatasetInterface * dataset(const QString &datasetname);
    virtual void iterateDataset(CuteReport::DatasetInterface * dtst);
    virtual void iterateDataset(const QString & objectName);
//    virtual void processBand(CuteReport::BandInterface * band);
//    virtual void processBand(const QString & objectName);
    virtual void newPage();
    virtual void newColumnOrPage();

    virtual QVariant getStorageObject(const QString & objectUrl);

    virtual CuteReport::BandInterface * lastProcessedBand();
    virtual CuteReport::BandInterface * currentProcessingBand();
    virtual void prepareCurrentPage();
    virtual void postprocessCurrentPage();
    virtual quint32 lastProcessedItemId(const QString & itemName);
    virtual CuteReport::RenderedItemInterface * lastProcessedItemPointer(const QString & itemName);
    virtual CuteReport::RenderedItemInterface * processedItemPointer(const QString & itemName, quint32 id);

    virtual void error(const QString & /*sender*/, const QString & /*errorMessage*/){}

    virtual bool printBand(const QString & bandName, bool withChildren = true, const QPointF absPos = QPointF(),
                              const QString & unit = QString(), QList<quint32> * renderedIdList = 0);
    virtual bool printBand(const QString & bandName, const QString & pageName, bool withChildren = true,
                                       const QPointF absPos = QPointF(), const QString & unit = QString(), QList<quint32> * renderedIdList = 0);

    virtual bool printItem(const QString & itemName, bool withChildren = true, const QPointF absPos = QPointF(),
                              const QString & unit = QString(), QList<quint32> * renderedIdList = 0 );
    virtual bool printItem(const QString & itemName, quint32 renderedParentId, bool withChildren = true,
                              const QPointF absPos = QPointF(), const QString & unit = QString(),
                              QList<quint32> * renderedIdList = 0 );

    virtual bool processPage(const QString &templatePageName, int renderedPageCount = -1, QList<quint32> * renderedPagesIdList = 0);

    virtual QString name();

    void run();

    virtual int passNumber() {return 0;}
    virtual int passTotal() {return 1;}
    virtual void setPassTotal(int value) {Q_UNUSED(value);}
    virtual int lineNum();
    virtual void setLineNum(int value);

    virtual QVariant reportVariableValue(const QString & variableName);
    virtual void setReportVariableValue(const QString & variableName, const QVariant & value);
    virtual QVariant reportData(const QString & dataName);

    virtual void setLocaleName(const QString & localeName);
    virtual QString localeName() const;

signals:
    
public slots:
    
private:
//    QString preprocessEvaluateString(QString str, const CuteReport::ItemInterface * item);
//    const CuteReport::BandInterface *getBandForItem(const CuteReport::ItemInterface *item);
//    qreal calculateAgregateFunction(const QString & functName, const QString & expression, qreal previousValue);

    RendererProcessor * m_processor;

    friend class RendererProcessor;
};

SUIT_END_NAMESPACE
#endif // SUIT_NAMESPACE_RENDERERITEMINTERFACE_H
