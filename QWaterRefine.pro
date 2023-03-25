#-------------------------------------------------
#
# Project created by QtCreator 2017-01-03T18:53:43
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QWaterRefine
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    initdb.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc

win32:RC_FILE += QWaterRefine.rc
