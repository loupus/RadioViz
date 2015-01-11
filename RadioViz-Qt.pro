#-------------------------------------------------
#
# Project created by QtCreator 2015-01-10T20:58:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RadioViz-Qt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    camerawidget.cpp

HEADERS  += mainwindow.h \
    radioviz.h \
    camerawidget.h

FORMS    +=

unix: LIBS += -L/usr/local/lib -lavutil -lavcodec -lavformat -lavdevice  -lswscale -lopencv_core -lopencv_imgproc -lopencv_highgui
