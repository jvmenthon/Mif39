#-------------------------------------------------
#
# Project created by QtCreator 2015-04-14T23:34:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Interactions
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    entity.cpp \
    mesh.cpp \
    activeentity.cpp \
    passiveentity.cpp

HEADERS  += mainwindow.h \
    entity.h \
    mesh.h \
    activeentity.h \
    passiveentity.h

FORMS    += mainwindow.ui
