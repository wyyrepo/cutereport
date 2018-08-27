! include( ../../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

include (log/log.pri)

DEPENDPATH += $$INCLUDEPATH

lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG += uitools dll
} else {
    QT += widgets
}

TARGET = CuteReportCore
TEMPLATE =  lib

HEADERS += \
    cutereport_globals.h \
    reportcore.h \
    pageinterface.h \
    datasetinterface.h \
    reportplugininterface.h \
    reportinterface.h \
    cutereport_types.h \
    baseiteminterface.h \
    baseiteminterface_p.h \
    itemexternaldatainterface.h \
    iteminterface.h \
    iteminterface_p.h \
    bandinterface.h \
    bandinterface_p.h \
    storageinterface.h \
    serializerinterface.h \
    rendererinterface.h \
    rendererpublicinterface.h \
    item_common/simplerendereditem.h \
    item_common/simpleitemview.h \
    renderediteminterface.h \
    renderediteminterface_p.h \
    iteminterfaceview.h \
    printerinterface.h \
    exportinterface.h \
    objectfactory.h \
    forminterface.h \
    cutereport.h \
    cutereport_functions.h \
    dummyitem.h \
    dummyband.h \
    renderedreport.h \
    renderedreportinterface.h \
    scriptextensioninterface.h \
    importinterface.h \
    designeriteminterface.h \
    scriptengineinterface.h \
    scriptdelegate.h \
    inheritancedatacontainer.h \
    reportcoreconfigcleanup.h \
    abstractpainterdelegate.h

SOURCES += \
    reportcore.cpp \
    pageinterface.cpp \
    datasetinterface.cpp \
    reportplugininterface.cpp \
    reportinterface.cpp \
    cutereport_types.cpp \
    baseiteminterface.cpp \
    itemexternaldatainterface.cpp \
    iteminterface.cpp \
    bandinterface.cpp \
    storageinterface.cpp \
    serializerinterface.cpp \
    rendererinterface.cpp \
    rendererpublicinterface.cpp \
    item_common/simplerendereditem.cpp \
    item_common/simpleitemview.cpp \
    renderediteminterface.cpp \
    iteminterfaceview.cpp \
    printerinterface.cpp \
    exportinterface.cpp \
    objectfactory.cpp \
    forminterface.cpp \
    cutereport.cpp \
    cutereport_functions.cpp \
    dummyitem.cpp \
    dummyband.cpp \
    renderedreport.cpp \
    renderedreportinterface.cpp \
    scriptextensioninterface.cpp \
    importinterface.cpp \
    designeriteminterface.cpp \
    scriptengineinterface.cpp \
    scriptdelegate.cpp \
    inheritancedatacontainer.cpp \
    reportcoreconfigcleanup.cpp \
    abstractpainterdelegate.cpp


PRIVATE_HEADERS.files = reportcore.h reportinterface.h datasetinterface.h bandinterface.h iteminterface.h \
                        baseiteminterface.h exportinterface.h forminterface.h printerinterface.h \
                        rendererinterface.h pageinterface.h storageinterface.h serializerinterface.h \
                        scriptextensioninterface.h cutereport_globals.h cutereport_types.h cutereport_functions.h \
                        reportplugininterface.h abstractpainterdelegate.h scriptdelegate.h
#                        iteminterfaceview.h renderediteminterface.h


INSTALLS += PRIVATE_HEADERS

contains(DEFINES, SYSTEMINSTALL) {
    DESTDIR = ../../$$BUILD_DIR
    target.path = $$REPORT_LIBS_PATH
    PRIVATE_HEADERS.path = $$REPORT_HEADERS_PATH
    !contains(DEFINES, STATIC_CORE) {
        INSTALLS += target
    }

} else {
    DESTDIR = ../../$$BUILD_DIR/$$REPORT_LIBS_PATH
    PRIVATE_HEADERS.path = $$OUT_PWD/../../$$BUILD_DIR/$$REPORT_HEADERS_PATH
}

contains(DEFINES, STATIC_CORE) {
    CONFIG += static
} else {
    win32: TARGET_EXT = .dll
}

win32 {
    DEFINES += LIB_EXPORTS
    CONFIG(debug, debug|release): TARGET = $$join(TARGET,,,d)
}

RESOURCES += \
    core_resources.qrc
