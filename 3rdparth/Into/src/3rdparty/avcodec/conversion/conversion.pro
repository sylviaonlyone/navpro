win32:!win32-g++:error("Must compile with MinGW")

isEmpty(MODE) { MODE = release }

DESTDIR = $$MODE
OBJECTS_DIR = $$MODE
MOC_DIR = $$MODE

TEMPLATE = lib
TARGET = avimgconversion
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../include
LIBS += -L../lib -lavformat -lavcodec -lavutil
CONFIG -= qt
CONFIG += static release $$MODE

# Input
SOURCES += imgconvert.c imgconvert_template.c
win32:QMAKE_DIR_SEP = /
