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
#ifndef RENDERERTOITEMINTERFACE_H
#define RENDERERTOITEMINTERFACE_H

#include <QObject>
#include <QRectF>
#include "cutereport_globals.h"
#include "cutereport_types.h"

namespace CuteReport
{

class BaseItemInterface;
class BandInterface;
class DatasetInterface;
class StorageInterface;
class FormInterface;
class RenderedPageInterface;
class RenderedItemInterface;
class PageInterface;

class CUTEREPORT_EXPORTS RendererPublicInterface : public QObject
{
    Q_OBJECT
public:
    enum PageDrawState {EmptyPage = 0, BackgroundDone = 1, HeadersDone = 2, FootersDone = 3, DrawingContent = 4, ContentDone = 5, ForegroundDone = 6};

    explicit RendererPublicInterface(QObject *parent = 0);
    virtual ~RendererPublicInterface();
    
    /** set/read global variable accessible to any item */
    Q_INVOKABLE virtual void setValue(const QString & valueName, const QVariant & value) = 0;
    Q_INVOKABLE virtual QVariant getValue(const QString & valueName) = 0;

    Q_INVOKABLE virtual QString processString(const CuteReport::ScriptString & scriptString) = 0;
    Q_INVOKABLE virtual QString processString(CuteReport::ScriptString & scriptString) = 0;

    Q_INVOKABLE virtual int currentPageNumber() = 0; // 1,2,3...
    Q_INVOKABLE virtual void setCurrentPageNumber(int value) = 0; // 1,2,3...

    Q_INVOKABLE virtual PageDrawState currentState() = 0;

    Q_INVOKABLE virtual QRectF pageFreeSpace(const QString & pageName = QString()) = 0; // if empty, then current page;
    Q_INVOKABLE virtual void setPageFreeSpace(const QRectF &rect, const QString & pageName = QString()) = 0; // if empty, then current page;
    Q_INVOKABLE virtual QPointF currentBandDelta() = 0; // mm
    Q_INVOKABLE virtual void setCurrentBandDelta(const QPointF & point) = 0; // mm

    /** manage bands that will be processed by renderer for each dataset iteration */
    Q_INVOKABLE virtual void registerBandToDatasetIteration(const QString &datasetName, const QString & objectName) = 0;
    Q_INVOKABLE virtual void registerBandToDatasetIteration(const QString & datasetName, CuteReport::BandInterface * band) = 0;
    Q_INVOKABLE virtual void unregisterBandFromDatasetIteration(const QString & datasetName, const QString & objectName) = 0;
    Q_INVOKABLE virtual void unregisterBandFromDatasetIteration(const QString &datasetName, CuteReport::BandInterface * band) = 0;
    Q_INVOKABLE virtual BandsList bandRegisteredToDataset(const QString & datasetName) = 0;

    /** Register strings that have to be processed before printing in some kind
     *  All scrings that have scripting, functions, translation, etc should be registered before rendering
    */
    Q_INVOKABLE virtual void registerStrings(const QList<CuteReport::InternalStringData> &strData, CuteReport::BaseItemInterface *item) = 0;

    /** method for clearing all accumulated aggregate values at the end of current iteration
     *  keep in mind values will not be reset immediately, so they will be accessable till iteration changed */
    Q_INVOKABLE virtual void resetAggregateFunctions(CuteReport::BandInterface * band) = 0;

    Q_INVOKABLE virtual CuteReport::DatasetInterface * dataset(const QString &datasetname) = 0;
    Q_INVOKABLE virtual void iterateDataset(CuteReport::DatasetInterface * dtst) = 0;
    Q_INVOKABLE virtual void iterateDataset(const QString & objectName) = 0;

//    Q_INVOKABLE virtual void processBand(CuteReport::BandInterface * band) = 0;
//    Q_INVOKABLE virtual void processBand(const QString & objectName) = 0;

    Q_INVOKABLE virtual void newPage() = 0;
    Q_INVOKABLE virtual void newColumnOrPage() = 0;

    Q_INVOKABLE virtual QVariant getStorageObject(const QString & objectUrl) = 0;

    Q_INVOKABLE virtual BandInterface * lastProcessedBand() = 0;
    Q_INVOKABLE virtual CuteReport::BandInterface * currentProcessingBand() = 0;
    Q_INVOKABLE virtual void prepareCurrentPage() = 0;
    Q_INVOKABLE virtual void postprocessCurrentPage() = 0;
    Q_INVOKABLE virtual quint32 lastProcessedItemId(const QString & itemName) = 0;
    /*Q_INVOKABLE*/ virtual CuteReport::RenderedItemInterface * lastProcessedItemPointer(const QString & itemName) = 0;
    virtual CuteReport::RenderedItemInterface * processedItemPointer(const QString & itemName, quint32 id) = 0;

    Q_INVOKABLE virtual int passNumber() = 0; // starting from 1
    Q_INVOKABLE virtual int passTotal() = 0;
    Q_INVOKABLE virtual void setPassTotal(int value) = 0;
    Q_INVOKABLE virtual int lineNum() = 0;
    Q_INVOKABLE virtual void setLineNum(int value) = 0;

    Q_INVOKABLE virtual QVariant reportVariableValue(const QString & variableName) = 0;
    Q_INVOKABLE virtual void setReportVariableValue(const QString & variableName, const QVariant & value) = 0;
    Q_INVOKABLE virtual QVariant reportData(const QString & dataName) = 0;

    Q_INVOKABLE virtual void setLocaleName(const QString & localeName) = 0;
    Q_INVOKABLE virtual QString localeName() const = 0;

    Q_INVOKABLE virtual void error(const QString & sender, const QString & errorMessage) = 0;

    Q_INVOKABLE virtual QString name() = 0;

    Q_INVOKABLE virtual bool printBand(const QString & bandName, bool withChildren = true, const QPointF absPos = QPointF(),
                                       const QString & unit = QString(),
                                       QList<quint32> * renderedIdList = 0) = 0;
    Q_INVOKABLE virtual bool printBand(const QString & bandName, const QString & pageName, bool withChildren = true,
                                       const QPointF absPos = QPointF(), const QString & unit = QString(),
                                       QList<quint32> * renderedIdList = 0) = 0;

    /// if pos is not set then current pos is applied
    /// if unit is not set than it is the Item's unit
    /// result of the methods is printed item id, or 0 if not printed for some reasons
    Q_INVOKABLE virtual bool printItem(const QString & itemName, bool withChildren = true, const QPointF absPos = QPointF(),
                                       const QString & unit = QString(),
                                       QList<quint32> * renderedIdList = 0 ) = 0;
    Q_INVOKABLE virtual bool printItem(const QString & itemName, quint32 renderedParentId, bool withChildren = true,
                                       const QPointF absPos = QPointF(), const QString & unit = QString(),
                                       QList<quint32> * renderedIdList = 0) = 0;

    virtual bool processPage(const QString &templatePageName, int renderedPageCount = -1, QList<quint32> * renderedPagesIdList = 0) = 0;

    Q_INVOKABLE virtual void run() = 0;

signals:
    void reportStart();

    void templatePageBefore(CuteReport::PageInterface * page);
    void templatePageAfter(CuteReport::PageInterface * page);

    void pageBefore(CuteReport::RenderedPageInterface * page);
    void pageDataAfter(CuteReport::RenderedPageInterface * page);
    void pageDesignAfter(CuteReport::RenderedPageInterface * page);
    void pageAfter(CuteReport::RenderedPageInterface * page);

    void bandBefore(CuteReport::BandInterface * band);
    void bandDataAfter(CuteReport::BandInterface * band);
    void bandLayoutBefore(CuteReport::BandInterface * band);
    void bandLayoutAfter(CuteReport::BandInterface * band);
    void bandAfter(CuteReport::BandInterface * band);

    void itemBefore(CuteReport::BaseItemInterface * item);
    void itemDataAfter(CuteReport::BaseItemInterface * item);
    void itemLayoutBefore(CuteReport::BaseItemInterface * item);
    void itemLayoutAfter(CuteReport::BaseItemInterface * item);
    void itemRenderingEndBefore(CuteReport::BaseItemInterface * item);
    void itemAfter(CuteReport::BaseItemInterface * item);

    void datasetBefore(CuteReport::DatasetInterface * dataset);
//    void datasetIteration(CuteReport::DatasetInterface * dataset);
    void requestIteration(CuteReport::BandInterface * band, bool * allow);
    void datasetAfter(CuteReport::DatasetInterface * dataset);

    void formBefore(CuteReport::FormInterface * dataset);
    void formAfter(CuteReport::FormInterface * dataset);

    void reportDone();
};

}

#endif // RENDERERTOITEMINTERFACE_H
