INCLUDEPATH += \
    $$PWD/../core \
    $$PWD/../core/log/ \
    $$PWD/../../thirdparty/propertyeditor/lib/  \
    $$PWD/../appsCore/designerCore/ \
    $$PWD/../appsCore/designerCore/widgets/ \
    $$PWD/../widgets/widgets/ \
    $$PWD/../../thirdparty/propertyeditor/lib/ \
    $$PWD/../../thirdparty/propertyeditor/ \
    $$PWD/../plugins/designer_plugins \
    $$PWD/../plugins/core_plugins \

DEPENDPATH += $$INCLUDEPATH


contains(DEFINES, STATICBUILD) {
    CONFIG += static
    lessThan(QT_MAJOR_VERSION, 5) {
    qtAddLibrary(QtDesignerComponents)
        CONFIG += designer
        QT += webkit
    } else {
        QT += designer core-private widgets xml network designer-private designercomponents-private
    }
}


LIBS += -L../../../$$BUILD_DIR -L../../../$$BUILD_DIR/$$REPORT_LIBS_PATH

contains(DEFINES, STATIC_PROPERTYEDITOR) {
    win32: CONFIG(debug, debug|release): LIBS += -lPropertyEditord
    else: LIBS += -lPropertyEditor
}

contains(DEFINES, STATICPLUGINS_PROPERTYEDITOR) {
    include($$PWD/../../thirdparty/propertyeditor/propertyeditor_static.pri)
    HEADERS += propertyeditor_static.h
    LIBS += -L$$PWD/../../$$BUILD_DIR/$$PROPERTYEDITOR_PLUGINS_PATH
}


win32: CONFIG(debug, debug|release): LIBS += -lCuteReportWidgetsd -lCuteDesignerd -lCuteReportCored -lCuteReportWidgetsThirdpartyd
else: LIBS += -lCuteReportWidgets -lCuteDesigner -lCuteReportCore -lCuteReportWidgetsThirdparty


contains(DEFINES, STATICPLUGINS_DESIGNER) {
    include($$PWD/../plugins/designer_plugins/designer_static.pri)
    HEADERS += $$PWD/../plugins/designer_plugins/designer_static.h
    LIBS += -L../../../$$BUILD_DIR/$$REPORT_DESIGNER_PLUGINS_PATH

    contains(DEFINES, EXTENDEDSUITE) {
        include($$PWD/../../addons_pro/plugins/designer_plugins/designer_staticExt.pri)
        HEADERS += $$PWD/../../addons_pro/plugins/designer_plugins/designer_staticExt.h
        INCLUDEPATH += $$PWD/../../addons_pro/plugins/designer_plugins
    }
}

contains(DEFINES, STATICPLUGINS_CORE) {
    include($$PWD/../plugins/core_plugins/core_static.pri)
    HEADERS += $$PWD/../plugins/core_plugins/core_static.h
    LIBS += -L../../../$$BUILD_DIR/$$REPORT_PLUGINS_PATH

    contains(DEFINES, EXTENDEDSUITE) {
        include($$PWD/../../addons_pro/plugins/core_plugins/core_staticExt.pri)
        HEADERS += $$PWD/../../addons_pro/plugins/core_plugins/core_staticExt.h
        INCLUDEPATH += $$PWD/../../addons_pro/plugins/core_plugins
    }
}

!contains(DEFINES, STATIC_PROPERTYEDITOR) {
    win32: CONFIG(debug, debug|release): LIBS += -lPropertyEditord
    else: LIBS += -lPropertyEditor
}

win32: CONFIG(debug, debug|release): LIBS += -lCuteDesignerd -lCuteReportWidgetsd -lCuteReportCored
else: LIBS += -lCuteDesigner -lCuteReportWidgets -lCuteReportCore
