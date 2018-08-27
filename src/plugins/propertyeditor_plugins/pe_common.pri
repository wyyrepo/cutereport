! include( ../../../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

TEMPLATE = lib

INCLUDEPATH += ../../../thirdparty/propertyeditor/lib \
        ../../../core

DEPENDPATH=$$INCLUDEPATH

CONFIG += plugin

LIBS += -L../../../../$$BUILD_DIR -L../../../../$$BUILD_DIR/$$REPORT_LIBS_PATH
win32: CONFIG(debug, debug|release): LIBS += -lCuteReportCored -lPropertyEditord
else: LIBS += -lCuteReportCore -lPropertyEditor

contains(DEFINES, SYSTEMINSTALL) {
    DESTDIR = ../../../../$$BUILD_DIR
    target.path += $$PROPERTYEDITOR_PLUGINS_PATH
    INSTALLS += target
} else {
    DESTDIR = ../../../../$$BUILD_DIR/$$PROPERTYEDITOR_PLUGINS_PATH
}
