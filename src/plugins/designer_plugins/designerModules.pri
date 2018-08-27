! include( ../../../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../../core \
               ../../../widgets/widgets \
               ../../../widgets/widgets_thirdparty \
               ../../../thirdparty/propertyeditor/lib \
               ../../../apps/designer \
               ../../../appsCore/designerCore \
               ../../../appsCore/designerCore/widgets/ \
               ../../../appsCore/designerCore/widgets/objectinspector/ \
               ../../../appsCore/designerCore/widgets/fonteditor/ \

DEPENDPATH += $$INCLUDEPATH

LIBS += -L../../../../$$BUILD_DIR -L../../../../$$BUILD_DIR/$$REPORT_LIBS_PATH
win32: CONFIG(debug, debug|release): LIBS += -lCuteReportCored -lPropertyEditord -lCuteDesignerd -lCuteReportWidgetsd -lCuteReportWidgetsThirdpartyd
else: LIBS += -lCuteReportCore -lPropertyEditor -lCuteDesigner -lCuteReportWidgets -lCuteReportWidgetsThirdparty

contains(DEFINES, SYSTEMINSTALL) {
    DESTDIR = ../../../../$$BUILD_DIR
    !contains(DEFINES, STATICPLUGINS_CORE) {
        target.path = $$REPORT_DESIGNER_PLUGINS_PATH
        INSTALLS += target
    }
} else {
    DESTDIR = ../../../../$$BUILD_DIR/$$REPORT_DESIGNER_PLUGINS_PATH
}

contains(DEFINES, STATICPLUGINS_DESIGNER) {
    CONFIG += static
}

