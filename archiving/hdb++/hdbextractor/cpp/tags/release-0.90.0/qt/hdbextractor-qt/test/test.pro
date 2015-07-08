#-------------------------------------------------
#
# Project created by QtCreator 2014-08-08T16:46:30
#
#-------------------------------------------------

QT       += core

# QT       -= gui

INCLUDEPATH += ../src ../../../src ../../../db/src

TARGET = test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L.. -lhdbextractor-qt -L../../../ -lhdbextractor++

SOURCES += main.cpp
