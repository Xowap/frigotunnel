#-------------------------------------------------
#
# Project created by QtCreator 2015-01-17T20:03:52
#
#-------------------------------------------------

include(../defaults.pri)

QT       += core testlib
QT       -= gui

TARGET = tests

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    parsepackettest.cpp \
    expiringsettest.cpp

LIBS += -L../lib -lfrigotunnel

HEADERS += \
    parsepackettest.h \
    expiringsettest.h

RESOURCES += \
    resources.qrc
