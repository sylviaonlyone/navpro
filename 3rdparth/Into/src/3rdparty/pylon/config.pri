include(common.pri)

INCLUDEPATH += $$INTODIR/3rdparty/pylon/wrapper
LIBS += -L$$INTODIR/3rdparty/pylon/wrapper/$$MODE -lpylonwrapper

exists($$PWD/driver):EXT_ENABLED=true
