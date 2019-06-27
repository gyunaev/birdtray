#-------------------------------------------------
#
# Project created by QtCreator 2018-09-10T20:39:25
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = birdtray
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lsqlite3
#-lXpm -lXmu

SOURCES += \
        main.cpp \
    trayicon.cpp \
    settings.cpp \
    unreadcounter.cpp \
    sqlite_statement.cpp \
    colorbutton.cpp \
    databaseaccounts.cpp \
    databaseunreadfixer.cpp \
    dialogaddeditaccount.cpp \
    dialogsettings.cpp \
    windowtools.cpp \
    dialogaddeditnewemail.cpp \
    setting_newemail.cpp \
    modelnewemails.cpp \
    modelaccounttree.cpp \
    morkparser.cpp \
    utils.cpp

HEADERS += \
    trayicon.h \
    settings.h \
    unreadcounter.h \
    sqlite_statement.h \
    colorbutton.h \
    databaseaccounts.h \
    databaseunreadfixer.h \
    dialogaddeditaccount.h \
    dialogsettings.h \
    version.h \
    windowtools.h \
    dialogaddeditnewemail.h \
    setting_newemail.h \
    modelnewemails.h \
    modelaccounttree.h \
    morkparser.h \
    utils.h

FORMS += \
    dialogaddeditaccount.ui \
    dialogsettings.ui \
    dialogaddeditnewemail.ui

RESOURCES += \
    resources.qrc

RC_FILE = res\birdtray.rc

unix {
     SOURCES += windowtools_x11.cpp
     HEADERS += windowtools_x11.h
     LIBS += -lX11
     QT += x11extras
}
win32 {
     DEFINES += -DUNICODE
     SOURCES += windowtools_win.cpp birdtrayeventfilter.cpp processhandle.cpp
     HEADERS += windowtools_win.h birdtrayeventfilter.h processhandle.h
     LIBS += user32.lib
}
