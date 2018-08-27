!include( ../../../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

!include( ../app.pri ) {
    error( Can not find the app.pri file! )
}


#QT       += core

TARGET = cutereport_cli

#CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    console.cpp

HEADERS += \
    console.h


contains(DEFINES, SYSTEMINSTALL) {
    DESTDIR = ../../../$$BUILD_DIR
    target.path += $$REPORT_BINARIES_PATH
    INSTALLS += target
} else {
    DESTDIR = ../../../$$BUILD_DIR/$$REPORT_BINARIES_PATH
}
