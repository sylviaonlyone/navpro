INTODIR = ../../..
include(../../piiplugin.pri)

TARGET = piiiportcameradriver
DEFINES += PII_BUILD_IPORTCAMERADRIVER

INCLUDEPATH += $$INTODIR/3rdparty/iport/include
LIBS += -L$$INTODIR/3rdparty/iport/lib -lCyImgLib

PLUGINDEPS = Camera
include($$INTODIR/plugindeps.pri)
