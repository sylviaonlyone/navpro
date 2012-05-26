INTODIR = ../../../
include(../../demos.pri)

FORMS += ui/*.ui
RESOURCES += resources.qrc

INCLUDEPATH += . $$INTODIR/gui
LIBS += -L$$INTODIR/gui/$$MODE -lpiigui$$LIBVER

