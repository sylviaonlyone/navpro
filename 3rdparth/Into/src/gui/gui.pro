TEMPLATE        = lib
TARGET          = piigui
HEADERS         = *.h
SOURCES         = *.cc

INTODIR = ..
include($$INTODIR/base.pri)
VERSION         = $$INTOVERSION

RESOURCES += resources.qrc
DEFINES += PII_BUILD_GUI
LIBS += -lpiicore$$LIBVER
