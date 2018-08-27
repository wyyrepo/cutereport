! include( ../../../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

TARGET = CuteReportWidgetsThirdparty
TEMPLATE = lib

INCLUDEPATH = ../../core \
DEPENDPATH = $$INCLUDEPATH

LIBS += -L../../../$$BUILD_DIR -L../../../$$BUILD_DIR/$$REPORT_LIBS_PATH
win32: CONFIG(debug, debug|release): LIBS += -lCuteReportCored
else: LIBS += -lCuteReportCore

contains(DEFINES, SYSTEMINSTALL) {
    DESTDIR = ../../../$$BUILD_DIR
    target.path += $$REPORT_LIBS_PATH
    INSTALLS += target
} else {
    DESTDIR = ../../../$$BUILD_DIR/$$REPORT_LIBS_PATH
}

HEADERS += \
    qt_blurimage.h \
    fancytabs.h \
    stylehelper.h \
    objectinspector.h \
    objectmodel.h \
    \
    qscriptsyntaxhighlighter_p.h \
    qscriptedit_p.h \
    qfunctions_p.h \

SOURCES += \
    fancytabs.cpp \
    stylehelper.cpp \
    objectinspector.cpp \
    objectmodel.cpp \
    \
    qscriptsyntaxhighlighter.cpp \
    qscriptedit.cpp \

win32 : TARGET_EXT = .dll

win32 {
    DEFINES += CUTEREPORT_WIDGET_THIRDPARTY_EXPORTS
    CONFIG(debug, debug|release): TARGET = $$join(TARGET,,,d)
}


# workaround for Qt builds without QtTest module
# required for original Nokia files
DEFINES += Q_AUTOTEST_EXPORT=\\\"\\\"
