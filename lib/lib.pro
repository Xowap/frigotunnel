#-------------------------------------------------
#
# Project created by QtCreator 2015-01-17T11:44:08
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = lib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += frigotunnel.cpp \
    frigomessage.cpp

HEADERS += frigotunnel.h \
    frigomessage.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
