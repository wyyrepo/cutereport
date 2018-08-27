#QMAKE_CFLAGS = -nologo -Zm200 -Zc:wchar_t

equals(FRAMEWORK_TYPE, SYS): DEFINES += SYSTEMINSTALL

contains(DEFINES, SYSTEMINSTALL) {
    DEFINES += GLOBAL_LOG_FILE
    DEFINES -= SYSTEMINSTALL

    REPORT_BINARIES_PATH =
    REPORT_LIBS_PATH =
    REPORT_HEADERS_PATH =                development/include/cutereport
    REPORT_RESOURCES_PATH =              cutereport_resources/share
    REPORT_IMAGES_PATH =                 cutereport_resources/images
    CONFIG(debug, debug|release): REPORT_PLUGINS_PATH =                development/Qt$${QT_MAJOR_VERSION}/plugins/debug/core/
    else:                         REPORT_PLUGINS_PATH =                development/Qt$${QT_MAJOR_VERSION}/plugins/release/core/
    REPORT_EXAMPLES_PATH =               development/examples
    REPORT_TEMPLATES_PATH =              cutereport_resources/templates

    REPORT_DESIGNER_PATH =
    CONFIG(debug, debug|release): REPORT_DESIGNER_PLUGINS_PATH =       development/Qt$${QT_MAJOR_VERSION}/plugins/debug/designer
    else:                         REPORT_DESIGNER_PLUGINS_PATH =       development/Qt$${QT_MAJOR_VERSION}/plugins/release/designer
    REPORT_DESIGNER_LIBS_PATH =
    REPORT_DESIGNER_IMAGES_PATH=         cutereport_resources/designer/images
    REPORT_DESIGNER_RESOURCES_PATH =     cutereport_resources/designer/share

    PROPERTYEDITOR_LIBS_PATH =
    PROPERTYEDITOR_HEADERS_PATH =        development/include/propertyeditor
    CONFIG(debug, debug|release): PROPERTYEDITOR_PLUGINS_PATH =        development/Qt$${QT_MAJOR_VERSION}/plugins/debug/propertyeditor
    else:                         PROPERTYEDITOR_PLUGINS_PATH =        development/Qt$${QT_MAJOR_VERSION}/plugins/release/propertyeditor
    QTDESIGNER_PLUGINS_PATH =            designer

    DOC_PATH =                           cutereport_resources/doc

    THIRDPARTY_BIN_PATH =                cutereport_resources/bin
    THIRDPARTY_LIB_PATH =                cutereport_resources/

} else {

    REPORT_BINARIES_PATH =
    REPORT_LIBS_PATH =
    REPORT_HEADERS_PATH =                cutereport_resources/include
    REPORT_RESOURCES_PATH =              cutereport_resources/share
    REPORT_IMAGES_PATH =                 cutereport_resources/images
    CONFIG(debug, debug|release): REPORT_PLUGINS_PATH =                development/plugins/core/
    else:                         REPORT_PLUGINS_PATH =                cutereport_resources/plugins/core/
    REPORT_EXAMPLES_PATH =               cutereport_resources/examples
    REPORT_TEMPLATES_PATH =              cutereport_resources/templates

    REPORT_DESIGNER_PATH =
    CONFIG(debug, debug|release): REPORT_DESIGNER_PLUGINS_PATH =       development/plugins/debug/designer
    else:                         REPORT_DESIGNER_PLUGINS_PATH =       cutereport_resources/plugins/designer
    REPORT_DESIGNER_LIBS_PATH =
    REPORT_DESIGNER_IMAGES_PATH=         cutereport_resources/designer/images
    REPORT_DESIGNER_RESOURCES_PATH =     cutereport_resources/designer/share

    PROPERTYEDITOR_LIBS_PATH =
    PROPERTYEDITOR_HEADERS_PATH =        cutereport_resources/include/propertyeditor
    CONFIG(debug, debug|release): PROPERTYEDITOR_PLUGINS_PATH =        development/plugins/propertyeditor
    else:                         PROPERTYEDITOR_PLUGINS_PATH =        cutereport_resources/plugins/propertyeditor
    QTDESIGNER_PLUGINS_PATH =            designer

    DOC_PATH =                           cutereport_resources/doc

    THIRDPARTY_BIN_PATH =                cutereport_resources/bin
    THIRDPARTY_LIB_PATH =                cutereport_resources/
}

*-g++* {
    COMPILER_VER="mingw"
}
*-msvc* {
    lessThan(QT_MAJOR_VERSION, 5) {
        COMPILER_VER="msvc2010"
    } else {
        #MSVC_VER = $$(VisualStudioVersion)
        equals(MSVC_VER, 10.0) : COMPILER_VER="msvc2010"
        equals(MSVC_VER, 11.0) : COMPILER_VER="msvc2012"
        equals(MSVC_VER, 12.0) : COMPILER_VER="msvc2013"
        equals(MSVC_VER, 13.0) : COMPILER_VER="msvc2014"
        equals(MSVC_VER, 14.0) : COMPILER_VER="msvc2015"
        #message (MSVC_VER = $$MSVC_VER)

        #it seems for Qt5 only:
        contains(QT_ARCH, x86_64): COMPILER_VER=$${COMPILER_VER}_64
    }
}


DEFINES += REPORT_VARS_PATH=\\\"$$REPORT_VARS_PATH\\\"

DEFINES += REPORT_BINARIES_PATH=\\\"$$REPORT_BINARIES_PATH\\\"
DEFINES += REPORT_LIBS_PATH=\\\"$$REPORT_LIBS_PATH\\\"
DEFINES += REPORT_HEADERS_PATH=\\\"$$REPORT_HEADERS_PATH\\\"
DEFINES += REPORT_RESOURCES_PATH=\\\"$$REPORT_RESOURCES_PATH\\\"
DEFINES += REPORT_IMAGES_PATH=\\\"$$REPORT_IMAGES_PATH\\\"
DEFINES += REPORT_PLUGINS_PATH=\\\"$$REPORT_PLUGINS_PATH\\\"
DEFINES += REPORT_EXAMPLES_PATH=\\\"$$REPORT_EXAMPLES_PATH\\\"
DEFINES += REPORT_TEMPLATES_PATH=\\\"$$REPORT_TEMPLATES_PATH\\\"

DEFINES += REPORT_DESIGNER_PATH=\\\"$$REPORT_DESIGNER_PATH\\\"
DEFINES += REPORT_DESIGNER_PLUGINS_PATH=\\\"$$REPORT_DESIGNER_PLUGINS_PATH\\\"
DEFINES += REPORT_DESIGNER_LIBS_PATH=\\\"$$REPORT_DESIGNER_LIBS_PATH\\\"
DEFINES += REPORT_DESIGNER_IMAGES_PATH=\\\"$$REPORT_DESIGNER_IMAGES_PATH\\\"
DEFINES += REPORT_DESIGNER_RESOURCES_PATH=\\\"$$REPORT_DESIGNER_RESOURCES_PATH\\\"

DEFINES += PROPERTYEDITOR_LIBS=\\\"$$PROPERTYEDITOR_LIBS_PATH\\\"
DEFINES += PROPERTYEDITOR_HEADERS_PATH=\\\"$$PROPERTYEDITOR_HEADERS_PATH\\\"
DEFINES += PROPERTYEDITOR_PLUGINS_PATH=\\\"$$PROPERTYEDITOR_PLUGINS_PATH\\\"

DEFINES += REPORT_LOGFILENAME=\\\"$$REPORT_LOGFILENAME\\\"

DEFINES += COMPILER_VER=\\\"$$COMPILER_VER\\\"
DEFINES += PROCESSOR_TYPE=\\\"$$QMAKE_TARGET.arch\\\"
#it seems for Qt5 only:
#contains(QT_ARCH, i386) {

CONFIG -= debug_and_release
#QMAKE_LFLAGS += /INCREMENTAL:NO
#QMAKE_LFLAGS += /VERBOSE

#CONFIG(debug, debug|release) {
#    TARGET = $$join(TARGET,,,d)
#    message("debug mode")
#    message("Target =" $$TARGET)
#}else {
##    message("release mode")
#}
