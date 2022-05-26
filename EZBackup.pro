#-------------------------------------------------
#
# Project created by QtCreator 2015-05-11T11:22:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EZBackup
TEMPLATE = app


SOURCES += main.cpp\
        ezbackup.cpp

HEADERS  += ezbackup.h

FORMS    += ezbackup.ui

RC_FILE = EZBackup.rc

RESOURCES += \
    resources.qrc

QT += winextras

