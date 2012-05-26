INTODIR = ../../
include($$INTODIR/base.pri)

FORMS += ui/*.ui
RESOURCES += resources.qrc

HEADERS   = *.h
SOURCES   = *.cc
LIBS += -lpiiydin$$LIBVER -lpiicore$$LIBVER
