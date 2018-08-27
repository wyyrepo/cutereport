! include( ../designerModules.pri ) {
    error( Can not find the designerModules.pri file! )
}

TARGET = ReportEditor

SOURCES += \
    $$PWD/reportcontainer.cpp \
    $$PWD/reporteditor.cpp \
    $$PWD/reportproperties.cpp \
    $$PWD/parameterswidget.cpp \
    $$PWD/comboboxdelegate.cpp \
    $$PWD/parametersmodel.cpp

HEADERS +=  \
    $$PWD/reportcontainer.h \
    $$PWD/reporteditor.h \
    $$PWD/reportproperties.h \
    $$PWD/parameterswidget.h \
    $$PWD/comboboxdelegate.h \
    $$PWD/parametersmodel.h

FORMS += $$PWD/reportcontainer.ui \
    $$PWD/reportproperties.ui \
    $$PWD/parameterswidget.ui \

RESOURCES += \
    reporteditor.qrc


