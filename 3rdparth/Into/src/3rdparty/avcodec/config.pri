INCLUDEPATH += $$INTODIR/3rdparty/avcodec/conversion
LIBS        += -L$$INTODIR/3rdparty/avcodec/conversion/$$MODE
win32 {
  INCLUDEPATH   += $$INTODIR/3rdparty/avcodec/extra
  LIBS          += -L$$INTODIR/3rdparty/avcodec/extra/lib
  LIBS          += libavimgconversion.a libavformat.a libavcodec.a libavutil.a libgcc.a libmingwex.a libcoldname.a libwsock32.a libmsvcrt.a  
  QMAKE_LFLAGS  += /OPT:NOREF
}
unix {
  LIBS          += -lavformat -lavcodec -lavutil -lavimgconversion
}
