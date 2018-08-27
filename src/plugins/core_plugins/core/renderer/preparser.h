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
#ifndef PREPARSER_H
#define PREPARSER_H

#include <QString>
#include <QScriptValue>
#include <QScriptContext>

#include "plugins_common.h"

namespace CuteReport {
class DatasetInterface;
class BandInterface;
class ReportInterface;
class BaseItemInterface;
}

SUIT_BEGIN_NAMESPACE
class ScriptEngine;
SUIT_END_NAMESPACE

USING_SUIT_NAMESPACE

SUIT_BEGIN_NAMESPACE
class PreParser
{
public:
    PreParser();

    void registerScriptObjects(ScriptEngine * scriptEngine);
    bool mainScriptPreprocess(QString & script, QStringList *errors = 0);

    bool initialItemScriptPreprocess(QString &script, const CuteReport::BaseItemInterface* item, QStringList * errors);
    bool itemScriptPreprocess(QString &script, const QObject *object, QStringList * errors);

    static QSet<QString> findParameterVariables(const QString & str);

private:
    static QScriptValue scriptPrint(QScriptContext *context, QScriptEngine *engine);

    void datasetFieldReplace(QString & script);
    void parameterVariablesReplace(QString &script);
    void formattingReplace(QString &script);

};

SUIT_END_NAMESPACE
#endif // PREPARSER_H
