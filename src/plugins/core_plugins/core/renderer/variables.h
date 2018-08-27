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
#ifndef VARIABLES_H
#define VARIABLES_H

#include  <QScriptValue>
#include "plugins_common.h"

namespace CuteReport {
class DatasetInterface;
class BandInterface;
class ReportInterface;
class BaseItemInterface;
class PageInterface;
}

SUIT_BEGIN_NAMESPACE
class ScriptEngine;
class RendererProcessor;
SUIT_END_NAMESPACE

USING_SUIT_NAMESPACE

SUIT_BEGIN_NAMESPACE
class Variables
{
public:
    explicit Variables();

    void registerScriptObjects(ScriptEngine * scriptEngine);
    bool initialItemScriptPreprocess(QString &script, const CuteReport::BaseItemInterface* item, QStringList * errors);

private:
//    void setValue(const QString & valueName, const QVariant &value, QScriptValue::PropertyFlags flag);

    ScriptEngine * m_scriptEngine;
};

SUIT_END_NAMESPACE

#endif // VARIABLES_H
