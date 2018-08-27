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
#ifndef REPORTCORE_H
#define REPORTCORE_H

#include <QObject>
#include <QMap>
#include "cutereport_globals.h"
#include "cutereport_types.h"


namespace CuteReport
{

class DatasetInterface;
class PageInterface;
class BaseItemInterface;
class StorageInterface;
class StorageOptionsInterface;
class RendererInterface;
class RenderedPageInterface;
class PrinterInterface;
class FormInterface;
class SerializerInterface;
class ImportInterface;
class ExportInterface;
class Log;
class ReportInterface;
class RenderedReportInterface;
class ReportPluginInterface;
class ScriptExtensionInterface;
class DesignerItemInterface;

enum BaseType {BasedOnTemplate, BasedOnReport};
enum RenderingDestination { RenderToPreview, RenderToExport, RenderToPrinter};

struct QueueReport {
    explicit QueueReport();
    virtual ~QueueReport();
    ReportInterface * report;
    QString url;
    RenderingDestination destination;
    QString rendererName;
    QString destinationName;
    bool success;
};


struct QueueReportExport: public QueueReport {
    QString format;
    QString outputUrl;
    QStringList options;
};


class CUTEREPORT_EXPORTS ReportCore : public QObject
{
    Q_OBJECT

public:
    explicit ReportCore(QObject *parent = 0, QSettings *settings = 0, bool interactive = true, bool initLogSytem = true);
    explicit ReportCore(QSettings *settings, bool interactive = true);
    ~ReportCore();

    int versionMajor() const;
    int versionMinor() const;
    QString license() const;

    /*! determine if report objects can be changed or no
     * If it is set to true usually it means plugins will show theirs dialogs
     * internal reports signal will be connected to look for any property changes, etc.
     * This property is set to true by default
     * You can set it to false to slighly resource economy if you do report processing in background
     * without any report template changing in runtime and without any user action
     */
    bool interactive() const;

    /*! syncing all storages and other data with remote or local systems
     *  emits finished() signal
     * do not operate with ReportCore while you have no finished() signal
     * read meassage() for have information;
    */
    void finish();

    /*! restoreState method reads settings and restores all configured objects.
     * If removeCurrentState is "true" then all current objects will be removed
     * and only objects defined in Settings will be added. Otherwize all objects defined
     * in the setting will be updated and objects not defined will be left as is.
     * This method is called automatically if "AutoRestore" setting value is "true".
     * When "AutoRestore" is defined it read value of "CleanRestore" that define value
     * of "cleanRestore" parameter.
     * "CleanRestore" is true by default.
     *  storeState method saves setting of all objects and default names.
     * This method is called automatically if "AutoStore" setting value is "true".
     * Default for "AutoStore" if not defined is "false"
     * Default for "AutoRestore" if not definded is "true".
     **/
    void restoreState(bool cleanRestore = true);
    void storeState();

    /*! Method set the name of default storage for storage requests where url scheme is not present.
     * storageName should be one of the Core's storages
     * Returns QString::null if storage is not defined
    */
    bool setDefaultStorage(const QString &objectName);
    QString defaultStorageName() const;

    bool setDefaultPrinter(const QString objectName);
    QString defaultPrinterName() const;

    bool setDefaultRenderer(const QString &objectName);
    QString defaultRendererName() const;

    /** Root widget where all dialogs will be centered on
     *  usually it's a main window
     **/
    void setRootWidget(QWidget * widget);
    QWidget * rootWidget();

    /*! These methods return a list of appropriate plugins.
     * These methods are not designed to change any default plugins and used only to get service information
     * like plugin name, version, etc.
     * It is primary used in designing tools
     **/
    const QList<ReportPluginInterface *> modules(ModuleType moduleType) const;

    /*! returns module by module name.
     * If name is not specified, first appropriate module is returned
     **/
    const ReportPluginInterface * module(ModuleType moduleType, const QString &moduleName) const;

    /*! returns a list of module names for appropriate module type */
    QStringList moduleNames(ModuleType moduleType, bool shortName = false) const;

    /*! converts module type to/from string */
    QString moduleTypeToString(ModuleType type) const;
    ModuleType moduleTypeFromString(const QString & type) const;

    /*! moduleOptions returns module object's property list.
     * Each string is in format propertyName=propertyValue
     * The list can include object name if "includeObjectName" set to true
     * setModuleOptions restores module object properties from string list
     * moduleOptionsStr and setModuleOptionsStr do the same work but using string instead of string list where eacg row is joined using
     * defined delimiter.Delimiter can not be defined space.
     * Delimiter contains 1 character and it is saved on the first position of the result string.
     * By default delimiter is "," (comma)
     **/
    QStringList moduleOptions(ReportPluginInterface * module, bool includeObjectName = true);
    void setModuleOptions(ReportPluginInterface * module, const QStringList & options, bool setObjectNameIfDefined = true);
    QString moduleOptionsStr(ReportPluginInterface * module, bool includeObjectName = true, const QString & delimiter = QString());
    void setModuleOptionsStr(ReportPluginInterface * module, const QString & options, bool setObjectNameIfDefined = true);

    /*! setStorage will replace current one with the same objectName if exists
     */
    QList<StorageInterface *>       storageList(ReportInterface * report = 0) const;
    QStringList                     storageNameList(ReportInterface * report = 0) const;
    StorageInterface *              storage(const QString & objectName, ReportInterface * report = 0) const;
    StorageInterface *              storageByUrl(const QString & urlString, ReportInterface * report = 0) const;
    void                            setStorage(StorageInterface * storage);
    void                            deleteStorage(const QString & objectName);

    QList<RendererInterface *>      rendererList(ReportInterface * report = 0) const;
    QStringList                     rendererNameList(ReportInterface * report = 0) const;
    RendererInterface *             renderer(const QString &objectName) const;
    RendererInterface *             renderer(ReportInterface * report, const QString &objectName = QString()) const;
    void                            setRenderer(RendererInterface * renderer);
    void                            deleteRenderer(const QString & objectName);

    QList<PrinterInterface *>       printerList(ReportInterface * report = 0) const;
    QStringList                     printerNameList(ReportInterface * report = 0) const;
    PrinterInterface *              printer(const QString & objectName, ReportInterface * report = 0) const;
    void                            setPrinter(PrinterInterface * printer);
    void                            deletePrinter(const QString & objectName);

    PageInterface *     pageByName(const QString & pageName, CuteReport::ReportInterface * report) const;
    BaseItemInterface * itemByName(const QString & itemName, CuteReport::PageInterface * page = 0) const; //if page is 0, look at all pages
    BaseItemInterface * itemByName(const QString & itemName, const QString & pageName, CuteReport::ReportInterface * report) const;
    DatasetInterface *  datasetByName(const QString & datasetName, CuteReport::ReportInterface * report) const;
    FormInterface *     formByName(const QString & formName, CuteReport::ReportInterface * report) const;

    QList<ReportInterface *>        reports() const;
    CuteReport::ReportInterface *   createReport();
    void                            deleteReport(CuteReport::ReportInterface * report);

    /*! Below are methods to create a new object of specified module name.
     * if moduleName is empty they will create an object of the first accessible module.
     * It does NOT join object to report.
     **/

    CuteReport::PageInterface *     createPageObject(const QString & moduleName, CuteReport::ReportInterface *report = 0);
    CuteReport::DatasetInterface *  createDatasetObject(const QString & moduleName, CuteReport::ReportInterface *report = 0);
    CuteReport::BaseItemInterface * createItemObject(const QString & moduleName, CuteReport::ReportInterface *report = 0, QObject * parent = 0);
    CuteReport::PrinterInterface *  createPrinterObject(const QString & moduleName, CuteReport::ReportInterface *report = 0);
    CuteReport::StorageInterface *  createStorageObject(const QString & moduleName, CuteReport::ReportInterface *report = 0);
    CuteReport::RendererInterface * createRendererObject(const QString & moduleName, CuteReport::ReportInterface *report = 0);
    CuteReport::FormInterface *     createFormObject(const QString & moduleName, CuteReport::ReportInterface *report = 0);
    CuteReport::ExportInterface *   createExportObject(const QString & moduleName, CuteReport::ReportInterface *report = 0);
    ScriptExtensionInterface *      createScriptExtentionObject(const QString & moduleName, CuteReport::ReportInterface *report = 0, QObject * parent = 0);

    QByteArray  serialize(const QObject * object, bool *ok = 0, QString * error = 0, const QString & moduleName = QString());
    QObject *   deserialize(const QByteArray &data, bool *ok = 0, QString * error = 0, const QString & moduleName = QString());


    bool saveReport(const QString & urlString, CuteReport::ReportInterface * report, QString * errorText = 0);

    bool reportExists(const QString & urlString);

    CuteReport::ReportInterface * loadReport(const QString & urlString,
                                             QString * errorText = 0);

    bool        saveObject(const QString & urlString,
                            CuteReport::ReportInterface * report,
                            const QByteArray &objectData,
                            QString * errorText = 0);

    QByteArray    loadObject(const QString & urlString,
                              CuteReport::ReportInterface * report,
                              QString * errorText = 0);

    QString localCachedFileName(const QString & url, CuteReport::ReportInterface * report);
    QList<StorageObjectInfo> objectsList(const QString & url, CuteReport::ReportInterface * report, QString *errorText = 0);
    QList<StorageObjectInfo> objectsList(const QString & url, ReportInterface *report, const QStringList & nameFilters, QDir::Filters filters,
                                                 QDir::SortFlags sort, QString *errorText = 0);

    /*! Renderer methods */
//    const QList<QString> renderers();
    bool render(ReportInterface* report, const QString &rendererName = QString());
    bool render(const QString & reportUrl, const QString &rendererName = QString());
    void stopRendering(ReportInterface *report, bool silent = false); // silent = true if we don't know if report rendering or not, to prevent warnings
    bool isRendered(ReportInterface *report);
    int rendererTotalPages(ReportInterface *report) const;
    RenderedPageInterface * rendererGetPage(ReportInterface *report, int number) const;
    void renderDataClear(ReportInterface * report);
    void renderDataclearAll();

    /*! Printer methods */
    void print(ReportInterface* report, const QString & printerName = QString());
    void print(const QString url, const QString & printerName = QString());

    /*! Import methods */
    QStringList importExtensions() const;
    bool canImport(const QString &reportUrl) const;
    QStringList importModulesForFile(const QString &reportUrl) const;
    ReportInterface *import(const QString &reportUrl, const QString & moduleName = QString()) const;

    /*! Export methods
    options is in format parameter=value;parameter2=value2
    space symbol is allowed */
    void exportTo(ReportInterface* report, const QString & format, const QString & outputUrl = QString(), const QStringList & options = QStringList());
    void exportTo(const QString &reportUrl, const QString & format, const QString & outputUrl = QString(), const QStringList & options = QStringList());

    static void log(LogLevel level, const QString & sender, const QString & message);
    static void log(LogLevel level, const QString & sender, const QString & shortMessage, const QString & fullMessage);

    static bool isNameUnique(QObject *object, const QString &name, QObject * rootObject);
    static QString uniqueName(QObject * object, const QString &proposedName, QObject *rootObject);

    int maxRenderingThreads() const;
    void setMaxRenderingThreads(int maxRenderingThreads);

    ScriptString processString(ScriptString & scriptString);
    ScriptString processString(const ScriptString &scriptString);

    QSet<QString> getReportParameters(ReportInterface *report, QString *error = 0);

    QSettings * settings();

    /** always returns path without ending slash */
    QString resourcesPath() const;
    QString imagesPath() const;
    QString pluginsPath() const;
    QString settingsPath() const;

    bool inheritReport(ReportInterface * report, const QString & baseReportUrl);
    bool inheritReport(ReportInterface * report, const QString & baseReportFileName, const QString &baseReportPath);
    bool detachReport(ReportInterface * report, bool ownInheritedItems = false);


public slots:
    void sendMetric(CuteReport::MetricType type, const QVariant &value);

signals:
    void finished(bool);
    void syncMessage(const QString & moduleName, const QString & message);

    void rendererStarted(CuteReport::ReportInterface * report);
    void rendererDone(CuteReport::ReportInterface * report, bool success);
    void rendererDone(const QString & reportUrl, bool success);
    void rendererMessage(CuteReport::ReportInterface * report, int logLevel, QString message);
    void rendererProcessingPage(CuteReport::ReportInterface * report, int page, int total, int pass, int passTotal);
    void rendererProcessedPage(CuteReport::ReportInterface * report, CuteReport::RenderedPageInterface * page, int pageNumber);

    void printingDone(CuteReport::ReportInterface * report, bool successfully);
    void printingDone(const QString & reportUrl, bool successfully);

    void exportDone(CuteReport::ReportInterface * report, bool successfully);
    void exportDone(const QString & reportUrl, bool successfully);

    void reportObjectCreated(CuteReport::ReportInterface * report);
    void reportObjectDestroyed(CuteReport::ReportInterface * report);
    void metricUpdated(CuteReport::MetricType type, const QVariant &value);

private slots:
    void _rendererStarted();
    void _rendererDone(bool successful);
    void _rendererMessage(int logLevel, QString message);
    void _rendererProcessingPage(int page, int total, int pass, int passTotal);

    void _reportObjectCreated(CuteReport::ReportInterface * report);

private:
    static bool loadPlugins(QSettings *settings);
    static void processModuleList(QList<ReportPluginInterface *> &list);
    static void fixLoadedReportIssues(ReportInterface * report);
    ReportPluginInterface * getOriginalModuleByName(const QString & moduleName, const QList<ReportPluginInterface *> &list) const;
    ReportPluginInterface * getExtendedModuleByName(const QString & origModuleName, const QList<ReportPluginInterface *> &list) const;
    ReportPluginInterface * getModuleByName(const QString & moduleName, const QList<ReportPluginInterface *> &list);
    void saveObjectsOptions(const QString & prefix, const QList<ReportPluginInterface*> & objects, bool clearPrefix = true);
    void loadObjectsOptions(const QString & prefix, QList<ReportPluginInterface*> & objects, const QList<ReportPluginInterface*> * plugins);
    bool removeObject(const QString &  objectName, QList<ReportPluginInterface*> & objects);
    bool replaceObject(ReportPluginInterface* object, QList<ReportPluginInterface*> & objects, QPointer<ReportPluginInterface> defaultObject);
    QStringList objectNames(const QList<ReportPluginInterface *> &objects) const;
    ReportPluginInterface * getObjectByName(const QString & objectName, const QList<ReportPluginInterface *> &list) const;
    void createLocalObjects(QList<ReportPluginInterface*> & objects, const QList<ReportPluginInterface*> * plugins);
    ReportPluginInterface * findDefaultObject(const QString &type, const QString &fullModuleNameList, QList<ReportPluginInterface*> & objects);
    RendererInterface * getRendererForReport(ReportInterface *report, QString *error);

    void init(QSettings * settings, bool initLogSystem);
    inline bool checkReportPointer(CuteReport::ReportInterface * report, QString * errorText = 0) const;
    bool _render(QueueReport *queueReport);
    void _renderDone(QueueReport *queueReport);
    void _export(QueueReport *queueReport);
    void _exportDone(QueueReport *queueReport);
    void _print(QueueReport *queueReport);
    void _printDone(QueueReport *queueReport);

private:
    static int m_refCount;

    static QList<ReportPluginInterface*> * m_itemPlugins;
    static QList<ReportPluginInterface*> * m_pagePlugins;
    static QList<ReportPluginInterface*> * m_datasetPlugins;
    static QList<ReportPluginInterface*> * m_storagePlugins;
    static QList<ReportPluginInterface*> * m_serializePlugins;
    static QList<ReportPluginInterface*> * m_rendererPlugins;
    static QList<ReportPluginInterface*> * m_printerPlugins;
    static QList<ReportPluginInterface*> * m_formPlugins;
    static QList<ReportPluginInterface*> * m_importPlugins;
    static QList<ReportPluginInterface*> * m_exportPlugins;
    static QList<ReportPluginInterface*> * m_scriptExtensionPlugins;

    QPointer<ReportPluginInterface>   m_defaultStorage;
    QPointer<ReportPluginInterface>   m_defaultRenderer;
    QPointer<ReportPluginInterface>   m_defaultPrinter;
    QPointer<ReportPluginInterface>   m_defaultSerializer;

    QWidget* m_rootWidget;
    QList<ReportPluginInterface*> m_storages;
    QList<ReportPluginInterface*> m_renderers;
    QList<ReportPluginInterface*> m_printers;
    QList<ReportPluginInterface*> m_serializers;
    QList<ReportPluginInterface*> m_exporters;

    QSettings * m_settings;
    bool m_interactive;

    QList<QueueReport*> m_waitingQueue;
    QHash<RendererInterface*, QueueReport*> m_renderingQueue;
    int m_maxRenderingThreads;
};

}
#endif // REPORTCORE_H
