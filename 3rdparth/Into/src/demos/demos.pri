# Common build configuration for all test programs that use Ydin. The
# default build mode is "release". Generate a debug makefile with
# "qmake MODE=debug".
#
# All one needs to do in a demos-specific .pro file is this:
# include(../demos.pri)

isEmpty(INTODIR) { INTODIR = ../../ }
include($$INTODIR/base.pri)

INCLUDEPATH += $$INTODIR/demos/utils
HEADERS   = *.h
SOURCES   = *.cc

LIBS += -lpiiydin$$LIBVER -lpiicore$$LIBVER -L$$INTODIR/demos/utils/$$MODE -ldemoutils

include($$INTODIR/plugindeps.pri)

#win32:CONFIG -= embed_manifest_exe
