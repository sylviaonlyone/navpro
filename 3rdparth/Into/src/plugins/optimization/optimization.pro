PLUGIN = Optimization
include(../piiplugin.pri)
enabled(lbfgs): SOURCES += $$INTODIR/3rdparty/lbfgs/*.cpp
enabled(lmfit): SOURCES += $$INTODIR/3rdparty/lmfit/*.c
