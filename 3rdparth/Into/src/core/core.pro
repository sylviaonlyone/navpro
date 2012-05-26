# Generate a debug makefile with "qmake MODE=debug"

TEMPLATE        = lib
TARGET          = piicore
HEADERS         = *.h
SOURCES         = *.cc serialization/*.cc matrix/*.cc
INCLUDEPATH     += $$(QTDIR)/include/Qt

!contains(DISABLE,network) {
  HEADERS += network/*.h
  SOURCES += network/*.cc
  QT += network
}

INTODIR          = ..
include($$INTODIR/base.pri)
VERSION         = $$INTOVERSION

DEFINES += PII_BUILD_CORE
