TEMPLATE        = lib
TARGET          = demoutils
HEADERS         = *.h
SOURCES         = *.cc

RESOURCES += resources.qrc

INTODIR         = ../..
include($$INTODIR/base.pri)

DEFINES += PII_BUILD_DEMOUTILS
LIBS += -lpiiydin$$LIBVER -lpiicore$$LIBVER
