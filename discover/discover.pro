#-------------------------------------------------
#
# Project created by QtCreator 2015-03-24T21:26:39
#
#-------------------------------------------------

include(../defaults.pri)

QT       += core network
QT       -= gui

TARGET = frigo-discover

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    discovertask.cpp

LIBS += -L../lib -lfrigotunnel

HEADERS += \
    discovertask.h
