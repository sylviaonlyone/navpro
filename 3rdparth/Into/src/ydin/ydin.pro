TEMPLATE        = lib
TARGET          = piiydin
HEADERS         = *.h
SOURCES         = *.cc

INTODIR = ..
include($$INTODIR/base.pri)
VERSION         = $$INTOVERSION

DEFINES += PII_BUILD_YDIN
LIBS += -lpiicore$$LIBVER
