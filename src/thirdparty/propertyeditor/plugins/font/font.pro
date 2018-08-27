! include( ../property_plugins.pri ) {
    error( Can not find the plugins.pri file! )
}

HEADERS += font.h changefont.h
SOURCES += font.cpp changefont.cpp
TARGET = FontProperty

