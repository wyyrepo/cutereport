
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
    DOC_PATH =                           $$REPORT_ROOT_PATH/share/cutereport/doc

} else {

contains(DEFINES, DEMO_BUILD) {
    message(DEMOBUILD)
    REPORT_ROOT_PATH = cutereport_demo.app/Contents
} else {
    message(NOT DEMO BUILD -----------------------)
    REPORT_ROOT_PATH = cutereport.app/Contents
}

    #REPORT_BINARIES_PATH =               $$REPORT_ROOT_PATH/MacOS
    REPORT_BINARIES_PATH =               
    REPORT_LIBS_PATH =                   $$REPORT_ROOT_PATH/Frameworks
    REPORT_HEADERS_PATH =                $$REPORT_ROOT_PATH/Resources/include
    REPORT_RESOURCES_PATH =              $$REPORT_ROOT_PATH/Resources
    REPORT_IMAGES_PATH =                 $$REPORT_ROOT_PATH/Resources/images
    REPORT_PLUGINS_PATH =                $$REPORT_ROOT_PATH/PlugIns/reportcore_plugins
    REPORT_EXAMPLES_PATH =               $$REPORT_ROOT_PATH/Resources/examples
    REPORT_TEMPLATES_PATH =              $$REPORT_ROOT_PATH/Resources/templates

    REPORT_DESIGNER_PATH =               
    REPORT_DESIGNER_PLUGINS_PATH =       $$REPORT_ROOT_PATH/PlugIns/designer_plugins
    REPORT_DESIGNER_LIBS_PATH =          $$REPORT_ROOT_PATH/Frameworks
    REPORT_DESIGNER_IMAGES_PATH =        $$REPORT_ROOT_PATH/Resources/images/designer
    REPORT_DESIGNER_RESOURCES_PATH =     $$REPORT_ROOT_PATH/Resources

    PROPERTYEDITOR_LIBS_PATH =           $$REPORT_ROOT_PATH/Frameworks
    PROPERTYEDITOR_HEADERS_PATH =        $$REPORT_ROOT_PATH/Resources/include/propertyeditor
    PROPERTYEDITOR_PLUGINS_PATH =        $$REPORT_ROOT_PATH/PlugIns/propertyeditor

    QTDESIGNER_PLUGINS_PATH =
    DOC_PATH =                           $$REPORT_ROOT_PATH/Resources/Docs

}

COMPILER_VER="clang"


DEFINES += REPORT_VARS_PATH=\\\"$$REPORT_VARS_PATH\\\"

DEFINES += REPORT_BINARIES_PATH=\\\"$$REPORT_BINARIES_PATH\\\"
DEFINES += REPORT_LIBS_PATH=\\\"../../../$$REPORT_LIBS_PATH\\\"
DEFINES += REPORT_HEADERS_PATH=\\\"../../../$$REPORT_HEADERS_PATH\\\"
DEFINES += REPORT_RESOURCES_PATH=\\\"../../../$$REPORT_RESOURCES_PATH\\\"
DEFINES += REPORT_IMAGES_PATH=\\\"../../../$$REPORT_IMAGES_PATH\\\"
DEFINES += REPORT_PLUGINS_PATH=\\\"../../../$$REPORT_PLUGINS_PATH\\\"
DEFINES += REPORT_EXAMPLES_PATH=\\\"../../../$$REPORT_EXAMPLES_PATH\\\"
DEFINES += REPORT_TEMPLATES_PATH=\\\"../../../$$REPORT_TEMPLATES_PATH\\\"

DEFINES += REPORT_DESIGNER_PATH=\\\"$$REPORT_DESIGNER_PATH\\\"
DEFINES += REPORT_DESIGNER_PLUGINS_PATH=\\\"../../../$$REPORT_DESIGNER_PLUGINS_PATH\\\"
DEFINES += REPORT_DESIGNER_LIBS_PATH=\\\"../../../$$REPORT_DESIGNER_LIBS_PATH\\\"
DEFINES += REPORT_DESIGNER_IMAGES_PATH=\\\"../../../$$REPORT_DESIGNER_IMAGES_PATH\\\"
DEFINES += REPORT_DESIGNER_RESOURCES_PATH=\\\"../../../$$REPORT_DESIGNER_RESOURCES_PATH\\\"

DEFINES += PROPERTYEDITOR_LIBS=\\\"../../../$$PROPERTYEDITOR_LIBS_PATH\\\"
DEFINES += PROPERTYEDITOR_HEADERS_PATH=\\\"../../../$$PROPERTYEDITOR_HEADERS_PATH\\\"
DEFINES += PROPERTYEDITOR_PLUGINS_PATH=\\\"../../../$$PROPERTYEDITOR_PLUGINS_PATH\\\"

DEFINES += REPORT_LOGFILENAME=\\\"$$REPORT_LOGFILENAME\\\"

DEFINES += COMPILER_VER=\\\"$$COMPILER_VER\\\"
DEFINES += PROCESSOR_TYPE=\\\"$$QMAKE_TARGET.arch\\\"
#it seems for Qt5 only:
#contains(QT_ARCH, i386) {
