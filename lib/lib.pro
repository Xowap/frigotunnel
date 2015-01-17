#-------------------------------------------------
#
# Project created by QtCreator 2015-01-17T11:44:08
#
#-------------------------------------------------

include(../defaults.pri)

QT       += network

QT       -= gui

TARGET = frigotunnel
TEMPLATE = lib
CONFIG += staticlib

SOURCES += frigotunnel.cpp \
    frigomessage.cpp \
    frigopacket.cpp \
    expiringset.cpp

HEADERS += frigotunnel.h \
    frigomessage.h \
    frigopacket.h \
    common.h \
    expiringset.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
