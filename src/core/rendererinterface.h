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
#ifndef RENDERERINTERFACE_H
#define RENDERERINTERFACE_H

#include "reportplugininterface.h"
//#include <qscriptengine.h>

#include <QObject>
#include <QGraphicsItem>
#include <QtCore>
#include <QWidget>

namespace CuteReport
{

class ReportInterface;
class RenderedPageInterface;
class RenderedReportInterface;
class RendererHelperInterface;

class CUTEREPORT_EXPORTS RendererInterface : public ReportPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CuteReport::ReportPluginInterface)

//    Q_PROPERTY(int dpi READ dpi WRITE setDpi NOTIFY dpiChanged)

    Q_PROPERTY(int _current_property READ _currentProperty WRITE _setCurrentProperty DESIGNABLE false STORED false)
    Q_PROPERTY(QString _current_property_description READ _current_property_description DESIGNABLE false STORED false)

public:
    // TODO
//    enum DataType {DataVariable, DataFunction, DataDataset};
//    Q_DECLARE_FLAGS(DataTypes, DataType)
    explicit RendererInterface(QObject *parent = 0);
    virtual ~RendererInterface();

    virtual RendererInterface * createInstance(QObject * parent = 0) const = 0;
    virtual RendererInterface * clone() const = 0;
    virtual CuteReport::RendererHelperInterface * helper() {return 0;}

    virtual void run(ReportInterface* report, ThreadingLevel threading = ThreadNo) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() = 0;

    virtual ScriptString processString(ScriptString & scriptString) = 0;

    virtual QSet<QString> reportParameters(CuteReport::ReportInterface *report) = 0;

    virtual ReportInterface * report() = 0;

//    virtual void setDpi(int dpi) = 0;
//    virtual int dpi() const = 0;

    virtual CuteReport::RenderedReportInterface * takeRenderedReport() = 0;

    virtual void _setCurrentProperty(int num) { m_currentProperty = num; }
    virtual int _currentProperty() { return m_currentProperty; }
    virtual QString _current_property_description() const;

    virtual QAbstractItemModel * createFunctionsModel() {return 0;}
    virtual QAbstractItemModel * createVariablesModel() {return 0;}

signals:
    void started();
    void done(bool errorsFound);
    void cancelled();
    void processingPage(int page, int total, int pass, int passTotal);
    void dpiChanged(int);

protected:
    explicit RendererInterface(const RendererInterface &dd, QObject * parent);

    friend class RenderedReportInterface;

    int m_currentProperty;
};


class CUTEREPORT_EXPORTS RendererHelperInterface: public QWidget
{
    Q_OBJECT
public:
    enum VisibleOptions{ReportsOptions, ObjectsOptions, AllOptions};
    RendererHelperInterface(RendererInterface *, VisibleOptions = AllOptions){}
    virtual void load() = 0;
    virtual void save() = 0;
};



} //namespace

Q_DECLARE_INTERFACE(CuteReport::RendererInterface, "CuteReport.RendererInterface/1.0")




#endif // RENDERERINTERFACE_H
