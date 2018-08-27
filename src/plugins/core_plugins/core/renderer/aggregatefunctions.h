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

#ifndef AGGREGATEFUNCTIONS_H
#define AGGREGATEFUNCTIONS_H

#include <QHash>
#include <QPair>
#include <QScriptEngine>
#include "plugins_common.h"

namespace CuteReport {
class DatasetInterface;
class BandInterface;
class ReportInterface;
}

SUIT_BEGIN_NAMESPACE
class ScriptEngine;
SUIT_END_NAMESPACE

USING_SUIT_NAMESPACE

SUIT_BEGIN_NAMESPACE
/** functions: sum, min, max, avg, count
 *  format: function(dataset."fieldname" [, precision])
 */


class AggregateFunctions
{
public:
    struct FunctionStruct {
        FunctionStruct()
            : value(0), iterationNumber(0)
        {}
        qreal value;
        int iterationNumber;
    };

    AggregateFunctions();

    void reset();

    /** add new function to process list, if not exists */
    void addFunction(const QString & bandName, const QString & datasetName, const QString & fieldName, const QString & functionName);

    qreal getValue(const QString & bandName, const QString & datasetName, const QString & fieldName, const QString & functionName, bool *error = 0);

    bool initialItemScriptPreprocess(QString &script, const QString &carrierBandName, CuteReport::ReportInterface *report, QStringList * errors);
    bool itemScriptPreprocess(QString &script, const QString &carrierBandName, CuteReport::ReportInterface *report, QStringList * errors);

    void processDatasetIteration(CuteReport::DatasetInterface * dataset);

    void resetValuesForDataset(const QString & datasetName);
    void resetValuesForBand(const QString & bandName, bool delayedTillNextRecursion = true);

    void registerScriptObjects(ScriptEngine * scriptEngine);
    bool mainScriptPreprocess(QString & script, CuteReport::ReportInterface * report, QStringList *errors = 0);

    static QScriptValue sum(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue avg(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue min(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue max(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue count(QScriptContext *context, QScriptEngine *engine);

private:
    bool _scriptPreprocess(QString &script, const QString &defaultBandName, CuteReport::ReportInterface *report, bool replace, bool addNewFunctions, QStringList * errors);
    static QScriptValue _scriptableFunction(QScriptContext *context, QScriptEngine *engine, const QString &funcName);

    qreal getValue(const QString & funcName, FunctionStruct & funcStruct);
    void accumulataValue(const QString & funcName, FunctionStruct & funcStruct, qreal value);

    bool isCommentOrString(const QString & str, int pos, int length);

    typedef QHash<QString, FunctionStruct> FunctionData; // functionName, data
    typedef QHash<QString, FunctionStruct>::iterator FieldFunctionsIterator;

    typedef QHash<QString, FunctionData> FieldData; // fieldName, data
    typedef QHash<QString, FunctionData>::iterator  FieldDataIterator;

    typedef QHash<QString, FieldData> DatasetData;  // datasetName, DatasetFields
    typedef QHash<QString, FieldData>::iterator DatasetDataIterator;

    typedef QHash<QString, DatasetData> BandData; // bandName, DatasetFields
    typedef QHash<QString, DatasetData>::iterator BandDataIterator;

    BandData m_data;
    QList<QString> m_delayedBandReset;
};

SUIT_END_NAMESPACE

#endif // AGGREGATEFUNCTIONS_H
