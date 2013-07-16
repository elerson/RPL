#-------------------------------------------------
#
# Project created by QtCreator 2013-01-08T20:08:27
#
#-------------------------------------------------

QT       += core network gui

CONFIG += serialport

TARGET = rpl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    Zigbee.cpp \
    rpl.cpp \
    ip6.cpp

HEADERS  += mainwindow.h \
    Zigbee.h \
    rpl.h \
    ip6.h \
    ByteArray.h \
    SinkPlatform.h

FORMS    += mainwindow.ui
