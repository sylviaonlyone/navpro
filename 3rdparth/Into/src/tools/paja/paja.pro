INTODIR = ../../
include($$INTODIR/base.pri)

FORMS += ui/*.ui
#RESOURCES += resources.qrc

HEADERS   = *.h
SOURCES   = *.cc
unix: LIBS += -lpiiydin -lpiicore
win32: LIBS += -lpiiydin2 -lpiicore2
