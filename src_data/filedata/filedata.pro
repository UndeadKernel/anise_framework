QT += core
QT -= gui

TARGET = filedata
TEMPLATE = lib
CONFIG += plugin
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../../src_framework

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
  DEFINES += QT_NO_DEBUG_OUTPUT
}

QMAKE_CLEAN += $$DESTDIR/*$$TARGET*

HEADERS += \
    filedata.h \
    interface.h

SOURCES += \
    filedata.cpp \
    interface.cpp

