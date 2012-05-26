include(../../base.pri)
SOURCES = *.cc
HEADERS = *.h
TARGET = aitomator
QT += script network
LIBS += -lpiiydin$$LIBVER -lpiicore$$LIBVER

exists(qtservice/src/qtservice.pri) {
  include(qtservice/src/qtservice.pri)
}
else {
  error(Please place the latest qtservice under qtservice/.)
}
