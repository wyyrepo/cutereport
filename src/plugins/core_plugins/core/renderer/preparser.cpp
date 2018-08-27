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

#include "preparser.h"
#include "cutereport_functions.h"
#include "scriptengine.h"

#include <QRegExp>
#include <QDebug>

SUIT_BEGIN_NAMESPACE

static const QString ParameterVariablesRegExp("(?:^|[^\\\\](?=\\$))\\$\\{(\\w+)\\}");
static const int ParameterVariablesRegExpNum = 1;

PreParser::PreParser()
{
}

void PreParser::registerScriptObjects(ScriptEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)
    scriptEngine->globalObject().setProperty("print", scriptEngine->newFunction( &PreParser::scriptPrint ) );
}


bool PreParser::mainScriptPreprocess(QString &script, QStringList *errors)
{
    Q_UNUSED(errors);
    datasetFieldReplace(script);
    return true;
}


bool PreParser::initialItemScriptPreprocess(QString &script, const CuteReport::BaseItemInterface *item, QStringList *errors)
{
    Q_UNUSED(item);
    Q_UNUSED(errors);
    datasetFieldReplace(script);
    return true;
}


bool PreParser::itemScriptPreprocess(QString &script, const QObject *object, QStringList *errors)
{
    Q_UNUSED(object);
    Q_UNUSED(errors);

    parameterVariablesReplace(script);
    datasetFieldReplace(script);
    formattingReplace(script);

    return true;
}


QSet<QString> PreParser::findParameterVariables(const QString &str)
{
    QSet<QString> set;

    QRegExp rx(ParameterVariablesRegExp);
    rx.setMinimal(true);
    int pos = 0;
    while (pos >= 0) {
        pos = rx.indexIn(str, pos);
        if (pos != -1) {
            set.insert( rx.cap(ParameterVariablesRegExpNum) );
            pos += rx.cap(0).length();
        }
    }

    return set;
}



void PreParser::parameterVariablesReplace(QString & script)
{
    QRegExp rx(ParameterVariablesRegExp);
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(script, pos)) != -1) {
            int length = rx.matchedLength();
            QString result = QString("__%1_").arg(rx.cap(ParameterVariablesRegExpNum));

            script.replace(pos, length, result);
            pos += result.length();
    }
}


void PreParser::datasetFieldReplace(QString &script)
{
    QRegExp rx("(\\w+)\\.(\\\"|&quot;)(\\w*)(\\\"|&quot;)(\\.(\\d+))?(:(\\d+))?");
    rx.setMinimal(false);

    int pos = 0;
    while ((pos = rx.indexIn(script, pos)) != -1) {
        int length = rx.matchedLength();
        QString row = rx.cap(6);
        QString result = QString("%1.getValue(\"%2\"%3)").arg(rx.cap(1)).arg(rx.cap(3)).arg(row.isEmpty() ? "" : ", " + row);

        script.replace(pos, length, result);
        pos += result.length();
    }
}


void PreParser::formattingReplace(QString &script)
{
    const QString regExpTagInline("\\s+\\#(.+)$");

    QRegExp reField(regExpTagInline, Qt::CaseSensitive);
    reField.setMinimal(false);

    int pos;
    if ((pos = reField.indexIn(script)) != -1) {
        int length = reField.matchedLength();
        script.remove(pos, length);
    }
}


QScriptValue PreParser::scriptPrint( QScriptContext *context, QScriptEngine *engine)
{
    ScriptEngine * eng = reinterpret_cast<ScriptEngine *>(engine);
    if (context->argumentCount() < 1)
        return QString();
    QString str(context->argument(0).toString());
    for (int i=1; i<context->argumentCount();++i)
        str = str.arg(context->argument(i).toString());
    eng->processor()->sendLog(CuteReport::LogInfo, "Print", str );
    return engine->undefinedValue();
}

SUIT_END_NAMESPACE
