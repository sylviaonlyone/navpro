# Common build instructions to all libraries, plug-ins and programs.
# Insert this in your project-specific pro file:
#
# INTODIR = into_root_dir_relative_to_your_project_dir
# include($$INTODIR/base.pri)

INTO_MAJOR = 2
INTO_MINOR = 0.0
INTOVERSION = $${INTO_MAJOR}.$${INTO_MINOR}

win32:LIBVER=$$INTO_MAJOR

isEmpty(MODE) { MODE = debug }
CONFIG += qt thread warn_on exceptions $$MODE

isEmpty(INTODIR) { INTODIR = $$PWD }

DESTDIR = $$MODE
OBJECTS_DIR = $$MODE
MOC_DIR = $$MODE

INCLUDEPATH += . \
  $$INTODIR/core \
  $$INTODIR/core/serialization \
  $$INTODIR/core/matrix \
  $$INTODIR/core/network \
  $$INTODIR/ydin \
  $$INTODIR/gui
LIBS += -L$$INTODIR/core/$$MODE -L$$INTODIR/ydin/$$MODE -L$$INTODIR/gui/$$MODE

# Windows installation directory
win32:exists(../lib/piiydin.lib): LIBS += -L$$INTODIR/../lib

win32-msvc2008 {
# Disable checked iterators for VC2008 because they cause compilation errors.
  DEFINES += _SECURE_SCL=0 _CRT_SECURE_NO_WARNINGS
}

# disable warnings about throw specifications and deprecated POSIX names with VC compilers
win32-msvc* {
  QMAKE_CXXFLAGS_DEBUG += /wd4290 /wd4996
  QMAKE_CXXFLAGS_RELEASE += /wd4290 /wd4996
  QMAKE_LFLAGS += /OPT:NOREF
}

win32-g++ {
  QMAKE_LFLAGS += -Wl,--export-all-symbols -Wl,--enable-auto-import
}

# HACK: we don't want rpath in shared libs!
# Change the -Wl,-rpath, commandline option to a harmless -L
unix {
  eval(QT_MAJOR_VERSION == 4 && QT_MINOR_VERSION > 5) {
    QMAKE_LFLAGS_RPATH = -L
  } else {
    QMAKE_RPATH = -L
  }
}

equals(MODE, release) {
  unix:CONFIG += separate_debug_info
  macx:CONFIG += x86 pcc
  DEFINES += _RELEASE QT_NO_DEBUG
} else {
  DEFINES += _DEBUG
}

include(extensions.pri)
