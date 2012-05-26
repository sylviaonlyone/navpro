PLUGIN = Image
include(../piiplugin.pri)
enabled(fast): SOURCES += $$INTODIR/3rdparty/fast/*.cc
