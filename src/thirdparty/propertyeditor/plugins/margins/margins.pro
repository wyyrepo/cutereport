! include( ../property_plugins.pri ) {
    error( Can not find the plugins.pri file! )
}

HEADERS += margins.h
SOURCES += margins.cpp
TARGET = MarginsProperty
