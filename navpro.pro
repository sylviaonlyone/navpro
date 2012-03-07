TEMPLATE = app
TARGET = pod
QT += core \
    gui \
    sql

QMAKE_CFLAGS += -Werror
QMAKE_CXXFLAGS += -Werror -Wnon-virtual-dtor -Wreorder -Woverloaded-virtual
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


#HEADERS += ctxlanguage.h \
#    ctxpodgui.h
SOURCES += main.cpp
FORMS += ctxpodgui.ui
#RESOURCES += ctxpodgui.qrc
#CTX_INCLUDEPATH = ../Environment/ \
#    ./database/

#INCLUDEPATH += $$CTX_INCLUDEPATH
#DEPENDPATH += $$CTX_INCLUDEPATH
DEFINES += QT_NO_DEBUG_OUTPUT

CONFIG(release, debug|release) {
     release: DEFINES += NDEBUG USER_NO_DEBUG _DISABLE_LOG_
}
