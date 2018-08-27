!include( ../profiles/common.pri ) {
    error( Can not find the common.pri file! )
}

TEMPLATE = subdirs
SUBDIRS = \
        thirdparty \
        core \
        widgets \
        plugins \
        appsCore \
#	apps \

#!contains(DEFINES, STATICBUILD) {
    widgets.depends = core
    plugins.depends = core widgets appsCore thirdparty
    appsCore.depends = core widgets
    #apps.depends = core thirdparty widgets appsCore plugins
#}


