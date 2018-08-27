LIBS += -lDetail
LIBS += -lDetailFooter
LIBS += -lDetailHeader
LIBS += -lOverlay
LIBS += -lPageFooter
LIBS += -lPageHeader
LIBS += -lSummary
LIBS += -lTitle

LIBS += -lPage
LIBS += -lPrinter
LIBS += -lRenderer
LIBS += -lScript

LIBS += -lDatasetCSV
LIBS += -lDatasetFileSystem
LIBS += -lDatasetModel
LIBS += -lDatasetSQL

LIBS += -lExportPDF
#LIBS += -lExportSVG

LIBS += -lArc
LIBS += -lChord
LIBS += -lEllipse
LIBS += -lImage
LIBS += -lLine
LIBS += -lMemo
LIBS += -lPie
LIBS += -lRectangle

contains(DEFINES, GPL): LIBS += -lBarcode
#LIBS += -lChart

LIBS += -lSerializerXML

LIBS += -lStorageFileSystem
LIBS += -lStorageGIT
LIBS += -lStorageResource
LIBS += -lStorageSQL


QT += printsupport
