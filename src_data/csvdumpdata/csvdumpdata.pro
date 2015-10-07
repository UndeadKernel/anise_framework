QT += core
QT -= gui

TARGET = csvdumpdata
TEMPLATE = lib
CONFIG += plugin
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../../src_framework \
               ../../src_data

CONFIG(debug,debug|release) {
  # Debug...
  DESTDIR = ../../bin/debug/data
  OBJECTS_DIR = build/debug
  MOC_DIR = build/debug/moc
  RCC_DIR = build/debug/rcc
} else {
  # Release...
  DESTDIR = ../../bin/release/data
  OBJECTS_DIR = build/release
  MOC_DIR = build/release/moc
  RCC_DIR = build/release/rcc
  #DEFINES += QT_NO_DEBUG_OUTPUT
  DEFINES += QT_MESSAGELOGCONTEXT
}

QMAKE_CLEAN += $$DESTDIR/*$$TARGET*

HEADERS += \
    csvdumpdata.h \
    interface.h

SOURCES += \
    csvdumpdata.cpp \
    interface.cpp

