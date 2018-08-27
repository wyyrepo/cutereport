!include( ../../../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

!include( ../app.pri ) {
    error( Can not find the app.pri file! )
}

macx {
    CONFIG += app_bundle
}

TARGET = cutereport
TEMPLATE = app

SOURCES += main.cpp \

HEADERS  += \

contains(DEFINES, SYSTEMINSTALL) {
    DESTDIR = ../../../$$BUILD_DIR
    target.path = $$REPORT_DESIGNER_PATH
    INSTALLS += target
} else {
    DESTDIR = ../../../$$BUILD_DIR/$$REPORT_DESIGNER_PATH
}
