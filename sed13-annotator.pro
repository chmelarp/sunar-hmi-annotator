#-------------------------------------------------
#
# Project created by QtCreator 2012-09-26T14:20:57
#
#-------------------------------------------------

QT       += core gui

TARGET = sed13-annotator
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    workthread.cpp \
    cvwidget.cpp

HEADERS  += mainwindow.h \
    workthread.h \
    cvwidget.h

FORMS    += mainwindow.ui

# pridani opencv a dalsich knihoven
CONFIG += link_pkgconfig
PKGCONFIG = opencv
INCLUDEPATH += /usr/include/postgresql
LIBS += -lvtapi -llwgeom -lpqtypes -lproc

OTHER_FILES += \
    vtapi.conf
