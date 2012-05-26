PYLONPATH = $$INTODIR/3rdparty/pylon/driver
INCLUDEPATH += $$INTODIR/plugins/camera/genicam

win32 {
  LIBS += -L$$PYLONPATH/pylon/lib/Win32
  LIBS += -L$$PYLONPATH/genicam/library/cpp/lib/Win32_i86
  LIBS += -L$$PYLONPATH/genicam/bin/Win32_i86/GenApi/Generic
  LIBS += -lPylonBase_MD_VC80
}

unix {
  LIBS += -L$$PYLONPATH/lib64
  LIBS += -L$$PYLONPATH/genicam/bin/Linux64_x64
  LIBS += -L$$PYLONPATH/genicam/bin/Linux64_x64/GenApi/Generic
  LIBS += -lpylonbase
}

equals(MODE,release): DEFINES += NDEBUG
