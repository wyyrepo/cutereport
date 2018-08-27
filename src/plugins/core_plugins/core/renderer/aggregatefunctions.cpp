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

#include "aggregatefunctions.h"
#include "bandinterface.h"
#include "datasetinterface.h"
#include "scriptengine.h"
#include "reportinterface.h"

SUIT_BEGIN_NAMESPACE

const QString regExp1("\\b(sum|min|max|avg)\\s*\\(\\s*(\\w+)\\.getValue\\(\\\"(\\w+)\\\"\\s*\\)\\s*(,\\s*(\\w*))?\\s*\\)");
const QString regExp2("(count)\\s*\\(\\s*(\\w+)\\s*()(,\\s*(\\w*))?\\s*\\)");

AggregateFunctions::AggregateFunctions()
{
}


void AggregateFunctions::reset()
{
    m_delayedBandReset.clear();
    m_data.clear();
}


void AggregateFunctions::addFunction(const QString &bandName, const QString &datasetName, const QString &fieldName, const QString &functionName)
{
    BandDataIterator bandDataIt = m_data.find(bandName);
    if (bandDataIt == m_data.end()) {
        DatasetData d;
        bandDataIt = m_data.insert(bandName, d);
    }

    DatasetDataIterator datasetDataIt = bandDataIt.value().find(datasetName);
    if (datasetDataIt == bandDataIt.value().end()) {
        FieldData d;
        datasetDataIt = bandDataIt.value().insert(datasetName, d);
    }

    FieldDataIterator fieldDataIt = datasetDataIt.value().find(fieldName);
    if (fieldDataIt == datasetDataIt.value().end()) {
        FunctionData d;
        fieldDataIt = datasetDataIt.value().insert(fieldName, d);
    }

    FieldFunctionsIterator funcIt = fieldDataIt.value().find(functionName);
    if (funcIt == fieldDataIt.value().end()) {
        FunctionStruct s;
        fieldDataIt.value().insert(functionName, s);
    }
}


qreal AggregateFunctions::getValue(const QString &bandName, const QString &datasetName, const QString &fieldName, const QString &functionName, bool * error)
{
    BandDataIterator bandDataIt = m_data.find(bandName);
    if (bandDataIt != m_data.end()) {
        DatasetDataIterator datasetDataIt = bandDataIt.value().find(datasetName);
        if (datasetDataIt != bandDataIt.value().end()) {
            FieldDataIterator fieldDataIt = datasetDataIt.value().find(fieldName);
            if (fieldDataIt != datasetDataIt.value().end()) {
                FieldFunctionsIterator funcIt = fieldDataIt.value().find(functionName);
                if (funcIt != fieldDataIt.value().end()) {
                    FunctionStruct & s = funcIt.value();
                    return getValue(funcIt.key(), s);
                }
            }
        }
    }

    if (error)
        *error = true;
    return 0.0;
}


bool AggregateFunctions::initialItemScriptPreprocess(QString &script, const QString &carrierBandName, CuteReport::ReportInterface *report, QStringList * errors)
{
    return _scriptPreprocess(script, carrierBandName, report, false, true, errors);
}


bool AggregateFunctions::itemScriptPreprocess(QString &script, const QString &carrierBandName, CuteReport::ReportInterface *report, QStringList *errors)
{
    return _scriptPreprocess(script, carrierBandName, report, true, false, errors);
}


void AggregateFunctions::processDatasetIteration(CuteReport::DatasetInterface *dataset)
{
    foreach (const QString & bandName, m_delayedBandReset) {
        resetValuesForBand(bandName, false);
    }
    m_delayedBandReset.clear();

    BandDataIterator i;
    for (i = m_data.begin(); i != m_data.end(); ++i) {
        DatasetDataIterator dsIt= i.value().find(dataset->objectName());
        if (dsIt != i.value().end()) {
            FieldDataIterator fIt;
            for (fIt = dsIt.value().begin(); fIt != dsIt.value().end(); fIt++) {
                FieldFunctionsIterator funcIt;
                for (funcIt = fIt.value().begin(); funcIt != fIt.value().end(); funcIt++) {
                    FunctionStruct & s = funcIt.value();
                    accumulataValue(funcIt.key(), s, dataset->getValue(fIt.key()).toReal());
                }
            }
            break;
        }
    }
}


void AggregateFunctions::resetValuesForDataset(const QString &datasetName)
{
    BandDataIterator i;
    for (i = m_data.begin(); i != m_data.end(); ++i) {
        DatasetDataIterator dsIt= i.value().find(datasetName);
        if (dsIt != i.value().end()) {
            FieldDataIterator fIt;
            for (fIt = dsIt.value().begin(); fIt != dsIt.value().end(); fIt++) {
                FieldFunctionsIterator funcIt;
                for (funcIt = fIt.value().begin(); funcIt != fIt.value().end(); funcIt++) {
                    FunctionStruct & s = funcIt.value();
                    s.value = 0;
                    s.iterationNumber = 0;
                }
            }
            break;
        }
    }
}


void AggregateFunctions::resetValuesForBand(const QString &bandName, bool delayedTillNextRecursion)
{
    if (delayedTillNextRecursion) {
        m_delayedBandReset.append(bandName);
        return;
    }

    BandDataIterator i = m_data.find(bandName);
    if (i == m_data.end())
        return;

    DatasetDataIterator dsIt;
    for (dsIt = i.value().begin(); dsIt != i.value().end(); ++dsIt) {
        if (dsIt != i.value().end()) {
            FieldDataIterator fIt;
            for (fIt = dsIt.value().begin(); fIt != dsIt.value().end(); fIt++) {
                FieldFunctionsIterator funcIt;
                for (funcIt = fIt.value().begin(); funcIt != fIt.value().end(); funcIt++) {
                    FunctionStruct & s = funcIt.value();
                    s.value = 0;
                    s.iterationNumber = 0;
                }
            }
        }
    }
}


void AggregateFunctions::registerScriptObjects(ScriptEngine *scriptEngine)
{
    scriptEngine->globalObject().setProperty("sum", scriptEngine->newFunction(sum));
    scriptEngine->globalObject().setProperty("min", scriptEngine->newFunction(min));
    scriptEngine->globalObject().setProperty("max", scriptEngine->newFunction(max));
    scriptEngine->globalObject().setProperty("avg", scriptEngine->newFunction(avg));
    scriptEngine->globalObject().setProperty("count", scriptEngine->newFunction(count));
}


bool AggregateFunctions::mainScriptPreprocess(QString &script, CuteReport::ReportInterface *report, QStringList * errors)
{
    return _scriptPreprocess(script, "", report, true, true, errors);
}


bool AggregateFunctions::_scriptPreprocess(QString &script, const QString &parentBandName, CuteReport::ReportInterface *report, bool replace, bool addNewFunctions, QStringList * errors)
{
//    qDebug() << script;
    if (errors)
        (*errors).clear();

    bool result = true;

    QStringList regExpTemplates;
    regExpTemplates << regExp1 << regExp2;

    QRegExp reField;
    reField.setCaseSensitivity(Qt::CaseSensitive);
    reField.setMinimal(false);
    int pos;

    foreach (const QString& str, regExpTemplates) {
        reField.setPattern(str);
        pos = 0;
        while ((pos = reField.indexIn(script, pos)) != -1) {
            int length = reField.matchedLength();

            QString funcName = reField.cap(1);
            QString datasetName = reField.cap(2);
            QString fieldName = reField.cap(3);
            QString bandName = reField.cap(5);

            QString correctBandName = bandName.isEmpty() ? parentBandName : bandName;

            if (!parentBandName.isEmpty() && !report->dataset(datasetName) && errors) {
                result = false;
                *errors << QString("There is no such dataset '%1' in '%2' aggregate function agument").arg(datasetName).arg(funcName);
            }
            if (!parentBandName.isEmpty() && !report->item(correctBandName) && errors) {
                result = false;
                *errors << QString("There is no such band '%1' in '%2' aggregate function agument").arg(bandName).arg(funcName);
            }

            if (addNewFunctions && !correctBandName.isEmpty())
                addFunction(correctBandName, datasetName, fieldName, funcName);

            if (replace && !isCommentOrString(script, pos, length)) {
                QString replacingStr = QString("%1(\'%2\'%3)")
                                       .arg(funcName)
                                       .arg(datasetName + (fieldName.isEmpty() ? "" : "."+fieldName))
                                       .arg(bandName.isEmpty() ? "" : ", \'"+bandName+"\'");
                script.replace(pos, length, replacingStr);
                pos += replacingStr.length();
            } else {
                pos += length;
            }
        }
    }

//    qDebug() << script;

    return result;
}


QScriptValue AggregateFunctions::sum(QScriptContext *context, QScriptEngine *engine)
{
    return _scriptableFunction(context, engine, "sum");
}


QScriptValue AggregateFunctions::avg(QScriptContext *context, QScriptEngine *engine)
{
    return _scriptableFunction(context, engine, "avg");
}


QScriptValue AggregateFunctions::min(QScriptContext *context, QScriptEngine *engine)
{
    return _scriptableFunction(context, engine, "min");
}


QScriptValue AggregateFunctions::max(QScriptContext *context, QScriptEngine *engine)
{
    return _scriptableFunction(context, engine, "max");
}


QScriptValue AggregateFunctions::count(QScriptContext *context, QScriptEngine *engine)
{
    return _scriptableFunction(context, engine, "count");
}


QScriptValue AggregateFunctions::_scriptableFunction(QScriptContext *context, QScriptEngine *engine, const QString &funcName)
{
    ScriptEngine* scriptEngine = dynamic_cast<ScriptEngine*>(engine);
    if (!scriptEngine)
        return engine->undefinedValue();
    QScriptValue arg1 = context->argument(0);
    QScriptValue arg2 = context->argument(1);
//    qDebug() << arg1.toString() << arg2.toString();
//    qDebug() << arg1.isString() << arg2.isString() << arg2.isUndefined();
    if (arg1.isString() && (arg2.isUndefined() || arg2.isString())) {
        CuteReport::BandInterface * band = scriptEngine->processor()->rendererItemInterface()->currentProcessingBand();
        QString datasetName = arg1.toString().section(".", 0,0);
        QString fieldName = arg1.toString().section(".", 1,1);
        QString bandName  = arg2.isUndefined() ? (band ? band->objectName() : "") : arg2.toString();
        return scriptEngine->processor()->aggregateFunctions()->getValue(bandName, datasetName, fieldName, funcName);
    }
    return scriptEngine->undefinedValue();
}


qreal AggregateFunctions::getValue(const QString & funcName, FunctionStruct & funcStruct)
{
    if (funcName == "sum") {
        return funcStruct.value ;
    } else if (funcName  == "avg") {
        return (funcStruct.value / funcStruct.iterationNumber);
    } else if (funcName  == "min") {
        return funcStruct.value;
    } else if (funcName  == "max") {
        return funcStruct.value;
    } else if (funcName == "count")
        return funcStruct.iterationNumber;

    return 0.0;
}


void AggregateFunctions::accumulataValue(const QString & funcName, FunctionStruct & funcStruct, qreal value)
{
    if (funcName == "sum") {
        funcStruct.value += value;
    } else if (funcName  == "avg") {
        funcStruct.value += value;
    } else if (funcName  == "min") {
        if (funcStruct.iterationNumber == 0)
            funcStruct.value += value;
        else
            funcStruct.value = qMin(funcStruct.value, value);
    } else if (funcName  == "max") {
        if (funcStruct.iterationNumber == 0)
            funcStruct.value += value;
        else
            funcStruct.value = qMax(funcStruct.value, value);
    }

    funcStruct.iterationNumber++;
}


bool AggregateFunctions::isCommentOrString(const QString &str, int pos, int length)
{
    Q_UNUSED(length);

    int singleQuoteCount = 0;
    int doubleQuoteCount = 0;
    int max = qMin(pos, str.length());
    for (int i = 0; i < max; ++i) {
        if (str[i] == '\'') singleQuoteCount++;
        else if (str[i] == '\"') doubleQuoteCount++;
    }

    if (singleQuoteCount % 2 != 0) return true;
    else if (doubleQuoteCount % 2 != 0) return true;

    return false;
}

SUIT_END_NAMESPACE
