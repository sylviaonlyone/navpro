# Path to Into sources
INTODIR = ../../..
LIBS += -L$$INTODIR/lib
INCLUDEPATH += $$INTODIR/include
unix: LIBS += -lpiicore -lpiiydin -lpiigui
win32: LIBS += -lpiicore2 -lpiiydin2 -lpiigui2
SOURCES = Threshold.cc
TARGET = threshold
CONFIG += console
