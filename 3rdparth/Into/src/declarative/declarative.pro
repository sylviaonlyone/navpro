TEMPLATE = lib
TARGET = piideclarative
CONFIG += plugin
QT += declarative script
SOURCES = *.cc
HEADERS = *.h

INTODIR = ..
include($$INTODIR/base.pri)

DEFINES += PII_BUILD_DECLARATIVE
LIBS += -lpiicore$$LIBVER
