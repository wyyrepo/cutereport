
equals(FRAMEWORK_TYPE, SYS): DEFINES += SYSTEMINSTALL

contains(DEFINES, SYSTEMINSTALL) {

    REPORT_ROOT_PATH = $$SYSTEM_INSTALL_PATH

    REPORT_BINARIES_PATH =               $$REPORT_ROOT_PATH/bin
    REPORT_LIBS_PATH =                   $$REPORT_ROOT_PATH/lib/
    REPORT_HEADERS_PATH =                $$REPORT_ROOT_PATH/include/cutereport
    REPORT_RESOURCES_PATH =              $$REPORT_ROOT_PATH/share/cutereport
    REPORT_IMAGES_PATH =                 $$REPORT_ROOT_PATH/share/cutereport/images
    REPORT_PLUGINS_PATH =                $$REPORT_ROOT_PATH/lib/cutereport/reportcore_plugins
    REPORT_EXAMPLES_PATH =               $$REPORT_ROOT_PATH/share/cutereport/examples
    REPORT_TEMPLATES_PATH =              $$REPORT_ROOT_PATH/share/cutereport/templates

    REPORT_DESIGNER_PATH =               $$REPORT_ROOT_PATH/bin
    REPORT_DESIGNER_PLUGINS_PATH =       $$REPORT_ROOT_PATH/lib/cutereport/designer_plugins
    REPORT_DESIGNER_LIBS_PATH =          $$REPORT_ROOT_PATH/lib/
    REPORT_DESIGNER_IMAGES_PATH =        $$REPORT_ROOT_PATH/share/cutereport/designer/images
    REPORT_DESIGNER_RESOURCES_PATH =     $$REPORT_ROOT_PATH/share/cutereport/designer/

    PROPERTYEDITOR_LIBS_PATH =           $$REPORT_ROOT_PATH/lib/
    PROPERTYEDITOR_HEADERS_PATH =        $$REPORT_ROOT_PATH/include/propertyeditor
    PROPERTYEDITOR_PLUGINS_PATH =        $$REPORT_ROOT_PATH/lib/cutereport/propertyeditor_plugins

    QTDESIGNER_PLUGINS_PATH =

    DOC_PATH =                           $$REPORT_ROOT_PATH/share/cutereport

    THIRDPARTY_BIN_PATH =
    THIRDPARTY_LIB_PATH =

} else {

    REPORT_BINARIES_PATH =
    REPORT_LIBS_PATH =                   cutereport_resources/lib
    REPORT_HEADERS_PATH =                cutereport_resources/include
    REPORT_RESOURCES_PATH =              cutereport_resources/share
    REPORT_IMAGES_PATH =                 cutereport_resources/images
    REPORT_PLUGINS_PATH =                cutereport_resources/plugins/core/
    REPORT_EXAMPLES_PATH =               cutereport_resources/examples
    REPORT_TEMPLATES_PATH =              cutereport_resources/templates

    REPORT_DESIGNER_PATH =
    REPORT_DESIGNER_PLUGINS_PATH =       cutereport_resources/plugins/designer
    REPORT_DESIGNER_LIBS_PATH =          cutereport_resources/lib
    REPORT_DESIGNER_IMAGES_PATH=         cutereport_resources/designer/images
    REPORT_DESIGNER_RESOURCES_PATH =     cutereport_resources/designer/share

    PROPERTYEDITOR_LIBS_PATH =           cutereport_resources/lib
    PROPERTYEDITOR_HEADERS_PATH =        cutereport_resources/include/propertyeditor
    PROPERTYEDITOR_PLUGINS_PATH =        cutereport_resources/plugins/propertyeditor

    QTDESIGNER_PLUGINS_PATH =            designer

    DOC_PATH =                           cutereport_resources/doc

    THIRDPARTY_BIN_PATH =                cutereport_resources/bin
    THIRDPARTY_LIB_PATH =                cutereport_resources/lib
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

DEFINES += DOC_PATH=\\\"$$DOC_PATH\\\"

DEFINES += THIRDPARTY_BIN_PATH=\\\"$$THIRDPARTY_BIN_PATH\\\"
DEFINES += THIRDPARTY_LIB_PATH=\\\"$$THIRDPARTY_LIB_PATH\\\"

DEFINES += REPORT_LOGFILENAME=\\\"$$REPORT_LOGFILENAME\\\"

DEFINES += COMPILER_VER=\\\"gcc\\\"
DEFINES += PROCESSOR_TYPE=\\\"$$QMAKE_TARGET.arch\\\"
