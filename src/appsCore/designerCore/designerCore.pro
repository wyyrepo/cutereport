! include( ../../../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

TARGET = CuteDesigner
TEMPLATE = lib

SOURCES += \
    moduleinterface.cpp \
    mainwindow.cpp \
    aboutdialog.cpp \
    storagesettingsdialog.cpp \
    optionsdialog.cpp \
    optionsdialogstoragepage.cpp \
    widgets/messagewidget.cpp \
    widgets/messagelist.cpp \
    widgets/messagemodel.cpp \
    widgets/messagedelegate.cpp \
    widgets/renamedialog.cpp \
    optionsdialogrendererpage.cpp \
    optionsdialogprinterpage.cpp \
    designeriteminterfaceobject.cpp \
    designercore.cpp \
    optionsdialogdesigner.cpp

HEADERS  += $$PWD/../../core/cutereport_globals.h \
    moduleinterface.h \
    mainwindow.h \
    aboutdialog.h \
    storagesettingsdialog.h \
    optionsdialog.h \
    optionsdialogstoragepage.h \
    designer_globals.h \
    widgets/messagewidget.h \
    widgets/messagelist.h \
    widgets/messagemodel.h \
    widgets/messagedelegate.h \
    widgets/renamedialog.h \
    optionsdialogrendererpage.h \
    optionsdialogprinterpage.h \
    designeriteminterfaceobject.h \
    designercore.h \
    optionsdialogdesigner.h

RESOURCES += designerCore.qrc

FORMS += mainwindow.ui \
         aboutdialog.ui \
         storagesettingsdialog.ui \
         optionsdialog.ui \
         optionsdialogstoragepage.ui \
         widgets/messagelist.ui \
         widgets/renamedialog.ui \
         optionsdialogrendererpage.ui \
         optionsdialogprinterpage.ui \
        optionsdialogdesigner.ui \


INCLUDEPATH += ../../core \
            ../../widgets/widgets \
            ../../widgets/widgets_thirdparty \
            ../../_common \
            ../../core/log \
            ../../thirdparty/propertyeditor/lib  \
            ../designer/ \
            widgets \
            widgets/objectinspector

DEPENDPATH += $$INCLUDEPATH


LIBS += -L$$OUT_PWD/../../../$$BUILD_DIR -L$$OUT_PWD/../../../$$BUILD_DIR/$$REPORT_DESIGNER_LIBS_PATH
win32: CONFIG(debug, debug|release): LIBS += -lPropertyEditord -lCuteReportCored -lCuteReportWidgetsd -lCuteReportWidgetsThirdpartyd
else: LIBS += -lPropertyEditor -lCuteReportCore -lCuteReportWidgets -lCuteReportWidgetsThirdparty

#contains(DEFINES, PRO) {
#    SOURCES += $$PWD/../../../addons_pro/plugins/common/functions_ext.cpp
#    HEADERS += $$PWD/../../../addons_pro/plugins/common/functions_ext.h
#}

contains(DEFINES, SYSTEMINSTALL) {
    DESTDIR = ../../../$$BUILD_DIR
    !contains(DEFINES, STATIC_DESIGNER) {
        target.path += $$REPORT_DESIGNER_LIBS_PATH
        INSTALLS += target
    }
} else {
    DESTDIR = ../../../$$BUILD_DIR/$$REPORT_DESIGNER_LIBS_PATH
}


contains(DEFINES, STATIC_DESIGNER) {
    CONFIG += static
} else {
    CONFIG += shared
    win32: TARGET_EXT = .dll
}

win32 {
    DEFINES += CUTEREPORT_DESIGNER_EXPORTS
    CONFIG(debug, debug|release): TARGET = $$join(TARGET,,,d)
}



