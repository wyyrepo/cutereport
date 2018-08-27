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

#ifndef RENDERERDATA_H
#define RENDERERDATA_H
#include "rendererinterface.h"
#include "scriptengine.h"

#include <QMutexLocker>
#include <QPointer>

namespace CuteReport {
class FormInterface;
}

SUIT_BEGIN_NAMESPACE
class Thread;
class Renderer;
class RendererProcessor;
SUIT_END_NAMESPACE

USING_SUIT_NAMESPACE

SUIT_BEGIN_NAMESPACE
class RendererData
{
public:
    explicit RendererData();
    ~RendererData();

    void clear();
    void reset();

    void appendPage(CuteReport::RenderedPageInterface * page);
    CuteReport::RenderedPageInterface* getPage(int number);
    int pagesCount();
    void appendError(QString error);
    QStringList errors();

    CuteReport::ReportInterface * origReport;
    QPointer<CuteReport::ReportInterface> workingReportCopy;
    SUIT_NAMESPACE::Renderer * renderer;
    QPointer<ScriptEngine> scriptEngine;
    QPointer<RendererProcessor> processor;
    int dpi;

    QList<CuteReport::RenderedPageInterface* > pages;
private:
    QStringList m_errors;
};

SUIT_END_NAMESPACE

#endif // RENDERERDATA_H
