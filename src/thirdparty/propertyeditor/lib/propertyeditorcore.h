/***************************************************************************
 *   This file is part of the propertyEditor project                       *
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bog_dan_ro@yahoo.com                                                  *
 *   Copyright (C) 2013 by Mikhalov Alexander                              *
 *   alexander.mikhalov@gmail.com                                          *
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
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/

#ifndef PROPERTYEDITORCORE_H
#define PROPERTYEDITORCORE_H

#include "globals.h"
#include <QObject>

namespace PropertyEditor
{

enum LogLevel {
    LogUndefined = 0,
    LogDebug = 1,
    LogInfo = 2,
    LogError = 3,
    LogWarning = 4,
    LogCritical = 5,
    LogMetric = 6,
    LOGLEVELMAXVALUE = 7
};

class PropertyInterface;

class PROPERTYEDITOR_EXPORTS Core : public QObject
{
    Q_OBJECT
public:

    const QList<PropertyInterface *> &plugins() const;
    
    static Core *instance();
    static void createInstance(bool init = true);
    static void refInc();
    static void refDec();

    void init();
signals:
    void log(int, QString, QString, QString);

private:
    Core();
    ~Core();
    void loadPlugins();

    static Core * m_core;
    static int m_refCounter;

    QList<PropertyInterface*> m_plugins;
};

}

#endif // PROPERTYEDITORCORE_H
