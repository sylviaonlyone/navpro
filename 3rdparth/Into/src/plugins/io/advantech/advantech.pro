INTODIR = ../../..
PLUGIN = io/advantech
include(../../piiplugin.pri)
TARGET = piiadvantechiodriver
DEFINES += PII_BUILD_ADVANTECHIODRIVER

INCLUDEPATH += .. $$INTODIR/3rdparty/advantech/include
LIBS += -L$$INTODIR/3rdparty/advantech/lib -ladsapi32
