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

#include "propertyeditorcore.h"
#include "propertyinterface.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfoList>
#include <QStringList>
#include <QDir>
#include <QPluginLoader>

#define MODULENAME "PropertyEditor::Core"


namespace PropertyEditor
{

Core * Core::m_core = 0;
int Core::m_refCounter = 0;


Core *Core::instance()
{
    if (!m_core) {
        m_core = new Core();
        m_core->loadPlugins();
    }
    return m_core;
}


void Core::createInstance(bool init)
{
    m_core = new Core();
    if (init)
        m_core->loadPlugins();
}


void Core::refInc()
{
    ++m_refCounter;
}


void Core::refDec()
{
    --m_refCounter;
    if (!m_refCounter) {
        delete m_core;
        m_core = 0;
    }
}


void Core::init()
{
    emit log((int)LogDebug, MODULENAME, "init", "");
    m_core->loadPlugins();
}


Core::Core()
{
    //qDebug() << "PropertyEditor::Core CTOR";
}


Core::~Core()
{
    emit log((int)LogDebug, MODULENAME, "DTOR", "");
    //qDebug() << "PropertyEditor::Core DTOR";

    qDeleteAll(m_plugins);
    m_plugins.clear();
}


const QList<PropertyInterface*> & Core::plugins() const
{
    return m_plugins;
}


void Core::loadPlugins()
{
    /// looking for static plugins
    foreach (QObject *pluginObject, QPluginLoader::staticInstances()) {
        PropertyInterface *plugin = qobject_cast<PropertyInterface*>(pluginObject);
        if (plugin) {
            emit log((int)LogDebug, MODULENAME, QString("Found static plugin: %1").arg(plugin->metaObject()->className()), "");
            m_plugins.push_back(qobject_cast<PropertyInterface*>(plugin));
        }
    }

    /// looking for dynamic plugins
    QFileInfoList files;
    QStringList dirs;
    dirs << PROPERTYEDITOR_PLUGINS_PATH;
    dirs << "propertyeditor_plugins";


    QDir dir;
    foreach (const QString & dirStr, dirs) {
        dir.setPath(QDir::isRelativePath(dirStr) ? QCoreApplication::applicationDirPath() + "/" + dirStr : dirStr);
        emit log((int)LogDebug, MODULENAME, "Raw Plugin dir: " + dirStr, "" );
        emit log((int)LogDebug, MODULENAME, "Plugin dir: " + dir.absolutePath(), "");
        files += dir.entryInfoList(QStringList() << "*.so" << "*.dll" << "*.dylib", QDir::Files);
    }

    QPluginLoader loader;
    loader.setLoadHints(QLibrary::ResolveAllSymbolsHint|QLibrary::ExportExternalSymbolsHint);
    foreach(const QFileInfo & fileName, files)
    {
        loader.setFileName( fileName.absoluteFilePath());
        if (!loader.load()) {
            emit log((int)LogWarning, MODULENAME, "Error while loading plugin " + fileName.fileName() + ": " + loader.errorString(), "");
            continue;
        }
        PropertyInterface *plugin = qobject_cast<PropertyInterface*>(loader.instance());
        if (plugin) {
            m_plugins.push_back(plugin);
            emit log((int)LogWarning, MODULENAME, "Loading plugin: " + fileName.baseName(), "");
        } else {
            loader.unload();
            emit log((int)LogWarning, MODULENAME, QString("It\'s not a PropertyEditor plugin: %1").arg(fileName.baseName()), "");
        }
    }
}





}
