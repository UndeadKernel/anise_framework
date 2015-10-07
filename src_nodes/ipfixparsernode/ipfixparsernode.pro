QT += core
QT -= gui

TARGET = ipfixparsernode
TEMPLATE = lib
CONFIG += plugin
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../../src_framework \
               ../../src_data \
               extras/libfixbuf/include

LIBS += -L./extras/libfixbuf/ -lfixbuf
# Include the glib bindings using 'pkg-conf'.
CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0

CONFIG(debug,debug|release) {
  # Debug...
  DESTDIR = ../../bin/debug/nodes
  OBJECTS_DIR = build/debug
  MOC_DIR = build/debug/moc
  RCC_DIR = build/debug/rcc
} else {
  # Release...
  DESTDIR = ../../bin/release/nodes
  OBJECTS_DIR = build/release
  MOC_DIR = build/release/moc
  RCC_DIR = build/release/rcc
  #DEFINES += QT_NO_DEBUG_OUTPUT
  DEFINES += QT_MESSAGELOGCONTEXT
}

libfixbuf.path = $$DESTDIR/extras
libfixbuf.files = extras/libfixbuf/libfixbuf.so.3
INSTALLS += libfixbuf

QMAKE_CLEAN += $$DESTDIR/*$$TARGET*

HEADERS += \
    ipfixparsernode.h \
    interface.h \
    flowtemplate.h \
    cert_ie.h

SOURCES += \
    ipfixparsernode.cpp \
    interface.cpp

