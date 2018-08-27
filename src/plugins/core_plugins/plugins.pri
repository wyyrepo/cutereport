! include( ../../../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

TEMPLATE = lib
CONFIG += plugin

HEADERS += $$PWD/plugins_common.h

INCLUDEPATH +=  ../ \
                ../../ \
                ../../../../core/ \
                ../../../../widgets/widgets/ \
                ../../../../appsCore/designerCore/ \

DEPENDPATH += $$INCLUDEPATH

LIBS += -L../../../../../$$BUILD_DIR -L../../../../../$$BUILD_DIR/$$REPORT_LIBS_PATH
win32: CONFIG(debug, debug|release): LIBS += -lCuteReportCored -lCuteReportWidgetsd -lCuteDesignerd
else: LIBS += -lCuteReportCore -lCuteReportWidgets -lCuteDesigner


contains(DEFINES, SYSTEMINSTALL) {
    DESTDIR = ../../../../../$$BUILD_DIR
    !contains(DEFINES, STATICPLUGINS_CORE) {
        target.path = $$REPORT_PLUGINS_PATH
        INSTALL_HEADERS.path = $$REPORT_HEADERS_PATH
        INSTALLS += target
        INSTALLS += INSTALL_HEADERS
    }
} else {
    DESTDIR = ../../../../../$$BUILD_DIR/$$REPORT_PLUGINS_PATH
    INSTALL_HEADERS.path = $$OUT_PWD/../../../../../$$BUILD_DIR/$$REPORT_HEADERS_PATH
    INSTALLS += INSTALL_HEADERS
}

contains(DEFINES, STATICPLUGINS_CORE) {
    CONFIG += static
}

