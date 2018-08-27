VERSION = 1.3.2

!include(../build.cfg ) {
    error( Can not find the build.cfg file! )
}

equals(BUILD_TYPE, GPL) {
    DEFINES += REPORT_LICENSE=\\\"GPL\\\"
    DEFINES += DESIGNER_LICENSE=\\\"GPL\\\"
    DEFINES += GPL
}
equals(BUILD_TYPE, LGPL) {
    DEFINES += REPORT_LICENSE=\\\"LGPL\\\"
    DEFINES += DESIGNER_LICENSE=\\\"GPL\\\"
    DEFINES += LGPL
}
equals(BUILD_TYPE, PRO) {
    DEFINES += STATICBUILD
    DEFINES += DEMO
    DEFINES += PROBUILD
    DEFINES += REPORT_LICENSE=\\\"Commercial\\\"
    DEFINES += DESIGNER_LICENSE=\\\"Commercial\\\"
    DEFINES += PRO
}
equals(BUILD_TYPE, DEMO) {
    DEFINES += STATICBUILD
    DEFINES += PROBUILD
    DEFINES += REPORT_LICENSE=\\\"Commercial\\\"
    DEFINES += DESIGNER_LICENSE=\\\"Commercial\\\"
    DEFINES += DEMO_BUILD
}
equals(BUILD_TYPE, DEV) {
    DEFINES += REPORT_LICENSE=\\\"Commercial\\\"
    DEFINES += DESIGNER_LICENSE=\\\"Commercial\\\"
    DEFINES += GPL
    DEFINES += PRO
}


isEmpty(BUILD_DIR): BUILD_DIR = build
isEmpty(REPORT_VARS_PATH): REPORT_VARS_PATH = "temp/cutereport/"

DEFINES += BUILD_DATETIME='"\\\"$$_DATE_\\\""'

BUILD_DIR = build

unix:!macx: PROFILE_FILE = "linux.pri"
macx: PROFILE_FILE = "macx.pri"
win32: PROFILE_FILE = "win.pri"

!include( $$PROFILE_FILE ) {
    error( Can not find the $$PROFILE_FILE file! )
}


# -------------------------------------------------------------

REPORT_VERSION=$$VERSION
DEFINES += REPORT_VERSION=\\\"$$REPORT_VERSION\\\"

INCLUDEPATH += lib/
DEPENDPATH += $$INCLUDEPATH

lessThan(QT_MAJOR_VERSION, 5) {
    QT += script xml sql
} else {
    QT += script xml sql widgets
}


equals(BUILD_TYPE, PRO): DEFINES += EXTENDEDSUITE
equals(BUILD_TYPE, DEMO): DEFINES += EXTENDEDSUITE
equals(BUILD_TYPE, DEV): DEFINES += EXTENDEDSUITE

contains(DEFINES, STATICBUILD) {
    #DEFINES += STATIC_PROPERTYEDITOR
    #DEFINES += STATICPLUGINS_PROPERTYEDITOR
    DEFINES += STATIC_DESIGNER
    DEFINES += STATICPLUGINS_DESIGNER
    DEFINES += STATIC_CORE
    DEFINES += STATICPLUGINS_CORE
    DEFINES += STATIC_WIDGETS
}
