INTODIR = ../../..
PLUGIN = io/multicam
include(../../piiplugin.pri)
TARGET = piimulticamiodriver
DEFINES += PII_BUILD_MULTICAMIODRIVER

INCLUDEPATH += $$INTODIR/3rdparty/multicam/include
LIBS += -L$$INTODIR/3rdparty/multicam/lib -lMultiCam
