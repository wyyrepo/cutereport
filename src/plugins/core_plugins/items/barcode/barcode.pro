! include( ../../plugins.pri ) {
    error( Could not find the common.pri file! )
}

! include ( barcode.pri ) {
    error( Could not find item .pri file! )
}


TARGET = Barcode

DEFINES += ZINT_VERSION=\\\"2.4.4\\\"
DEFINES += NO_PNG

win32-msvc* {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    QMAKE_CFLAGS += /TP /wd4018 /wd4244 /wd4305
    QMAKE_CXXFLAGS += /TP /wd4018 /wd4244 /wd4305
}

ZINT_ROOT = $$PWD/../../../../../thirdparty/sources/zint/

INCLUDEPATH += \
    $$ZINT_ROOT \
    $$ZINT_ROOT/backend \
    $$ZINT_ROOT/backend_qt4 \

DEPENDPATH  += $$INCLUDEPATH

HEADERS += \
    $$ZINT_ROOT/backend_qt4/qzint.h \
    $$ZINT_ROOT/backend/zint.h \
    $$ZINT_ROOT/backend/sjis.h \
    $$ZINT_ROOT/backend/rss.h \
    $$ZINT_ROOT/backend/reedsol.h \
    $$ZINT_ROOT/backend/qr.h \
    $$ZINT_ROOT/backend/pdf417.h \
    $$ZINT_ROOT/backend/ms_stdint.h \
    $$ZINT_ROOT/backend/maxipng.h \
    $$ZINT_ROOT/backend/maxicode.h \
    $$ZINT_ROOT/backend/large.h \
    $$ZINT_ROOT/backend/gs1.h \
    $$ZINT_ROOT/backend/gridmtx.h \
    $$ZINT_ROOT/backend/gb2312.h \
    $$ZINT_ROOT/backend/font.h \
    $$ZINT_ROOT/backend/dmatrix.h \
    $$ZINT_ROOT/backend/composite.h \
    $$ZINT_ROOT/backend/common.h \
    $$ZINT_ROOT/backend/code49.h \
    $$ZINT_ROOT/backend/code1.h \
    $$ZINT_ROOT/backend/aztec.h \

SOURCES += \
    $$ZINT_ROOT/backend_qt4/qzint.cpp \
    $$ZINT_ROOT/backend/library.c \
    $$ZINT_ROOT/backend/upcean.c \
    $$ZINT_ROOT/backend/telepen.c \
    $$ZINT_ROOT/backend/svg.c \
    $$ZINT_ROOT/backend/rss.c \
    $$ZINT_ROOT/backend/render.c \
    $$ZINT_ROOT/backend/reedsol.c \
    $$ZINT_ROOT/backend/qr.c \
    $$ZINT_ROOT/backend/ps.c \
    $$ZINT_ROOT/backend/postal.c \
    $$ZINT_ROOT/backend/png.c \
    $$ZINT_ROOT/backend/plessey.c \
    $$ZINT_ROOT/backend/pdf417.c \
    $$ZINT_ROOT/backend/medical.c \
    $$ZINT_ROOT/backend/maxicode.c \
    $$ZINT_ROOT/backend/large.c \
    $$ZINT_ROOT/backend/imail.c \
    $$ZINT_ROOT/backend/gs1.c \
    $$ZINT_ROOT/backend/gridmtx.c \
    $$ZINT_ROOT/backend/dmatrix.c \
    $$ZINT_ROOT/backend/dllversion.c \
    $$ZINT_ROOT/backend/composite.c \
    $$ZINT_ROOT/backend/common.c \
    $$ZINT_ROOT/backend/code128.c \
    $$ZINT_ROOT/backend/code49.c \
    $$ZINT_ROOT/backend/code16k.c \
    $$ZINT_ROOT/backend/code1.c \
    $$ZINT_ROOT/backend/code.c \
    $$ZINT_ROOT/backend/aztec.c \
    $$ZINT_ROOT/backend/auspost.c \
    $$ZINT_ROOT/backend/2of5.c

