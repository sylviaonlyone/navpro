SIMPLONPATH = $$INTODIR/3rdparty/simplon/driver
INCLUDEPATH += $$INTODIR/plugins/camera/genicam

win32 {
  LIBS += -L$$SIMPLONPATH/lib
  LIBS += -llv.simplon
}

unix {
  LIBS += -L$$SIMPLONPATH/lib
  LIBS += -llv.simplon
}

equals(MODE,release): DEFINES += NDEBUG
