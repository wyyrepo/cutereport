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
#ifndef CUTEREPORTSCRIPTPLUGIN_H
#define CUTEREPORTSCRIPTPLUGIN_H

#include "scriptextensioninterface.h"
#include "cutereport_globals.h"
#include "plugins_common.h"

class Script : public CuteReport::ScriptExtensionInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "CuteReport.ScriptExtensionInterface/1.0")
#endif
    Q_INTERFACES(CuteReport::ScriptExtensionInterface)

public:
    explicit Script(QObject *parent = 0);
    virtual ~Script();

    virtual QString moduleShortName() const;
    virtual QString suitName() const { return "Standard"; }

    QStringList keys() const;
    void initialize(const QString &key, QScriptEngine *engine);

    virtual ScriptExtensionInterface *createInstance(QObject * parent = 0) const;
};

#endif // CUTEREPORTSCRIPTPLUGIN_H
