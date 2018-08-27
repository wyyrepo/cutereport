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
#include "filesystem.h"
#include "reportcore.h"
#include "filesystemstoragehelper.h"

#include <QtGui>

inline void initMyResource() { Q_INIT_RESOURCE(filesystem); }

using namespace CuteReport;

QMap<QString, QString> StorageFileSystem::m_urlHints;

StorageFileSystem::StorageFileSystem(QObject * parent)
    : StorageInterface(parent),
      m_askForOverwrite(true),
      m_rootRelative(false)
{
    m_objectsRootPath = "/";
    m_localDefaultPath = "/";

    //    m_objectsPath = QString(REPORT_EXAMPLES_PATH);
    ////    m_localDefaultPath = QString(REPORT_EXAMPLES_PATH) + "/reports";
    //    m_localDefaultPath = QString(QDir::homePath() + "/" + REPORT_VARS_PATH + "/file_storage/");
    //    m_localDefaultPath.replace(QRegExp("/+"), "/");
    //    if (!QString(REPORT_VARS_PATH).isEmpty()) {
    //        QDir dir(m_localDefaultPath);
    //        if (!dir.exists())
    //            dir.mkpath(m_localDefaultPath);
    //    }

    if (m_urlHints.isEmpty()) {
        initHints();
    }
}


StorageFileSystem::~StorageFileSystem()
{
//    delete m_helper;
}


bool StorageFileSystem::moduleInit()
{
    initMyResource();
    return true;
}


void StorageFileSystem::initHints()
{
    m_urlHints.clear();

    QFile file(":/urlHints.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        reportCore()->log(LogError, ModuleName, "Hints are not recognized");
        return;
    }

    QTextStream in(&file);
    int linename = 0;
    while (!in.atEnd()) {
        linename++;
        QString line = in.readLine();
        if (line.left(1) == QString("#"))   // commented out
            continue;
        QString key = line.section("|",0,0).trimmed();
        QString value  = line.section("|", 1,1).trimmed();

        m_urlHints.insert(key, value);
    }
}


StorageFileSystem * StorageFileSystem::createInstance(QObject * parent) const
{
    return new StorageFileSystem(parent);
}


StorageFileSystem * StorageFileSystem::clone()
{
    return new StorageFileSystem(*this);
}


QString StorageFileSystem::urlScheme() const
{
    return "file";
}


QString StorageFileSystem::urlHint(const QString & key)
{
    if (m_urlHints.contains(key))
        return urlScheme() + ":" + m_urlHints.value(key);

    QFileInfo info(key);
//    qDebug() << info.absolutePath();
    return QString();
}


QStringList StorageFileSystem::urlHints()
{
    return m_urlHints.keys();
}


QString StorageFileSystem::localCachedFileName(const QString & url)
{
    QString path = urlToLocal(url);
#ifdef WIN32
    path.replace(QRegExp("^/(\\w)/"),"\\1:/");
#endif
    return path;
}


QString StorageFileSystem::normalizedURL(const QString &url)
{
    QString normalized = urlScheme() + ":" + urlToLocal(url);
    return normalized;
}


bool StorageFileSystem::saveObject(const QString &url, const QByteArray &objectData)
{
    QString absoluteFilePath = urlToLocal(url);
    if (absoluteFilePath.isEmpty())
        return false;

#ifdef WIN32
    absoluteFilePath.replace(QRegExp("^/(\\w)/"),"\\1:/");
#endif

    QFile file(absoluteFilePath);

    if (file.open(QIODevice::WriteOnly)){
        file.write(objectData);
        file.close();
        return true;
    } else {
        m_lastError = QString("File \'%1\' cannot be created: local path is \'%2\'").arg(url).arg(absoluteFilePath);
        return false;
    }
}


QByteArray StorageFileSystem::loadObject(const QString & url)
{
    reportCore()->log(CuteReport::LogDebug, "StorageFileSystem", "loadObject", QString("url: \'%1\'").arg(url));
    QString absoluteFilePath = urlToLocal(url);
    if (absoluteFilePath.isEmpty())
        return QByteArray();

#ifdef WIN32
    absoluteFilePath.replace(QRegExp("^/(\\w)/"),"\\1:/"); // format back to format "disk:/path" from "/disk/path"
#endif

    QFile file(absoluteFilePath);

    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("File \'%1\' cannot be opened: local path is \'%2\'").arg(url).arg(absoluteFilePath);
        return QByteArray();
    }

    QByteArray ba(file.readAll());
    file.close();

    return ba;
}


QList<StorageObjectInfo> StorageFileSystem::objectsList(const QString & url, const QStringList & nameFilters,
                                                        QDir::Filters filters, QDir::SortFlags sort, bool * ok)
{
    QList<StorageObjectInfo> list;

    QString absoluteFilePath = urlToLocal(url);
    if (absoluteFilePath.isEmpty()) {
        if (ok)
            *ok = false;
        return list;
    }

    QString root = urlToLocal("");
    if (!absoluteFilePath.startsWith(root)) {
        m_lastError = "Url is out of root";
        if (ok)
            *ok = false;
        return list;
    }

#ifdef WIN32
    QString winDir = absoluteFilePath;
    winDir.replace(QRegExp("^/(\\w)/"),"\\1:/");
    QDir dir (winDir);
#else
    QDir dir (absoluteFilePath);
#endif


    if (!dir.exists()) {
        m_lastError = "Url path does not exist";
        if (ok)
            *ok = false;
        return list;
    }

    QFileInfoList infoList;
    bool doProcess = true;

#ifdef WIN32
    if (absoluteFilePath == "/") {
        doProcess = false;
        if (filters.testFlag(QDir::Dirs)) {
            infoList = QDir::drives();
            foreach (const QFileInfo &fileInfo, infoList) {
                QString driveName = fileInfo.absoluteFilePath();
                driveName.remove(":/");
                StorageObjectInfo objectInfo;
                objectInfo.name = driveName + ":";
                objectInfo.url = urlScheme() + ":/" + driveName + "/";
                objectInfo.size = fileInfo.size();
                objectInfo.type = FileDrive ;

                list.append(objectInfo);
            }
        }
    }
#endif

    if (doProcess) {
        infoList = dir.entryInfoList(nameFilters, filters | QDir::NoDotAndDotDot, sort);

        foreach (const QFileInfo &fileInfo, infoList) {
            StorageObjectInfo objectInfo;
            objectInfo.name = fileInfo.isDir() ? fileInfo.absoluteFilePath().section("/",-1,-1) : fileInfo.absoluteFilePath();
            objectInfo.url = urlScheme() + ":" + pathCutOff(fileInfo.absoluteFilePath());
            objectInfo.size = fileInfo.size();
            objectInfo.type = fileInfo.isDir() ? FileDir : FileUnknown;

            //        QString ext = fileInfo.suffix();
            //        if (ext.indexOf(QRegExp("([Jj][Pp][eE]?[gG])|([Pp][Nn][gG]|[Bb][Mm][Pp])|([Ii][Cc][Oo])")) == 0)
            //            objectInfo.type = FileImage;

            list.append(objectInfo);
        }
    }

    return list;
}


QList<StorageObjectInfo> StorageFileSystem::objectsList(const QString & url, bool *ok)
{
    return objectsList(url, QStringList(), QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot,
                       QDir::Name | QDir::DirsFirst, ok );
}


bool StorageFileSystem::objectExists(const QString & url)
{
    QString absoluteFilePath = urlToLocal(url);
    if (absoluteFilePath.isEmpty())
        return false;

    QString root = urlToLocal("");
    if (!absoluteFilePath.startsWith(root))
        return false;

    QFileInfo file (absoluteFilePath);

    return file.exists();
}


QString StorageFileSystem::urlToLocal(const QString &url)
{
    QUrl u(url);
    QString urlScheme = u.scheme();
    QString path = u.path();

#ifdef WIN32
    if (urlScheme.size() == 1) { // windows disk label (so path is absolute)
        path = "/" + urlScheme +  path;
        return path;
    } else { /// format /c/path
        path.replace(QRegExp("^/(\\w)/"),"\\1:/");
    }
#endif

    reportCore()->log(CuteReport::LogDebug, "ReportCore", "urlToLocal", QString("urlScheme: \'%1\' path: \'%2\'").arg(url, path));
//    if (!urlScheme.isEmpty() && urlScheme != this->urlScheme() ) {
//        m_lastError = QString("Not appropriate scheme \'%1\' for storage scheme \'%2\'").arg(urlScheme).arg(this->urlScheme());
//        return QString();
//    }

    QFileInfo fileInfo(path);

    QString absoluteFilePath;

    if (!fileInfo.isAbsolute()) {
        reportCore()->log(CuteReport::LogDebug, "ReportCore", "urlToLocal", QString("not absolute path"));
        if (m_objectsRootPath.isEmpty() || m_objectsRootPath == "/") {
            absoluteFilePath = (m_rootRelative ? m_localDefaultPath : QCoreApplication::applicationDirPath()) + "/" + fileInfo.filePath();
        } else {
            QFileInfo fInfo(m_objectsRootPath);
            absoluteFilePath = (fInfo.isAbsolute() ? "" : m_localDefaultPath + "/") + m_objectsRootPath + "/" + fileInfo.filePath();
        }
    } else {
        reportCore()->log(CuteReport::LogDebug, "ReportCore", "urlToLocal", QString("absolute path"));
        absoluteFilePath = fileInfo.absoluteFilePath();
    }

    absoluteFilePath.replace(QRegExp("/+"), "/");

#ifdef WIN32
    absoluteFilePath.replace(QRegExp("^(\\w):/"),"/\\1/");
#endif

    reportCore()->log(CuteReport::LogDebug, "ReportCore", "urlToLocal", QString("absoluteFilePath: \'%1\'").arg(absoluteFilePath));

    return absoluteFilePath;
}


QString StorageFileSystem::pathCutOff(const QString & absPath)
{
    QString _absPath = absPath;
#ifdef WIN32
    _absPath.replace(QRegExp("^(\\w):/"),"/\\1/");
#endif
    QFileInfo pathIn(_absPath);
    QFileInfo path(urlToLocal(""));

    QString resultFileName;

    if (pathIn.absoluteFilePath().startsWith(path.absolutePath())) {
        resultFileName = pathIn.absoluteFilePath().remove(0, path.absolutePath().size());
        if (resultFileName[0] == '/')
            resultFileName.remove(0,1);
    } else
        resultFileName =  absPath;

    return resultFileName;
}


QString StorageFileSystem::lastError() const
{
    return m_lastError;
}



CuteReport::StorageHelperInterface * StorageFileSystem::createHelper(CuteDesigner::Core * designer)
{
//    if (!m_helper)
//        m_helper = new FileSystemStorageHelper(this);
//    return m_helper;
    return new FileSystemStorageHelper(this, designer);
}


void StorageFileSystem::setObjectsRootPath(const QString & path)
{
    if (path == m_objectsRootPath)
        return;

    m_objectsRootPath = path;

    emit objectsRootPathChanged(m_objectsRootPath);
    emit changed();
}


const QString & StorageFileSystem::objectsRootPath() const
{
    return m_objectsRootPath;
}


bool StorageFileSystem::rootRelative()
{
    return m_rootRelative;
}


void StorageFileSystem::setRootRelative(bool b)
{
    if (b == m_rootRelative)
        return;

    m_rootRelative = b;

    emit rootRelativeChanged(m_rootRelative);
    emit changed();

}


bool StorageFileSystem::askForOverwrite()
{
    return m_askForOverwrite;
}


void StorageFileSystem::setAskForOverwrite(bool b)
{
    if (b == m_askForOverwrite)
        return;

    m_askForOverwrite = b;

    emit askForOverwriteChanged(m_askForOverwrite);
    emit changed();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(StorageFileSystem, StorageFileSystem)
#endif
