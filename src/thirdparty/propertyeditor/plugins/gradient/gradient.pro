! include( ../property_plugins.pri ) {
    error( Can not find the plugins.pri file! )
}

HEADERS += gradient.h \
        changegradient.h \
        creategradientdialog.h

SOURCES += gradient.cpp \
        changegradient.cpp \
        creategradientdialog.cpp \

TARGET = GradientProperty

FORMS += \
    creategradientdialog.ui
