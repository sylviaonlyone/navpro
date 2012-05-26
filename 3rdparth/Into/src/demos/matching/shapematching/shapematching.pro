INTODIR = ../../../
include(../../demos.pri)

FORMS += ui/*.ui

INCLUDEPATH += . $$INTODIR/gui
LIBS += -L$$INTODIR/gui/$$MODE -lpiigui$$LIBVER
