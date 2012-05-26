INTODIR = ../../..
PLUGIN = camera/multicam
include(../../piiplugin.pri)
TARGET = piimulticamdriver
DEFINES += PII_BUILD_MULTICAMDRIVER

SOURCES += ../serial/*.cc
HEADERS += ../serial/*.h

INCLUDEPATH += $$INTODIR/3rdparty/multicam/include
LIBS += -L$$INTODIR/3rdparty/multicam/lib -lMultiCam -lclseremc
