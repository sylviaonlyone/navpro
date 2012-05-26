isEmpty(MODE) { MODE = debug }

INTODIR = ../../..
TEMPLATE = lib
CONFIG -= qt
CONFIG += $$MODE
SOURCES = *.cc
TARGET = simplonwrapper

DESTDIR = $$MODE
OBJECTS_DIR = $$MODE
MOC_DIR = $$MODE

DEFINES += BUILD_SIMPLON_WAPI

include(../common.pri)

INCLUDEPATH += $$INTODIR/3rdparty/simplon/wrapper
INCLUDEPATH += $$SIMPLONPATH/include

SOURCES += $$SIMPLONPATH/include/lv.simplon.class.cpp