#-------------------------------------------------
#
# Project created by QtCreator 2015-08-06T12:29:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LookFor
TEMPLATE = app

CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    file_system_model.cpp

HEADERS  += mainwindow.hpp \
    macros.hpp \
    file_system_model.hpp

FORMS    += mainwindow.ui

RESOURCES += \
    assets.qrc
