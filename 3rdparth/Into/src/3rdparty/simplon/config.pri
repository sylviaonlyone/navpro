include(common.pri)

INCLUDEPATH += $$INTODIR/3rdparty/simplon/wrapper
LIBS += -L$$INTODIR/3rdparty/simplon/wrapper/$$MODE -lsimplonwrapper

exists($$PWD/driver):EXT_ENABLED=true
