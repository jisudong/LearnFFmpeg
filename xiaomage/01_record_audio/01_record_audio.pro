QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audiothread.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    audiothread.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

macx {
    HOME_PATH = /usr/local/ffmpeg
    QMAKE_INFO_PLIST = mac/Info.plist
}

win32 {
    HOME_PATH = ..
}

INCLUDEPATH += $${HOME_PATH}/include

LIBS += -L $${HOME_PATH}/lib \
        -lavformat \
        -lavdevice \
        -lavutil

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

