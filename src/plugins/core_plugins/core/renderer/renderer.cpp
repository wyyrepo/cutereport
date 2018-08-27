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
#include "renderer.h"
#include "cutereport_types.h"
#include "datasetinterface.h"
#include "pageinterface.h"
#include "bandinterface.h"
#include "rendereriteminterface.h"
#include "renderediteminterface.h"
#include "rendererdata.h"
#include "rendererprocessor.h"
#include "reportcore.h"
#include "objectfactory.h"
#include "renderedreport.h"

#include <QtCore>

using namespace CuteReport;

USING_SUIT_NAMESPACE

SUIT_BEGIN_NAMESPACE

static const QString MODULENAME = "Renderer";

Renderer::Renderer(QObject *parent)
    :RendererInterface(parent)
    ,m_processor(0)
    ,m_renderedReport(0)
    ,m_renderingThread(0)
    ,m_report(0)
    ,m_antialiasing(false)
    ,m_textAntialiasing(false)
    ,m_smoothPixmapTransform(false)
    ,m_dpi(0)
    ,m_delay(0)
{
}


Renderer::~Renderer()
{
    delete m_renderedReport;
    delete m_processor;
    if (m_renderingThread)
        m_renderingThread->deleteLater();
}


Renderer::Renderer(const Renderer &dd, QObject * parent)
    :RendererInterface(dd, parent)
    ,m_processor(0)
    ,m_renderedReport(0)
    ,m_renderingThread(0)
    ,m_report(dd.m_report)
    ,m_antialiasing(dd.antialiasing())
    ,m_textAntialiasing(dd.textAntialiasing())
    ,m_smoothPixmapTransform(dd.smoothPixmapTransform())
    ,m_dpi(dd.dpi())
    ,m_delay(dd.delay())
{
}


RendererInterface *Renderer::createInstance(QObject *parent) const
{
    return new Renderer(parent);
}


RendererInterface * Renderer::clone() const
{
    return new Renderer(*this, 0);
}


QString Renderer::moduleShortName() const
{
    return QString("Renderer");
}


void Renderer::run(ReportInterface *report, ThreadingLevel threading)
{
     Q_UNUSED(threading);

    if (m_processor) {
        ReportCore::log(LogInfo, MODULENAME, "Renderer can not be started, because previous task is not completed yet");
        return;
    } else
        ReportCore::log(LogInfo, MODULENAME, "started");

    delete m_renderedReport;
    delete m_processor;
    m_report = report;

    m_processor = new SUIT_NAMESPACE::RendererProcessor(this, report);
    connect(m_processor, SIGNAL(processingPage(int,int,int,int)), this, SIGNAL(processingPage(int,int,int,int))/*, Qt::QueuedConnection*/);
    connect(m_processor, SIGNAL(started()), this, SIGNAL(started())/*, Qt::QueuedConnection*/);
    connect(m_processor, SIGNAL(done(bool,CuteReport::RenderedReport*)), this , SLOT(slotProcessorDone(bool,CuteReport::RenderedReport*))/*, Qt::QueuedConnection*/);
    connect(m_processor, SIGNAL(log(CuteReport::LogLevel,QString,QString)), this, SLOT(slotLog(CuteReport::LogLevel,QString,QString))/*, Qt::QueuedConnection*/);

//    if (threading == ThreadNo)
//        renderer->start();
//    else {
//    }
    m_processor->start();
}


void Renderer::slotLog(CuteReport::LogLevel level, const QString & shortMessage, const QString & fullMessage) const
{
    ReportCore::log(level, "Renderer::Processor", shortMessage, fullMessage);
}


void Renderer::stop()
{
    ReportCore::log(LogInfo, MODULENAME, "cancelled");
    if (m_processor)
        m_processor->terminate();
}


bool Renderer::isRunning()
{
    return m_processor;
}


ScriptString Renderer::processString(ScriptString &scriptString)
{
    if (!scriptString.report) {
        ReportCore::log(LogError, MODULENAME, "Report pointer must be specified");
        return scriptString;
    }
    SUIT_NAMESPACE::RendererProcessor * processor = new SUIT_NAMESPACE::RendererProcessor(this, scriptString.report);
    processor->initScriptEngine();
    processor->processString(scriptString);
    return scriptString;
}


//QString Renderer::processString(ReportInterface *report, const QString &string, const QString &delimiterBegin, const QString &delimiterEnd, QString * error)
//{
//    SUIT_NAMESPACE::RendererProcessor * processor = new SUIT_NAMESPACE::RendererProcessor(this, report);
//    return processor->processString(string, delimiterBegin, delimiterEnd);
//}


//QString Renderer::processString(ReportInterface *report, const QString &string, QString * error)
//{
//    SUIT_NAMESPACE::RendererProcessor * processor = new SUIT_NAMESPACE::RendererProcessor(this, report);
//    return processor->processString(string);
//}


QSet<QString> Renderer::reportParameters(ReportInterface *report)
{
    SUIT_NAMESPACE::RendererProcessor * processor = new SUIT_NAMESPACE::RendererProcessor(this, report);
    processor->initScriptEngine();
    return processor->reportParameters(report);
}


CuteReport::RenderedReportInterface * Renderer::takeRenderedReport()
{
    CuteReport::RenderedReportInterface * rReport = m_renderedReport;
    m_renderedReport = 0;
    return rReport;
}


ReportInterface * Renderer::report()
{
    return m_report;
}


void Renderer::slotProcessorDone(bool successfull, RenderedReport * renderedReport)
{
    m_renderedReport = renderedReport;
    ReportCore::log(LogDebug, MODULENAME, "slotProcessorDone");

    m_processor->deleteLater();
    m_processor = 0;

    emit done(successfull);
}


void Renderer::slotReportDestroyed(QObject * object)
{
    Q_UNUSED(object);
    stop();
}


void Renderer::setAntialiasing(bool b)
{
    if (b == m_antialiasing)
        return;

    m_antialiasing = b;

    emit antialiasingChanged(m_antialiasing);
    emit changed();
}


bool Renderer::antialiasing() const
{
    return m_antialiasing;
}


void Renderer::setTextAntialiasing(bool b)
{
    if (b == m_textAntialiasing)
        return;

    m_textAntialiasing = b;

    emit textAntialiasingChanged(m_textAntialiasing);
    emit changed();
}


bool Renderer::textAntialiasing() const
{
    return m_textAntialiasing;
}


void Renderer::setSmoothPixmapTransform(bool b)
{
    if (b == m_smoothPixmapTransform)
        return;

    m_smoothPixmapTransform = b;

    emit smoothPixmapTransformChanged(m_smoothPixmapTransform);
    emit changed();
}


bool Renderer::smoothPixmapTransform() const
{
    return m_smoothPixmapTransform;
}


void Renderer::setDpi(int dpi)
{
    if (dpi == m_dpi)
        return;

    m_dpi = dpi;
    if (m_dpi < 0)
        m_dpi = 0;

    emit dpiChanged(m_dpi);
    emit changed();
}


int Renderer::dpi() const
{
    return m_dpi;
}


void Renderer::setDelay(int delay)
{
    if (delay == m_delay)
        return;

    m_delay = delay;
    if (m_delay < 0)
        m_delay = 0;

    emit delayChanged(m_delay);
    emit changed();
}


int Renderer::delay() const
{
    return m_delay;
}


QString Renderer::_current_property_description() const
{
    QString propertyName = metaObject()->property(m_currentProperty).name();

    if (propertyName == "antialiasing")
        return tr("Use antialiasing while rendering");
    else if (propertyName == "textAntialiasing")
        return tr("Use text antialiasing while rendering");
    else if (propertyName == "smoothPixmapTransform")
        return tr("Use smooth pixmap transformation");
    else if (propertyName == "dpi")
        return tr("Dot Per Inch for rendered page");
    else if (propertyName == "iterationDelay")
        return tr("Use iteration delay for debug");

    return RendererInterface::_current_property_description();
}


SUIT_END_NAMESPACE

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Renderer, SUIT_NAMESPACE::Renderer)
#endif
