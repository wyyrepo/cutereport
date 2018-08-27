! include( ../designerModules.pri ) {
    error( Can not find the designerModules.pri file! )
}

TARGET = ScriptEditor

SOURCES += \
    scripteditor.cpp \

HEADERS +=  \
    scripteditor.h \

RESOURCES += \
    script_editor.qrc

