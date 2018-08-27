!include( $$PWD/../../../../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

CONFIG += plugin
TEMPLATE = lib

INCLUDEPATH += ../../lib/
DEPENDPATH=$$INCLUDEPATH

LIBS += -L../../../../../$$BUILD_DIR -L../../../../../$$BUILD_DIR/$$PROPERTYEDITOR_LIBS_PATH 
win32: CONFIG(debug, debug|release): LIBS += -lPropertyEditord
else: LIBS += -lPropertyEditor

contains(DEFINES, SYSTEMINSTALL) {
    DESTDIR = ../../../../../$$BUILD_DIR/
    target.path += $$PROPERTYEDITOR_PLUGINS_PATH
    INSTALLS += target
} else {
    DESTDIR = ../../../../../$$BUILD_DIR/$$PROPERTYEDITOR_PLUGINS_PATH
}

contains(DEFINES, STATICPLUGINS_PROPERTYEDITOR) {
    CONFIG += static
}
