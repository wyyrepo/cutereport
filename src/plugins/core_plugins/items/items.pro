TEMPLATE = subdirs
SUBDIRS = image \
        memo \
	line \
	rectangle \
        ellipse \
	arc \
	chord \
	pie \

!include( $$PWD/../../../../profiles/common.pri ) {
    error( Items.pro: Cannot find the common.pri file! )
}

contains(DEFINES, GPL): SUBDIRS += barcode
