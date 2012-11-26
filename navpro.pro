TEMPLATE = app
TARGET = pod
QT += core \
    gui 


QMAKE_CFLAGS += -Werror
#QMAKE_CXXFLAGS += -Werror -Wnon-virtual-dtor -Wreorder -Woverloaded-virtual

#RESOURCES += navpro.qrc
HEADERS += laneTracker.h \
           particleFilter.h \
           navproCore.h
SOURCES += main.cpp \
           laneTracker.cpp \
           particleFilter.cpp \
           navproCore.cpp
#FORMS += navpro.ui

DEFINES += QT_NO_DEBUG_OUTPUT

CV_INCLUDEPATH = /usr/local/include/ 
CV_LIBPATH = /usr/local/lib/

INCLUDEPATH += $$CV_INCLUDEPATH
               
DEPENDPATH += INCLUDEPATH

LIBS += -L$$CV_LIBPATH -lopencv_core -lopencv_highgui -lopencv_imgproc

QMAKE_LFLAGS += -Wl,-rpath,$$CV_LIBPATH


CONFIG += debug

CONFIG(release, debug|release) {
     release: DEFINES += NDEBUG USER_NO_DEBUG _DISABLE_LOG_
}

#----------------------------------------------------
# PII library related
#----------------------------------------------------
#INTODIR=$$PWD/3rdparth/Into/src
#INCLUDEPATH += $$INTODIR/../include
#include($$INTODIR/base.pri)

#specific plugin we want
#PLUGIN=transforms
#include(../piiplugin.pri)
#include($$INTODIR/plugins/piiplugin.pri)

#PII_LIBCOREPATH = ./3rdparth/Into/src/core/debug/
#PII_LIBYDINPATH = ./3rdparth/Into/src/ydin/debug/
#PII_LIBGUIPATH = ./3rdparth/Into/src/gui/debug/
#PII_LIBTRANSFORMPATH = ./3rdparth/Into/src/plugins/transforms/debug/
#
#QMAKE_LFLAGS +=  -Wl,-rpath,$$PII_LIBCOREPATH
#QMAKE_LFLAGS +=  -Wl,-rpath,$$PII_LIBYDINPATH
#QMAKE_LFLAGS +=  -Wl,-rpath,$$PII_LIBGUIPATH
#QMAKE_LFLAGS +=  -Wl,-rpath,$$PII_LIBTRANSFORMPATH
#
#win32:LIBS += -lpiicore2 -lpiiydin2
#
#unix|macx:LIBS += -L$$PII_LIBCOREPATH -lpiicore \
#                  -L$$PII_LIBYDINPATH -lpiiydin \
#                  -L$$PII_LIBGUIPATH -lpiigui \
#                  -L$$PII_LIBTRANSFORMPATH -lpiitransforms

#----------------------------------------------------
# Lagercy code
#----------------------------------------------------
# unused warnings  -Wold-style-cast

# Creates a file with svnversion info whenever any source or header changes
#build_info.commands = echo \"/* AUTO-GENERATED -- see CtxPodGui.pro */\" > auto_version.inc &&
#build_info.commands += echo -n \"static const char* svn_revision = \\\"\" >> auto_version.inc &&
#build_info.commands += svnversion -cn ../.. >> auto_version.inc &&
#build_info.commands += echo -n \\\"\";\" >> auto_version.inc &&
#build_info.commands += echo "" >> auto_version.inc 
#
#build_info.depends = $(SOURCES) $(HEADERS)
#build_info.target = auto_version.inc

#QMAKE_EXTRA_TARGETS += build_info
#RESOURCES += ctxpodgui.qrc
#CTX_INCLUDEPATH = ../Environment/ \
#    ./database/

#INCLUDEPATH += $$CTX_INCLUDEPATH
#DEPENDPATH += $$CTX_INCLUDEPATH

