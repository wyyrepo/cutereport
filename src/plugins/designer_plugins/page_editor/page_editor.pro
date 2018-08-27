! include( ../designerModules.pri ) {
    error( Can not find the designerModules.pri file! )
}

TARGET = PageEditor

SOURCES += \
    pageeditorcontainer.cpp \
    pageeditor.cpp \
    itemstoolwidget.cpp \
    fonteditor/fonteditor.cpp \
    alignmenteditor/alignmenteditor.cpp \
    frameeditor/frameeditor.cpp

HEADERS +=  \
    pageeditorcontainer.h \
    pageeditor.h \
    itemstoolwidget.h \
    fonteditor/fonteditor.h \
    alignmenteditor/alignmenteditor.h \
    frameeditor/frameeditor.h

FORMS += pageeditorcontainer.ui \

RESOURCES += \
    page_editor.qrc

INCLUDEPATH += fonteditor/ \
    alignmenteditor/ \
    frameeditor/
