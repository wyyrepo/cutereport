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

#ifndef RENDERERPROCESSOR_H
#define RENDERERPROCESSOR_H

#include "rendererinterface.h"
#include "rendereriteminterface.h"
#include "rendererpublicinterface.h"
#include "cutereport_globals.h"
#include "plugins_common.h"

#include <QScriptValue>
#include <QMutex>
#include <QObject>


/** global variables :
 * LINE - current dataset line starting from 1
 * PAGE - current page number starting from 1
 * PAGES - total pages (require report double pass)
 * PASSES - report pass number
 * TPAGE - current page of template: means number of page in designer
 * TPAGES - total pages of template: means number of pages in designer (starting from 1)
 */


namespace CuteReport {
class DatasetInterface;
class BandInterface;
class BaseItemInterface;
class PageInterface;
class RenderedItemInterface;
class RenderedReport;
}

SUIT_BEGIN_NAMESPACE
class Renderer;
class RendererData;
class AggregateFunctions;
class PreParser;
class Variables;
SUIT_END_NAMESPACE

USING_SUIT_NAMESPACE

SUIT_BEGIN_NAMESPACE
class RendererProcessor: public QObject
{
    Q_OBJECT
public:
    explicit RendererProcessor(Renderer * renderer, CuteReport::ReportInterface * report);
    ~RendererProcessor();

    void initScriptEngine();

    void start();
    void terminate();

    void registerBandToDatasetIteration(const QString &datasetName, const QString & objectName);
    void registerBandToDatasetIteration(const QString & datasetName, CuteReport::BandInterface * band);
    void unregisterBandFromDatasetIteration(const QString & datasetName, const QString & objectName);
    void unregisterBandFromDatasetIteration(const QString &datasetName, CuteReport::BandInterface * band);
    CuteReport::BandsList bandRegisteredToDataset(const QString & datasetName);

    void processString(CuteReport::ScriptString & scriptString);
//    QString processString(const QString & string, const QObject *object = 0, QString *error = 0);
    void registerRenderingStrings(const QList<CuteReport::InternalStringData> &strDatas, CuteReport::BaseItemInterface *item);
    QString preprocessEvaluateString(const QString &str, const QObject *object);
    bool preprocessScript(QString &str);

    const CuteReport::BandInterface *getBandForItem(const QObject *object);

    AggregateFunctions *aggregateFunctions() const;
    RendererItemInterface *rendererItemInterface() const;

    void sendLog(CuteReport::LogLevel level, const QString & shortMessage, const QString & fullMessage);

    QSet<QString> reportParameters(CuteReport::ReportInterface *report);

public slots:
    void run();

signals:
    void started();
    void message(int logLevel, QString msg);
    void done(bool withouErrors, CuteReport::RenderedReport *);
    void processingPage(int page, int total, int pass, int passTotal);
    void log(CuteReport::LogLevel level, const QString & shortMessage, const QString & fullMessage);

private slots:
    void _done(bool success);
    void scriptEngineException(QScriptValue value);

private:
    void registerEvaluationString(const QString & string, const QString & delimiterBegin, const QString & delimiterEnd, CuteReport::BaseItemInterface *item);
    void registerEvaluationString(const QString & string, CuteReport::BaseItemInterface *item);
    void processEventsIfNeeded();
    void resetScriptEngine();
    bool evaluateScript();
    void resetData();
    void renderReportPage(CuteReport::PageInterface * page);
    void initBands(QList<CuteReport::BandInterface*> bands);
    void newColumnOrPage();
    void firstColumn();
    void shiftColumn();
    void createNewRenderingPage();
    void completePage(CuteReport::RenderedPageInterface *page);
    void processBand(CuteReport::BandInterface * band, CuteReport::RenderingStage stage);
    bool prepareItemData(CuteReport::BaseItemInterface * item, QList<CuteReport::BaseItemInterface*> & activeItems, bool withChildren);
    void placeItem(CuteReport::BaseItemInterface * item, CuteReport::RenderedItemInterface *parent, QList<CuteReport::BaseItemInterface*> &processedItems, bool withChildren = true);
    void completePlacedItem(CuteReport::BaseItemInterface * item, QList<CuteReport::BaseItemInterface*> &processedItems, bool withChildren = true);
    void processDataset(CuteReport::DatasetInterface * dtst);
    bool canFitBandToPage(CuteReport::BandInterface * band);
    bool terminated();
    QVariant getValue(const QString & valueName);
    void setValue(const QString & valueName, const QVariant &value, QScriptValue::PropertyFlags flag = QScriptValue::KeepExistingFlags);
    CuteReport::DatasetInterface * datasetRegisteredTo(CuteReport::BandInterface * band);

    QString _processString(const QString & string, CuteReport::ScriptString &scriptString);

private:
    SUIT_NAMESPACE::RendererData * m_data;
    RendererItemInterface * m_rendererItemInterface;
    bool m_terminate;
    QMutex mutex;
    QList<CuteReport::BandInterface*> topBands;
    QList<CuteReport::BandInterface*> bottomBands;
    QList<CuteReport::BandInterface*> freeBands;

    CuteReport::PageInterface * m_currentPage;
    CuteReport::DatasetInterface * m_currentDataset;
    int m_currentPageNumber;
    int m_currentDatasetLine;
    QRectF m_freeSpace;
    QPointF m_bandDelta;
    //QList<CuteReport::RenderedPageInterface*> m_renderedPages;
    CuteReport::RenderedPageInterface * m_currentRenderedPage;
    QList<CuteReport::BandInterface *> m_bandsDone;
    CuteReport::BandInterface *m_lastProcessedBand;
    CuteReport::BandInterface *m_processingBand;
    QHash<QString, CuteReport::BandInterface*> m_datasetRegister; // dataset, bands
//    QHash<QString, QString> m_groups; // groups, datasets_name

    bool m_antialiasing;
    bool m_textAntialiasing;
    bool m_smoothPixmapTransform;
    int m_dpi;
    int m_delay;
    CuteReport::RendererPublicInterface::PageDrawState m_state;
    AggregateFunctions * m_aggregateFunctions;
    PreParser * m_preparser;
    Variables * m_variables;
    bool m_runs;
    bool m_processEvents;

    int m_pages;
    quint32 m_lastItemId;
    QHash<QString, quint32> m_lastIdForItem;
    int m_currentColumn;
    qreal m_columnableTop;
    qreal m_columnableBottom;

    friend class RendererItemInterface;
    friend class Variables;
};


class Thread: public QThread
{
    Q_OBJECT
public:
    explicit Thread();
    ~Thread();
//public slots:
//    void stop();
};

SUIT_END_NAMESPACE

#endif // RENDERERPROCESSOR_H

/*
 dataset: band: function: value

  */
