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
    file_system_model.cpp \
    proxy_item_delegate.cpp \
    dir_scaner.cpp \
    reg_exp_dialog.cpp

HEADERS  += mainwindow.hpp \
    macros.hpp \
    file_system_model.hpp \
    proxy_item_delegate.hpp \
    dir_scaner.hpp \
    reg_exp_dialog.hpp

FORMS    += mainwindow.ui \
    regexpdialog.ui

RESOURCES += \
    assets.qrc
