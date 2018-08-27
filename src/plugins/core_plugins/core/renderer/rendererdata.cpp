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

#include "rendererdata.h"
#include "pageinterface.h"
#include "rendererprocessor.h"
#include "forminterface.h"

#include <QScriptEngine>
#include <QWidget>

using namespace CuteReport;


SUIT_BEGIN_NAMESPACE

RendererData::RendererData():
    origReport(0)
  ,renderer(0)
  ,scriptEngine(0)
  ,processor(0)
{
//    scriptEngine = new QScriptEngine();
}


RendererData::~RendererData()
{    
    delete scriptEngine;
    qDeleteAll(pages);
}


void RendererData::clear()
{
    delete scriptEngine;
    scriptEngine = 0;
    qDeleteAll(pages);
    pages.clear();
    m_errors.clear();
}

void RendererData::reset()
{
    qDeleteAll(pages);
    pages.clear();
    //    qDeleteAll(forms);
    //    forms.clear();
    m_errors.clear();
}


//void RendererData::appendForm(CuteReport::FormInterface * form)
//{
//    QPointer<CuteReport::FormInterface> pointer(form);
//    forms.append(pointer);
//}


//int RendererData::formCounter()
//{
//    return forms.size();
//}


//FormInterface *RendererData::form(int num)
//{
//    return forms.at(num).data();
//}


void RendererData::appendPage(RenderedPageInterface * page)
{
    QPointer<RenderedPageInterface> pointer(page);
    pages.append(pointer);
}

RenderedPageInterface* RendererData::getPage(int number)
{
    if (number < pages.count() && number >=0)
        return pages[number];
    else
        return 0;
}

int RendererData::pagesCount()
{
    return pages.count();
}

void RendererData::appendError(QString error)
{
    m_errors.append(error);
}

QStringList RendererData::errors()
{
    return m_errors;
}

SUIT_END_NAMESPACE
