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
#include "variables.h"
#include "scriptengine.h"
#include "qdatetime.h"
#include "rendereriteminterface.h"
#include "datasetinterface.h"
#include "bandinterface.h"
#include "rendererdata.h"
#include "reportinterface.h"

SUIT_BEGIN_NAMESPACE


static QScriptValue getPageNum(QScriptContext *, QScriptEngine * engine)
{
    ScriptEngine * e = reinterpret_cast<ScriptEngine*>(engine);
    RendererItemInterface * r = e->processor()->rendererItemInterface();
    return r->currentPageNumber();
}


static QScriptValue getLineNum(QScriptContext *, QScriptEngine * engine)
{
    ScriptEngine * e = reinterpret_cast<ScriptEngine*>(engine);
    RendererItemInterface * r = e->processor()->rendererItemInterface();
    CuteReport::DatasetInterface * ds = r->dataset(r->currentProcessingBand()->dataset());
    return ds ? ds->getCurrentRowNumber() : 0;
}


Variables::Variables()
{
}


void Variables::registerScriptObjects(ScriptEngine *scriptEngine)
{
    m_scriptEngine = scriptEngine;

    m_scriptEngine->globalObject().setProperty("PAGE", m_scriptEngine->newFunction(getPageNum), QScriptValue::PropertyGetter);
    m_scriptEngine->globalObject().setProperty("LINE", m_scriptEngine->newFunction(getLineNum), QScriptValue::PropertyGetter);

    QVariantHash variables = scriptEngine->processor()->m_data->workingReportCopy->variables();
    QVariantHash::const_iterator i;
    for (i = variables.constBegin(); i != variables.constEnd(); ++i) {
        m_scriptEngine->globalObject().setProperty("__"+i.key()+"_",  m_scriptEngine->newVariant(i.value()));
    }
}


bool Variables::initialItemScriptPreprocess(QString &script, const CuteReport::BaseItemInterface *item, QStringList *errors)
{
    Q_UNUSED(item)
    Q_UNUSED(errors)
    Q_UNUSED(script)
    return true;
}


SUIT_END_NAMESPACE
