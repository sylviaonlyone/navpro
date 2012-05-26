# On Windows, only MSVC is supported
# qmake -spec win32-msvc2008
# nmake
# If you are compiling the wrapper for mingw:
# nmake mingw

isEmpty(MODE) { MODE = debug }

INTODIR = ../../..
TEMPLATE = lib
CONFIG -= qt
CONFIG += $$MODE
SOURCES = *.cc
TARGET = pylonwrapper

DESTDIR = $$MODE
OBJECTS_DIR = $$MODE
MOC_DIR = $$MODE

unix:QMAKE_LFLAGS += -Wl,-E

#DEFINES += BUILD_GENICAM_WAPI
#QMAKE_LFLAGS += /def:pylonwrapper.def

include(../common.pri)

INCLUDEPATH += $$INTODIR/3rdparty/pylon/wrapper

unix {
  INCLUDEPATH += $$PYLONPATH/include $$PYLONPATH/genicam/library/CPP/include
}

win32 {
  !win32-msvc*: error(The Pylon wrapper must be compiled with MSVC.)
  INCLUDEPATH += $$PYLONPATH/pylon/include $$PYLONPATH/genicam/library/cpp/include
  QMAKE_LFLAGS += /OPT:NOREF
  MINGWLIB_TARGET = $$MODE\\libpylonwrapper.a
  QMAKE_EXTRA_TARGETS += msvc2mingw mingw
  msvc2mingw.commands += dlltool -U -D pylonwrapper.dll -d pylonwrapper.def -l $$MINGWLIB_TARGET $(DESTDIR_TARGET)
  msvc2mingw.depends = all
  msvc2mingw.target = $$MINGWLIB_TARGET
  mingw.depends = $$MINGWLIB_TARGET
}
