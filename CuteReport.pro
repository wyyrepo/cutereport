!include( $$PWD/profiles/common.pri ) {
    error( CuteReport.pro: Cannot find the common.pri file! )
}

TEMPLATE = subdirs
SUBDIRS += \
        src

!equals(BUILD_TYPE, DEMO){
    SUBDIRS += apps
    apps.subdir = src/apps
}

contains(DEFINES, EXTENDEDSUITE) {
    SUBDIRS += addons_pro
    equals(BUILD_TYPE, DEMO) || equals(BUILD_TYPE, DEV){
        SUBDIRS += demo_app
        demo_app.subdir = addons_pro/apps/demo_app
        demo_app.depends = src addons_pro
    }
    contains(DEFINES, STATICBUILD) {
        src.depends = addons_pro
    } else {
        addons_pro.depends = src
    }
    apps.depends = src addons_pro
    message(Extended suite included)
} else {
    apps.depends = src
    message(Without Extended suite)
}

ICON_FILES.files = pkg/*.ico pkg/*.svg
DOC_FILES.files = COPYING CHANGELOG LICENSE README THIRDPARTY-LICENSE-*
DATASET_FILES.files = examples/datasets/*
REPORT_FILES.files = examples/reports/*
CODE_EXAMPLES_FILES.files = examples/code/*
TEMPLATE_FILES.files = examples/templates/*
IMAGE_FILES.files = examples/images/*

macx: ICON = cutereport.icns
    
INSTALLS += ICON_FILES DOC_FILES ADDITIONAL_FILES REPORT_FILES TEMPLATE_FILES IMAGE_FILES CODE_EXAMPLES_FILES
# DATASET_FILES

contains(DEFINES, SYSTEMINSTALL) {
    ICON_FILES.path = $$REPORT_IMAGES_PATH
    DOC_FILES.path = $$DOC_PATH
    ADDITIONAL_FILES.path = $$REPORT_RESOURCES_PATH
    DATASET_FILES.path = $$REPORT_EXAMPLES_PATH/datasets/
    REPORT_FILES.path = $$REPORT_EXAMPLES_PATH/reports/
    CODE_EXAMPLES_FILES.path = $$REPORT_EXAMPLES_PATH/code/
    TEMPLATE_FILES.path = $$REPORT_EXAMPLES_PATH/templates/
    IMAGE_FILES.path = $$REPORT_EXAMPLES_PATH/images/

    win32 {
        ISS_FILE = $$PWD/addons_pro/pkg/pkg_scripts/install_$${BUILD_TYPE}_qt$${QT_MAJOR_VERSION}.iss
        exists($$ISS_FILE) {
            PKG_FILES.files += $$ISS_FILE $$PWD/addons_pro/pkg/pkg_scripts/modpath.iss
            PKG_FILES.path = $$REPORT_ROOT_PATH
            INSTALLS += PKG_FILES
        }
    }

} else {
    ICON_FILES.path = $$OUT_PWD/$$BUILD_DIR/$$REPORT_IMAGES_PATH
    DOC_FILES.path = $$OUT_PWD/$$BUILD_DIR/$$DOC_PATH
    ADDITIONAL_FILES.path = $$OUT_PWD/$$BUILD_DIR/$$REPORT_RESOURCES_PATH
    DATASET_FILES.path = $$OUT_PWD/$$BUILD_DIR/$$REPORT_EXAMPLES_PATH/datasets/
    REPORT_FILES.path = $$OUT_PWD/$$BUILD_DIR/$$REPORT_EXAMPLES_PATH/reports/
    CODE_EXAMPLES_FILES.path =  $$OUT_PWD/$$BUILD_DIR/$$REPORT_EXAMPLES_PATH/code/
    TEMPLATE_FILES.path = $$OUT_PWD/$$BUILD_DIR/$$REPORT_EXAMPLES_PATH/templates/
    IMAGE_FILES.path = $$OUT_PWD/$$BUILD_DIR/$$REPORT_EXAMPLES_PATH/images/

    win32 {
        ISS_FILE = $$PWD/addons_pro/pkg/pkg_scripts/install_$${BUILD_TYPE}_qt$${QT_MAJOR_VERSION}.iss
        exists($$ISS_FILE) {
            PKG_FILES.files += $$ISS_FILE $$PWD/addons_pro/pkg/pkg_scripts/modpath.iss
            PKG_FILES.path = $$OUT_PWD/$$BUILD_DIR/
            INSTALLS += PKG_FILES
        }
    }
}

