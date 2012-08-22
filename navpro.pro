TEMPLATE = app
TARGET = pod
QT += core \
    gui \
    sql

INTODIR=$$PWD/3rdparth/Into/src
INCLUDEPATH += $$INTODIR/../include
include($$INTODIR/base.pri)

#specific plugin we want
#PLUGIN=transforms
#include(../piiplugin.pri)
#include($$INTODIR/plugins/piiplugin.pri)

QMAKE_CFLAGS += -Werror
QMAKE_CXXFLAGS += -Werror -Wnon-virtual-dtor -Wreorder -Woverloaded-virtual

RESOURCES += navpro.qrc
HEADERS += navproCore.h
SOURCES += main.cpp \
           navproCore.cpp
FORMS += navpro.ui

DEFINES += QT_NO_DEBUG_OUTPUT

win32:LIBS += -lpiicore2 -lpiiydin2
unix|macx:LIBS += -L./3rdparth/Into/src/core/debug/ -lpiicore -L./3rdparth/Into/src/ydin/debug/ -lpiiydin -L./3rdparth/Into/src/gui/debug/ -lpiigui -L./3rdparth/Into/src/plugins/transforms/debug/ -lpiitransforms

CONFIG(release, debug|release) {
     release: DEFINES += NDEBUG USER_NO_DEBUG _DISABLE_LOG_
}

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

